#include "cli.h"
#include "uart.h"
#include "flash_store.h"
#include "fan.h" // Forward declaration or include if fan methods needed
#include "ds18b20.h" // Forward declaration

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLI_BUFFER_SIZE 64

static char cli_rx_buffer[CLI_BUFFER_SIZE];
static uint8_t cli_rx_index = 0;

extern AppSettings current_settings;
extern float current_temperature;

void CLI_Init(void) {
    UART_PrintLn("CLI Initialized. Type 'help' for commands.");
}

static void CLI_ParseCommand(char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        UART_PrintLn("Available commands:");
        UART_PrintLn("  help            - Show this help");
        UART_PrintLn("  set_min <val>   - Set minimum temperature");
        UART_PrintLn("  set_max <val>   - Set maximum temperature");
        UART_PrintLn("  get_temp        - Get current temperature");
        UART_PrintLn("  get_settings    - Get current settings");
    } else if (strncmp(cmd, "set_min ", 8) == 0) {
        float val = strtof(cmd + 8, NULL);
        current_settings.min_temp = val;
        FlashStore_Save(&current_settings);
        char buf[32];
        sprintf(buf, "Min temp set to: %.2f", val);
        UART_PrintLn(buf);
    } else if (strncmp(cmd, "set_max ", 8) == 0) {
        float val = strtof(cmd + 8, NULL);
        current_settings.max_temp = val;
        FlashStore_Save(&current_settings);
        char buf[32];
        sprintf(buf, "Max temp set to: %.2f", val);
        UART_PrintLn(buf);
    } else if (strcmp(cmd, "get_temp") == 0) {
        char buf[32];
        sprintf(buf, "Current Temp: %.2f", current_temperature);
        UART_PrintLn(buf);
    } else if (strcmp(cmd, "get_settings") == 0) {
        char buf[64];
        sprintf(buf, "Min: %.2f, Max: %.2f", current_settings.min_temp, current_settings.max_temp);
        UART_PrintLn(buf);
    } else {
        UART_PrintLn("Unknown command");
    }
}

void CLI_Process(void) {
    while (UART_Available()) {
        uint8_t rx_byte = UART_Read();
        if (rx_byte == '\r' || rx_byte == '\n') {
            if (cli_rx_index > 0) {
                cli_rx_buffer[cli_rx_index] = '\0';
                UART_PrintLn(""); // Newline for echo
                CLI_ParseCommand(cli_rx_buffer);
                cli_rx_index = 0;
            }
        } else if (rx_byte == '\b' || rx_byte == 0x7F) { // Handle backspace
             if (cli_rx_index > 0) {
                 cli_rx_index--;
                 UART_Print("\b \b");
             }
        } else {
            if (cli_rx_index < CLI_BUFFER_SIZE - 1) {
                cli_rx_buffer[cli_rx_index++] = rx_byte;
                // Echo back
                char echo[2];
                echo[0] = rx_byte;
                echo[1] = '\0';
                UART_Print(echo);
            }
        }
    }
}
