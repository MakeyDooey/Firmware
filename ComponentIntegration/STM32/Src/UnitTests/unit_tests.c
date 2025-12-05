//unit_tests.c
// WRAP EVERYTHING IN DEBUG SO IT DOESN'T EXIST IN RELEASE BUILDS
#ifdef DEBUG

#include "unit_tests.h"
#include "main.h"
#include "stm32h7xx_nucleo.h"
#include "unity.h"
#include <stdio.h>
#include <string.h> // For strlen and memset

extern UART_HandleTypeDef hcom_uart[COMn];
extern RNG_HandleTypeDef hrng;
extern UART_HandleTypeDef huart7; // Assumes default handle name from CubeMX

#define RUN_TEST_PRETTY(a) RUN_TEST(a); printf("\r");
#define TEST_BAUD_RATE 115200
// SRAM4 is accessible by both cores at 0x38000000
#define SHARED_RAM_ADDR 0x38000000

typedef struct {
	volatile uint32_t test_flag;    // 0 = Idle, 1 = M4 Request, 2 = M7 Response
	volatile uint32_t data_sent;    // Data M4 sends to M7
	volatile uint32_t data_echo;    // Data M7 sends back
} IPC_Mailbox_t;

#define IPC_BOX ((IPC_Mailbox_t *)SHARED_RAM_ADDR)

// --- UNITY SETUP/TEARDOWN ---
void setUp(void) {
	// Ensure a clean slate: All LEDs OFF before each test
	BSP_LED_Off(LED1);
	BSP_LED_Off(LED2);
	BSP_LED_Off(LED3);
}

void tearDown(void) {
	// Small delay to visualize the test running
	HAL_Delay(50);
}

// --- TEST CASES ---

// Test 1: Green LED (PB0)
static void test_Green_LED_Toggle(void) {
	// Act
	BSP_LED_On(LED1);
	HAL_Delay(10);

	// Assert: Read Input Data Register of GPIOB, Pin 0
	TEST_ASSERT_EQUAL_INT_MESSAGE(GPIO_PIN_SET,
			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0),
			"Green LED (PB0) Failed to Turn ON");

	// Clean up
	BSP_LED_Off(LED1);
}

// Test 2: Yellow LED (PE1)
// Note: This is on PORT E
static void test_Yellow_LED_Toggle(void) {
	// Act
	BSP_LED_On(LED2);
	HAL_Delay(10);

	// Assert: Read Input Data Register of GPIOE, Pin 1
	TEST_ASSERT_EQUAL_INT_MESSAGE(GPIO_PIN_SET,
			HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1),
			"Yellow LED (PE1) Failed to Turn ON");

	// Clean up
	BSP_LED_Off(LED2);
}

// Test 3: Red LED (PB14)
static void test_Red_LED_Toggle(void) {
	// Act
	BSP_LED_On(LED3);
	HAL_Delay(10);

	// Assert: Read Input Data Register of GPIOB, Pin 14
	TEST_ASSERT_EQUAL_INT_MESSAGE(GPIO_PIN_SET,
			HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14),
			"Red LED (PB14) Failed to Turn ON");

	// Clean up
	BSP_LED_Off(LED3);
}

static void test_Button_Default_Is_Released(void) {
	// 1. Read Button State
	// On Nucleo, Released = 0 (GPIO_PIN_RESET), Pressed = 1 (GPIO_PIN_SET)
	uint32_t state = BSP_PB_GetState(BUTTON_USER);

	// 2. Assert it is RELEASED (0)
	// If this fails, the button might be stuck physically, or the pin ownership is wrong in IOC.
	TEST_ASSERT_EQUAL_INT_MESSAGE(GPIO_PIN_RESET, state,
			"User Button (PC13) Detected as PRESSED/STUCK during boot!");
}

