#include <Arduino.h>
#include <HardwareSerial.h>

// --- UART PINS (Serial1) ---
#define STM_RX_PIN 17 
#define STM_TX_PIN 18 
#define STM_UART_BAUD 115200

// --- MOTOR PINS ---
const int IN1_PIN = 15;
const int IN2_PIN = 4;
const int ENC_A_PIN = 38;
const int ENC_B_PIN = 37;

// --- MATH & CONSTANTS ---
const float COUNTS_PER_REV = 800; 
const int PWM_FREQ = 20000;
const int PWM_RES = 8;

// --- PID Data Structure ---
struct PID_Data {
  float Kp;
  float Ki;
  float Kd;
  float MODE; 
};

// --- GLOBAL VARIABLES ---
PID_Data my_pid = { 4.0, 0.5, 0.01, 0.0 }; 

// UART Buffers
char command_buffer[256];                
int buffer_index = 0;

// Motor Control Globals
volatile long encoderCount = 0;
float targetRotations = 0; 
unsigned long lastTime = 0;
float prevError = 0;
float integral = 0;

// --- ISR (Encoder Counting) ---
void IRAM_ATTR readEncoder() {
  int bState = digitalRead(ENC_B_PIN);
  if (digitalRead(ENC_A_PIN) == HIGH) {
    if (bState == LOW) encoderCount++;
    else encoderCount--;
  } else {
    if (bState == HIGH) encoderCount++;
    else encoderCount--;
  }
}

// --- MOTOR HELPER ---
void setMotor(int pwmVal) {
  if (pwmVal > 255) pwmVal = 255;
  if (pwmVal < -255) pwmVal = -255;
  if (abs(pwmVal) < 10) pwmVal = 0; 

  if (pwmVal > 0) {
    ledcWrite(IN1_PIN, pwmVal);
    ledcWrite(IN2_PIN, 0);
  } else if (pwmVal < 0) {
    ledcWrite(IN1_PIN, 0);
    ledcWrite(IN2_PIN, abs(pwmVal));
  } else {
    ledcWrite(IN1_PIN, 0);
    ledcWrite(IN2_PIN, 0);
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  
  // Hardware UART (Serial1)
  Serial1.begin(STM_UART_BAUD, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);
  
  // Motor & Encoder
  pinMode(IN1_PIN, OUTPUT); pinMode(IN2_PIN, OUTPUT);
  digitalWrite(IN1_PIN, LOW); digitalWrite(IN2_PIN, LOW);
  pinMode(ENC_A_PIN, INPUT_PULLUP); pinMode(ENC_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), readEncoder, CHANGE);

  // PWM
  ledcAttach(IN1_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(IN2_PIN, PWM_FREQ, PWM_RES);

  targetRotations = 0; 
  encoderCount = 0; 
  
  Serial.println("ESP32 Ready. Mode 0=Fwd, Mode 1=Back");
  
  while(Serial1.available() > 0) { char t = Serial1.read(); }
  lastTime = millis();
}

/**
 * @brief Processes a complete command string
 */
void process_command(char* command) {
  Serial.print("CMD: "); Serial.println(command); 

  // --- 1. Handle Manual MOVEMENT Strings ---
  if (strncmp(command, "forwards", 8) == 0) {
    my_pid.MODE = 0.0; // Update Mode Variable
    targetRotations += 1.5;
    Serial1.print("OK: Forwards (Mode set to 0)\n");
  } 
  else if (strncmp(command, "backwards", 9) == 0) {
    my_pid.MODE = 1.0; // Update Mode Variable
    targetRotations -= 1.5;
    Serial1.print("OK: Backwards (Mode set to 1)\n");
  }
  
  // --- 2. Handle GET_PID ---
  else if (strncmp(command, "GET_PID", 7) == 0) {
    char response_str[100];
    snprintf(response_str, 100, "Kp:%.2f, Ki:%.2f, Kd:%.2f, Mode:%.2f\n",
             my_pid.Kp, my_pid.Ki, my_pid.Kd, my_pid.MODE);
    Serial1.print(response_str); 
  } 
  
  // --- 3. Handle SET_PID_DATA (Update Params AND Move) ---
  else if (strncmp(command, "SET_PID_DATA ", 13) == 0) {
    char* data_str = command + 13; 
    
    // Parse 4 floats directly into global struct
    int items_parsed = sscanf(data_str, "%f,%f,%f,%f",
                              &my_pid.Kp, &my_pid.Ki, &my_pid.Kd, &my_pid.MODE);
    
    if (items_parsed == 4) {
      // Reset Integral to prevent jumps with new params
      integral = 0; 

      // Determine Direction based on the NEW Mode
      int direction = (int)my_pid.MODE;
      
      if (direction == 0) {
        targetRotations += 1.5; // Move Forwards with NEW PID
        Serial1.print("OK: PID Updated, Mode=0, Moving FWD\n");
      } 
      else if (direction == 1) {
        targetRotations -= 1.5; // Move Backwards with NEW PID
        Serial1.print("OK: PID Updated, Mode=1, Moving BCK\n");
      } 
      else {
        Serial1.print("OK: PID Updated (No Move)\n");
      }
    } else {
      Serial1.print("E_PARSE_ERROR\n"); 
    }
  } 
  else if (strncmp(command, "hello", 5) == 0) {
    Serial1.print("Hi from ESP32!\n");
  } 
  else {
    Serial1.print("UNKNOWN_COMMAND\n");
  }
}

void loop() {
  // ==========================================
  // 1. UART LISTENER
  // ==========================================
  if (Serial1.available() > 0) {
    char received_byte = Serial1.read();
    
    if (received_byte == '\n') {
      command_buffer[buffer_index] = '\0'; 
      if (buffer_index > 0) {
        if (command_buffer[buffer_index-1] == '\r') command_buffer[buffer_index-1] = '\0';
        process_command(command_buffer);
      }
      buffer_index = 0; 
    } 
    else if (received_byte != '\r' && buffer_index < 255) {
      command_buffer[buffer_index++] = received_byte;
    }
  }

  // ==========================================
  // 2. PID LOOP (Uses current my_pid values)
  // ==========================================
  unsigned long now = millis();
  if (now - lastTime >= 10) { 
    float dt = (now - lastTime) / 1000.0; 
    lastTime = now;

    long targetCounts = (long)(targetRotations * COUNTS_PER_REV);
    long error = targetCounts - encoderCount;

    // Uses the global my_pid struct which might have just changed
    float P = my_pid.Kp * error;
    integral += error * dt;
    float I = my_pid.Ki * integral;
    float D = my_pid.Kd * (error - prevError) / dt;
    prevError = error;

    setMotor((int)(P + I + D));
  }
}
