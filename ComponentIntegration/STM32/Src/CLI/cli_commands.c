/* In Core/Src/CLI/cli_commands.c */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h" // For osPriority definitions
#include "semphr.h"
#include "FreeRTOS_CLI.h"
#include <string.h>
#include <stdio.h> // For printf and snprintf
#include "main.h"

#include "stm32h7xx_nucleo.h" // For LED Access
/*-----------------------------------------------------------*/
/* FORWARD DECLARATIONS                      */
/*-----------------------------------------------------------*/

static BaseType_t prvHelloCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString);

static BaseType_t prvToggleLEDCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString);

static BaseType_t prvUARTSendCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString);

/* The implementation of the byte-by-byte echo test command. */
static BaseType_t prvEchoSendCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString);

/* The implementation for setting PID values. */
static BaseType_t prvSetPIDCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString);

/* The implementation for getting PID values. */
static BaseType_t prvGetPIDCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString);

/*-----------------------------------------------------------*/
/* COMMAND DEFINITIONS                      */
/*-----------------------------------------------------------*/

static const CLI_Command_Definition_t xHelloCommand = { "hello",
		"hello: Responds with 'Hello, World!'\r\n", prvHelloCommand, 0 };

static const CLI_Command_Definition_t xToggleLEDCommand =
		{ "toggle-led", "toggle-led: Toggles the green LED on/off.\r\n",
				prvToggleLEDCommand, 0 };

static const CLI_Command_Definition_t xUARTSendCommand = { "uart_send",
		"uart_send <string>: Sends a string (with newline) via UART7.\r\n",
		prvUARTSendCommand, 1 };

/* The structure for the "echo_send" command. */
static const CLI_Command_Definition_t xEchoSendCommand = {
		"echo_send", // The command name
		"echo_send <string>: Sends/Receives one byte at a time on UART7.\r\n"
				"                  (Requires a loopback or echo device).\r\n",
		prvEchoSendCommand, // The callback function to run
		1                   // Expects exactly one parameter
		};

static const CLI_Command_Definition_t xSetPIDCommand = {
		"set_pid", // The command name
		"set_pid <Kp> <Ki> <Kd> <Mode>: Sends 4 floats to the ESP32.\r\n",
		prvSetPIDCommand, // The callback function
		4                   // Expects exactly 4 parameters
		};

/* Structure for the "get_pid" command. */
static const CLI_Command_Definition_t xGetPIDCommand = {
		"get_pid", // The command name
		"get_pid: Requests current PID values from the ESP32.\r\n",
		prvGetPIDCommand, // The callback function
		0                   // Expects 0 parameters
		};

/*-----------------------------------------------------------*/
/* STATIC DATA & EXTERNS                      */
/*-----------------------------------------------------------*/
static uint8_t led_state = 0;
extern UART_HandleTypeDef huart7;
#define MAX_COMMAND_BUFFER_SIZE 256 // Renamed from MAX_PARAM_BUFFER_SIZE

/*-----------------------------------------------------------*/
/* COMMAND IMPLEMENTATIONS                      */
/*-----------------------------------------------------------*/

static BaseType_t prvHelloCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString) {
	(void) pcCommandString;
	strncpy(pcWriteBuffer, "Hello, World!\r\n", xWriteBufferLen);
	pcWriteBuffer[xWriteBufferLen - 1] = '\0';
	return pdFALSE;
}
static BaseType_t prvToggleLEDCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString) {
	(void) pcCommandString;
	if (led_state == 0) {
		BSP_LED_On(LED_GREEN);
		led_state = 1;
		strncpy(pcWriteBuffer, "LED Turned ON.\r\n", xWriteBufferLen);
	} else {
		BSP_LED_Off(LED_GREEN);
		led_state = 0;
		strncpy(pcWriteBuffer, "LED Turned OFF.\r\n", xWriteBufferLen);
	}
	pcWriteBuffer[xWriteBufferLen - 1] = '\0';
	return pdFALSE;
}