static void test_Serial_Connection_Health(void) {
	// 1. Verify the Handle State
	// If initialization failed, the state would be HAL_UART_STATE_RESET or HAL_UART_STATE_ERROR
	// It should be READY or BUSY_TX (since we are printing)
	uint32_t state = HAL_UART_GetState(&hcom_uart[COM1]);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(HAL_UART_STATE_RESET, state,
			"UART3 Peripheral is not initialized!");
	TEST_ASSERT_NOT_EQUAL_MESSAGE(HAL_UART_STATE_ERROR, state,
			"UART3 Peripheral reports an internal ERROR!");

	// 2. Verify Configuration (Baud Rate)
	// This ensures the firmware is actually set to 115200 as expected by your terminal
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(115200, hcom_uart[COM1].Init.BaudRate,
			"UART Baud Rate is not 115200!");

	// 3. Test Transmission Capability
	// Attempt to send a single "Space" character directly to the hardware
	// We use a short timeout (10ms). If the hardware is frozen, this will return HAL_TIMEOUT.
	uint8_t test_char = ' ';
	HAL_StatusTypeDef status = HAL_UART_Transmit(&hcom_uart[COM1], &test_char,
			1, 10);

	TEST_ASSERT_EQUAL_INT_MESSAGE(HAL_OK, status,
			"UART Hardware failed to transmit a byte (Timeout/Error)");
}

static void test_IPC_Ping_Pong(void) {

	// 1. SETUP: Clean the mailbox
	// We treat the pointer as a volatile hardware register
	IPC_BOX->test_flag = 0;
	IPC_BOX->data_sent = 0;
	IPC_BOX->data_echo = 0;

	// 2. ACT: Send a "Challenge" pattern
	uint32_t challenge = 0xAAAA5555; // Alternating bits
	IPC_BOX->data_sent = challenge;

	// Signal M7 to process it
	IPC_BOX->test_flag = 1;

	// 3. WAIT: Poll for response with Timeout
	// If M7 is dead/crashed, we don't want to hang forever.
	uint32_t start_tick = HAL_GetTick();
	uint32_t timeout_ms = 50; // Should happen instantly, 50ms is generous
	int responded = 0;

	while ((HAL_GetTick() - start_tick) < timeout_ms) {
		// Check if M7 set flag to 2
		if (IPC_BOX->test_flag == 2) {
			responded = 1;
			break;
		}
	}

	// 4. ASSERTIONS

	// A. Did it time out?
	TEST_ASSERT_TRUE_MESSAGE(responded,
			"IPC ERROR: M7 Core did not respond (Timeout)");

	// B. Did we get the correct math back?
	// M7 code performs Bitwise NOT (~), so we expect ~challenge
	uint32_t expected = ~challenge;
	uint32_t actual = IPC_BOX->data_echo;

	char msg[128];
	sprintf(msg, "IPC Data Corruption! Sent: %lX, Expected: %lX, Got: %lX",
			challenge, expected, actual);

	TEST_ASSERT_EQUAL_HEX32_MESSAGE(expected, actual, msg);
}

