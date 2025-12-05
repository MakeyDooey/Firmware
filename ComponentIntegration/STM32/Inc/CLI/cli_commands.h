// In Core/Inc/cli_commands.h
#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "FreeRTOS.h" // for BaseType_t

// Public function to register all commands
void vInitializeCLIAndTasks(void);

#endif // CLI_COMMANDS_H