/* This command is the "fire and forget" version */
static BaseType_t prvUARTSendCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString) {
	const char *pcParameter;
	BaseType_t xParameterStringLength;
	HAL_StatusTypeDef status_tx;
	uint8_t tx_buffer[MAX_COMMAND_BUFFER_SIZE];
	size_t total_len = 0;
	memset(pcWriteBuffer, 0x00, xWriteBufferLen);
	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1,
			&xParameterStringLength);
	if (pcParameter == NULL || xParameterStringLength == 0) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Missing or empty string parameter.\r\n");
		return pdFALSE;
	}
	if (xParameterStringLength + 1 > MAX_COMMAND_BUFFER_SIZE) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Parameter is too long (max %d bytes).\r\n",
				MAX_COMMAND_BUFFER_SIZE - 1);
		return pdFALSE;
	}
	memcpy(tx_buffer, pcParameter, xParameterStringLength);
	tx_buffer[xParameterStringLength] = '\n'; // Add the newline!
	total_len = xParameterStringLength + 1;

	status_tx = HAL_UART_Transmit(&huart7, tx_buffer, total_len, 100);
	if (status_tx != HAL_OK) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: UART7 transmit failed (status: %d).\r\n", status_tx);
		return pdFALSE;
	}
	snprintf(pcWriteBuffer, xWriteBufferLen, "OK: Sent %u bytes.\r\n",
			(unsigned int) total_len);
	return pdFALSE;
}
/*
 * ==========================================================
 * BYTE-BY-BYTE COMMAND-RESPONSE (Replaces original prvEchoSendCommand)
 * ==========================================================
 */

/**
 * @brief Implements the "echo_send" command (Command-Response Protocol).
 *
 * This function takes one parameter (a string), transmits it
 * byte-by-byte over UART7, and appends a newline ('\n').
 *
 * It then enters a receive loop, reading 1 byte at a time until
 * a newline ('\n') is received or a timeout occurs.
 */
static BaseType_t prvEchoSendCommand(char *pcWriteBuffer,
		size_t xWriteBufferLen, const char *pcCommandString) {

	const char *pcParameter;
	BaseType_t xParameterStringLength;

	// We need local buffers
	uint8_t rx_buffer[MAX_COMMAND_BUFFER_SIZE];
	uint8_t sent_str_buf[MAX_COMMAND_BUFFER_SIZE]; // <-- FIX: Buffer for sent string

	uint32_t send_timeout = 100; // Timeout for sending each byte
	uint32_t resp_timeout = 200; // Timeout for *each* received byte
	uint16_t rx_count = 0;
	char newline = '\n';
	HAL_StatusTypeDef status_tx, status_rx;

	// 1. Get the string parameter from the CLI
	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1,
			&xParameterStringLength);

	// Clear the buffers
	memset(pcWriteBuffer, 0x00, xWriteBufferLen);
	memset(rx_buffer, 0x00, MAX_COMMAND_BUFFER_SIZE);
	memset(sent_str_buf, 0x00, MAX_COMMAND_BUFFER_SIZE); // <-- FIX: Clear new buffer

	if (pcParameter == NULL || xParameterStringLength == 0) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Missing or empty string parameter.\r\n");
		return pdFALSE;
	}

	if (xParameterStringLength > MAX_COMMAND_BUFFER_SIZE - 2) { // -2 for \n and \0
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Parameter is too long (max %d bytes).\r\n",
				MAX_COMMAND_BUFFER_SIZE - 2);
		return pdFALSE;
	}

	// --- FIX: Copy pcParameter to a local, null-terminated buffer ---
	memcpy(sent_str_buf, pcParameter, xParameterStringLength);
	sent_str_buf[xParameterStringLength] = '\0'; // This is the crucial fix.

	// --- 2. Send Phase (Byte-by-Byte, as requested) ---
	// (We can still use pcParameter here, as we loop using the correct length)
	for (uint16_t ii = 0; ii < xParameterStringLength; ii++) {
		status_tx = HAL_UART_Transmit(&huart7, (uint8_t*) &pcParameter[ii], 1,
				send_timeout);
		if (status_tx != HAL_OK) {
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: TX failed on byte %d (status: %d).\r\n", ii,
					status_tx);
			return pdFALSE; // Abort command
		}
	}

	// Send the newline terminator
	status_tx = HAL_UART_Transmit(&huart7, (uint8_t*) &newline, 1,
			send_timeout);
	if (status_tx != HAL_OK) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: TX failed on newline terminator (status: %d).\r\n",
				status_tx);
		return pdFALSE; // Abort command
	}

	// --- 3. Receive Phase (Byte-by-Byte until \n or timeout) ---
	while (rx_count < (MAX_COMMAND_BUFFER_SIZE - 1)) {
		status_rx = HAL_UART_Receive(&huart7, &rx_buffer[rx_count], 1,
				resp_timeout);

		if (status_rx == HAL_OK) {
			// Got a byte. Check if it's the terminator.
			if (rx_buffer[rx_count] == '\r') {
				continue; // Ignore carriage return
			}
			if (rx_buffer[rx_count] == '\n') {
				rx_count++;
				break;      // End of message
			}
			rx_count++;
		} else if (status_rx == HAL_TIMEOUT) {
			break; // End of message (timeout)
		} else {
			// This is a real error
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: RX failed (status: %d) after %d bytes.\r\n",
					status_rx, rx_count);
			return pdFALSE; // Abort command
		}
	}

	// Null-terminate the received string
	rx_buffer[rx_count] = '\0';

	// --- 4. Format the final report ---
	// Remove the trailing newline (if it exists) from the rx_buffer
	if (rx_count > 0 && rx_buffer[rx_count - 1] == '\n') {
		rx_buffer[rx_count - 1] = '\0';
	}

	// --- FIX: Use the null-terminated 'sent_str_buf' for printing ---
	snprintf(pcWriteBuffer, xWriteBufferLen, "OK.\r\n"
			"  Sent    : '%s' (%u bytes + \\n)\r\n"
			"  Received: '%s' (%u bytes)\r\n",
			(char*) sent_str_buf, // <-- Use the safe, local buffer
			(unsigned int) xParameterStringLength, (char*) rx_buffer,
			(unsigned int) strlen((char*) rx_buffer));

	return pdFALSE;
}