static void test_RNG_Entropy_Health(void) {
	uint32_t rand1 = 0;
	uint32_t rand2 = 0;
	HAL_StatusTypeDef status;

	// 1. Generate First Number
	status = HAL_RNG_GenerateRandomNumber(&hrng, &rand1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(HAL_OK, status,
			"RNG Failed to generate number 1");

	// 2. Generate Second Number
	status = HAL_RNG_GenerateRandomNumber(&hrng, &rand2);
	TEST_ASSERT_EQUAL_INT_MESSAGE(HAL_OK, status,
			"RNG Failed to generate number 2");

	// 3. Assertions
	// A. It shouldn't be zero (statistically unlikely)
	TEST_ASSERT_NOT_EQUAL_MESSAGE(0, rand1,
			"RNG generated pure Zero (Suspicious)");

	// B. It shouldn't generate the exact same number twice (Frozen RNG)
	TEST_ASSERT_NOT_EQUAL_MESSAGE(rand1, rand2,
			"RNG Output is STUCK (Duplicate values)");
}

/*
 * @brief Tests UART7 peripheral health via a loopback test.
 * @note  **HARDWARE SETUP REQUIRED!**
 * This test requires you to physically connect the
 * UART7_TX pin (PE8) to the UART7_RX pin (PE7)
 * with a jumper wire before running.
 */
static void test_UART7_Loopback(void) {
	// --- 1. Define Customizable Message ---
	const char *LOOPBACK_MSG = "2.0,0.5,0.01,1";
	const uint16_t msg_len = strlen(LOOPBACK_MSG);
	uint8_t rx_buffer[msg_len + 1]; // +1 for null terminator
	uint32_t timeout = 1000; // 100ms timeout per byte

	// Clear RX buffer
	memset(rx_buffer, 0, msg_len + 1);

	// --- 2. Verify Handle State ---
	uint32_t state = HAL_UART_GetState(&huart7);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(HAL_UART_STATE_RESET, state,
			"UART7 Peripheral is not initialized!");
	TEST_ASSERT_NOT_EQUAL_MESSAGE(HAL_UART_STATE_ERROR, state,
			"UART7 Peripheral reports an internal ERROR!");

	// --- 3. Verify Configuration (Baud Rate) ---
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TEST_BAUD_RATE, huart7.Init.BaudRate,
			"UART7 Baud Rate is not equal to TEST_BAUD_RATE: 115200");

	// --- 4. Test Loopback TX/RX (Byte-by-Byte) ---
	HAL_StatusTypeDef status_tx;
	HAL_StatusTypeDef status_rx;

	// We must send and receive one byte at a time to avoid a buffer overrun
	for (uint16_t ii = 0; ii < msg_len; ii++) {
		// A. Transmit one byte
		status_tx = HAL_UART_Transmit(&huart7, (uint8_t*) &LOOPBACK_MSG[ii], 1,
				timeout);
		if (status_tx != HAL_OK) {
			char err_msg[100];
			sprintf(err_msg,
					"UART7 HAL_UART_Transmit failed (Timeout/Error) on byte %d",
					ii);
			TEST_FAIL_MESSAGE(err_msg);
			return; // Abort test
		}

		// B. Receive one byte
		status_rx = HAL_UART_Receive(&huart7, &rx_buffer[ii], 1, timeout);
		if (status_rx != HAL_OK) {
			char err_msg[120];
			sprintf(err_msg,
					"UART7 HAL_UART_Receive failed (Timeout/Error) on byte %d. Is loopback jumper (PE8-PE7) connected?",
					ii);
			TEST_FAIL_MESSAGE(err_msg);
			return; // Abort test
		}
	}

	// --- 5. Print Received Data ---
	// Ensure null termination for safe printing as a string
	rx_buffer[msg_len] = '\0';
	// Print the result via the main debug UART (COM1/UART3)
	printf("[Test] UART7 Received: '%s'\n\r", (char*) rx_buffer);

	// --- 6. Assertions ---
	// Compare the full string now that the loop is complete
	TEST_ASSERT_EQUAL_STRING_MESSAGE(LOOPBACK_MSG, (char* ) rx_buffer,
			"UART7 Loopback data corruption! TX != RX");
}

// --- PUBLIC RUNNER ---
void Run_Boot_Tests(void) {
	printf("\n\r[DEBUG] Starting Hardware Diagnostics (LEDs + Button)...\n\r");

	UNITY_BEGIN();

	RUN_TEST_PRETTY(test_Green_LED_Toggle);
	RUN_TEST_PRETTY(test_Yellow_LED_Toggle);
	RUN_TEST_PRETTY(test_Red_LED_Toggle);
	RUN_TEST_PRETTY(test_Button_Default_Is_Released);
	RUN_TEST_PRETTY(test_Serial_Connection_Health);
	RUN_TEST_PRETTY(test_IPC_Ping_Pong);
	RUN_TEST_PRETTY(test_RNG_Entropy_Health);
    RUN_TEST_PRETTY(test_UART7_Loopback);

	int failures = UNITY_END();

	if (failures == 0) {
		printf(">>> ALL HARDWARE OPERATIONAL. BOOTING. <<<\n\r\n\r");
		BSP_LED_On(LED1);
		HAL_Delay(200);
		BSP_LED_Off(LED1);
	} else {
		printf(">>> %d HARDWARE FAILURES DETECTED. <<< \n\r\n\r", failures);
		for (int i = 0; i < 3; i++) {
			BSP_LED_Toggle(LED3);
			HAL_Delay(100);
		}
	}
}

#endif // End DEBUG
