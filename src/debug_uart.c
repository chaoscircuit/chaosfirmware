/**
 * \file debug_uart.c
 * \brief Routines for interacting with the debug UART
 */

#include "debug_uart.h"
#include <string.h>
#include <stdlib.h>
#include "mdac.h"
#include "led.h"
#include "chaos.h"

#define COMMAND_SIZE 32
const int BAUD_RATE = 115000;

static char command[COMMAND_SIZE];

static void ProcessCommand(char* str);
static void PrintHelp();

void DBG_init(void) {
    /**
     * Initialize the debug UART
     * 
     * This should only need to be called once on boot up. This function
     * also does nothing if DEBUG is not defined in the header file.
     */
    #ifdef DEBUG
    int pClock = GetPeripheralClock();
    int BAUD_VALUE = ((pClock/16/BAUD_RATE)-1); 
    
    // Set up UART
    OpenUART1(UART_EN, UART_RX_ENABLE | UART_TX_ENABLE, BAUD_VALUE);
    
    // Enable interrupts
    ConfigIntUART1(UART_INT_PR2 | UART_RX_INT_EN);    
    
    // Write Startup String
    DBG_WriteString("****************UART 1 Initialized****************\r\n");
    PrintHelp();
    #endif
}

static void ProcessCommand(char* str) {
    /**
     * Process a command on the UART
     */
    if(strncmp(str, "mdac ", 5) == 0) {
    // Sets the MDAC value to the specified number
        DBG_WriteString("Setting custom mdac value.\r\n");
        int newValue = atoi(&str[4]);
        MDAC_setValue(newValue); // Set MDAC value to 0
    } else if(strncmp(str, "help", 4) == 0) {
    // Prints valid commands
        PrintHelp();
    } else if(strncmp(str, "reset", 5) == 0) {
    // Resets the device
        SoftReset();
    }  else if(strncmp(str, "ledtest", 7) == 0) {
    // Run the led test
        LED_test();
    } else if(strncmp(str, "chaoson", 7) == 0) {
    // Turn on the chaos circuit
        CHAOS_turnOn();
    } else if(strncmp(str, "chaosoff", 8) == 0) {
    // Turn off the chaos circuit
        CHAOS_turnOff();
    } else if(strncmp(str, "encen", 5) == 0) {
    // Enable the encoder interrupts
        ENC_intEnable();
    } else if(strncmp(str, "encdis", 6) == 0) {
    // Disable the encoder interrupts
        ENC_intDisable();
    } else if(strncmp(str, "ledchaos", 8) == 0) {
    // Toggles the chaos LED
        mChaosLED_Toggle();
    }
}

static void PrintHelp() {
    /**
     * Print the help on the debug commands
     */
    DBG_WriteString("\r\n*********Chaos Unit Debug UART Help***************\r\n");
    DBG_WriteString("\thelp\t\t-Prints this message.\r\n");
    DBG_WriteString("\tmdac #\t\t-Changes the value of the mdac to a specified number.\r\n");
    DBG_WriteString("\treset\t\t-Resets the Chaos Unit.\r\n");
    DBG_WriteString("\tledtest\t\t-Flashes the LEDs.\r\n");
    DBG_WriteString("\tchaoson\t\t-Powers on the Chaos circuitry.\r\n");
    DBG_WriteString("\tchaosoff\t-Powers down the Chaos circuitry.\r\n");
    DBG_WriteString("\tencen\t\t-Enables the encoder.\r\n");
    DBG_WriteString("\tencdis\t\t-Disables the encoder.\r\n");
    DBG_WriteString("\tledchaos\t-Toggles the chaos led.\r\n");
    
}

// UART 1 interrupt handler
void __ISR(_UART1_VECTOR, ipl2) IntUart1Handler(void) {
    /**
     * Handle UART1 interrupts
     */
    static int count = 0;
    if(mU1RXGetIntFlag()) {
        //Clear the RX interrupt Flag
        mU1RXClearIntFlag();
        
        // Read character
        char c = ReadUART1();
        
        // Echo what we just received.
        putcUART1(c);
        
        // Store character
        if (c == 0x7F || c == 0x08) { // backspace/delete
            count--;
        } else if (c == '\r') {
            command[count] = '\0';
            ProcessCommand(command);
            count = 0;
            putcUART1('\r');
            putcUART1('\n');
        } else {
            command[count] = c;
            count++;
            if(count >= COMMAND_SIZE) {
                count = 0;
            }
        }
    }
    
    if (mU1TXGetIntFlag()) {
        mU1TXClearIntFlag();
    }
}