/*
 * ==========================================================
 * SET_PID COMMAND IMPLEMENTATION
 * ==========================================================
 */
static BaseType_t prvSetPIDCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString) {

	// Buffers for the 4 parameters (must be null-terminated)
	char p1_buf[16], p2_buf[16], p3_buf[16], p4_buf[16];
	// Pointers and lengths from FreeRTOS_CLIGetParameter
	const char *pcParameter1, *pcParameter2, *pcParameter3, *pcParameter4;
	BaseType_t xParam1Len, xParam2Len, xParam3Len, xParam4Len;

	// Buffer for the final command to send (e.g., "SET_PID_DATA 1.2,0.5...")
	char tx_command_buf[MAX_COMMAND_BUFFER_SIZE];
	// Buffer for the response from ESP32
	uint8_t rx_buffer[MAX_COMMAND_BUFFER_SIZE];
	uint16_t rx_count = 0;
	uint32_t send_timeout = 100;
	uint32_t resp_timeout = 200; // Wait 200ms for each byte of the response
	HAL_StatusTypeDef status_tx, status_rx;

	// 1. Clear buffers
	memset(pcWriteBuffer, 0x00, xWriteBufferLen);
	memset(rx_buffer, 0x00, MAX_COMMAND_BUFFER_SIZE);
	memset(tx_command_buf, 0x00, MAX_COMMAND_BUFFER_SIZE);

	// 2. Get all 4 parameters
	pcParameter1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParam1Len);
	pcParameter2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParam2Len);
	pcParameter3 = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParam3Len);
	pcParameter4 = FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParam4Len);

	// Check for missing parameters
	if (!pcParameter1 || !pcParameter2 || !pcParameter3 || !pcParameter4) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Expected 4 parameters.\r\n");
		return pdFALSE;
	}

	// Check for buffer overflow
	if (xParam1Len > 15 || xParam2Len > 15 || xParam3Len > 15
			|| xParam4Len > 15) {
		snprintf(pcWriteBuffer, xWriteBufferLen,
				"Error: Parameters are too long.\r\n");
		return pdFALSE;
	}

	// 3. Create local, null-terminated copies
	memcpy(p1_buf, pcParameter1, xParam1Len);
	p1_buf[xParam1Len] = '\0';
	memcpy(p2_buf, pcParameter2, xParam2Len);
	p2_buf[xParam2Len] = '\0';
	memcpy(p3_buf, pcParameter3, xParam3Len);
	p3_buf[xParam3Len] = '\0';
	memcpy(p4_buf, pcParameter4, xParam4Len);
	p4_buf[xParam4Len] = '\0';

	// 4. Format the TX command string
	snprintf(tx_command_buf, MAX_COMMAND_BUFFER_SIZE,
			"SET_PID_DATA %s,%s,%s,%s\n", p1_buf, p2_buf, p3_buf, p4_buf);

	// 5. Send Phase (Byte-by-Byte)
	size_t tx_len = strlen(tx_command_buf);
	for (uint16_t ii = 0; ii < tx_len; ii++) {
		status_tx = HAL_UART_Transmit(&huart7, (uint8_t*) &tx_command_buf[ii],
				1, send_timeout);
		if (status_tx != HAL_OK) {
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: TX failed on byte %d (status: %d).\r\n", ii,
					status_tx);
			return pdFALSE;
		}
	}

	// 6. Receive Phase (Byte-by-Byte, waiting for "OK\n" or "E_...\n")
	while (rx_count < (MAX_COMMAND_BUFFER_SIZE - 1)) {
		status_rx = HAL_UART_Receive(&huart7, &rx_buffer[rx_count], 1,
				resp_timeout);
		if (status_rx == HAL_OK) {
			if (rx_buffer[rx_count] == '\r') {
				continue;
			} // Ignore CR
			if (rx_buffer[rx_count] == '\n') { // End of message
				rx_count++;
				break;
			}
			rx_count++;
		} else if (status_rx == HAL_TIMEOUT) {
			break; // ESP32 didn't respond
		} else {
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: RX failed (status: %d).\r\n", status_rx);
			return pdFALSE;
		}
	}
	rx_buffer[rx_count] = '\0'; // Null-terminate

	// Remove trailing newline for clean printing
	if (rx_count > 0 && rx_buffer[rx_count - 1] == '\n') {
		rx_buffer[rx_count - 1] = '\0';
	}

	// 7. Report response to CLI
	snprintf(pcWriteBuffer, xWriteBufferLen, "ESP32 Response: '%s'\r\n",
			(char*) rx_buffer);
	return pdFALSE;
}

/*
 * ==========================================================
 * GET_PID COMMAND IMPLEMENTATION
 * ==========================================================
 */
static BaseType_t prvGetPIDCommand(char *pcWriteBuffer, size_t xWriteBufferLen,
		const char *pcCommandString) {

	(void) pcCommandString; // Unused parameter

	const char *tx_command = "GET_PID\n";
	size_t tx_len = strlen(tx_command);

	// Buffer for the response from ESP32
	uint8_t rx_buffer[MAX_COMMAND_BUFFER_SIZE];
	uint16_t rx_count = 0;
	uint32_t send_timeout = 100;
	uint32_t resp_timeout = 200; // Wait 200ms for each byte of the response
	HAL_StatusTypeDef status_tx, status_rx;

	// 1. Clear buffers
	memset(pcWriteBuffer, 0x00, xWriteBufferLen);
	memset(rx_buffer, 0x00, MAX_COMMAND_BUFFER_SIZE);

	// 2. Send Phase (Byte-by-Byte)
	for (uint16_t ii = 0; ii < tx_len; ii++) {
		status_tx = HAL_UART_Transmit(&huart7, (uint8_t*) &tx_command[ii], 1,
				send_timeout);
		if (status_tx != HAL_OK) {
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: TX failed on byte %d (status: %d).\r\n", ii,
					status_tx);
			return pdFALSE;
		}
	}

	// 3. Receive Phase (Byte-by-Byte, waiting for the long PID string)
	//    This is the exact same logic as prvEchoSendCommand's receive part
	while (rx_count < (MAX_COMMAND_BUFFER_SIZE - 1)) {
		status_rx = HAL_UART_Receive(&huart7, &rx_buffer[rx_count], 1,
				resp_timeout);
		if (status_rx == HAL_OK) {
			if (rx_buffer[rx_count] == '\r') {
				continue;
			} // Ignore CR
			if (rx_buffer[rx_count] == '\n') { // End of message
				rx_count++;
				break;
			}
			rx_count++;
		} else if (status_rx == HAL_TIMEOUT) {
			break; // ESP32 stopped sending
		} else {
			snprintf(pcWriteBuffer, xWriteBufferLen,
					"Error: RX failed (status: %d).\r\n", status_rx);
			return pdFALSE;
		}
	}
	rx_buffer[rx_count] = '\0'; // Null-terminate

	// Remove trailing newline for clean printing
	if (rx_count > 0 && rx_buffer[rx_count - 1] == '\n') {
		rx_buffer[rx_count - 1] = '\0';
	}

	// 4. Report response to CLI
	//    This time, the received buffer IS the final output
	snprintf(pcWriteBuffer, xWriteBufferLen, "ESP32 PID: %s\r\n",
			(char*) rx_buffer);
	return pdFALSE;
}

/*-----------------------------------------------------------*/
/* PUBLIC REGISTRATION & CREATION FUNCTIONS                 */
/*-----------------------------------------------------------*/

/**
 * @brief Function to register all CLI commands.
 */
void vInitializeCLIAndTasks(void) {
	// Register all the CLI commands
	FreeRTOS_CLIRegisterCommand(&xHelloCommand);
	FreeRTOS_CLIRegisterCommand(&xToggleLEDCommand);
	FreeRTOS_CLIRegisterCommand(&xUARTSendCommand);
	FreeRTOS_CLIRegisterCommand(&xEchoSendCommand);
	FreeRTOS_CLIRegisterCommand(&xGetPIDCommand);
	FreeRTOS_CLIRegisterCommand(&xSetPIDCommand);

}
