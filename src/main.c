/**
 * \file main.c
 * \brief Main routine and initialization
 */
 
#include <peripheral/int.h>
#include <peripheral/ports.h>
#include <plib.h>
#include "USB/usb.h"
#include "GenericTypeDefs.h"
#include "led.h"
#include "adc.h"
#include "encoder.h"
#include "chaos.h"
#include "tone.h"
/**********************
 * Configuration Bits *
 **********************/

#ifndef OVERRIDE_CONFIG_BITS
        
    #pragma config UPLLEN   = ON            // USB PLL Enabled
    #pragma config FPLLMUL  = MUL_20        // PLL Multiplier
    #pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider
    #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
    #pragma config FPLLODIV = DIV_2         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer 
    #pragma config WDTPS    = PS2048        // Watchdog Timer Postscale
    #pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = HS            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
    #pragma config DEBUG    = ON            // Background Debugger Enable
            
#endif // OVERRIDE_CONFIG_BITS

void init(void) {
    /**
     * Initialize the system
     *
     * This simply calls init for each of the subsystems and configures 
     * the microcontroller.
     */
    SYSTEMConfig(SYS_CLOCK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    USB_init();
    LED_init();
    ADC_init();
    ENC_init();
    DBG_init();
    MDAC_init();
    TONE_init();
    TMR2_init();
    CHAOS_init();
    
}

int main ( void ) {
    /**
     * Main routine
     *
     * The main program execution happens here. Everything gets
     * initialized, and then the main loop simply checks for commands 
     * from the PC and processes them. 
     * 
     * Checks for the watchdog timer reset and outputs a warning message
     * to the debugger.
     *
     * The user interface is entirely interrupt driven (see encoder.c), 
     * so the main loop is very clean and concise.
     *
     * The ADC is also working hard in the background to fill buffers
     * which can then be sent out over USB. This is handled in adc.c.
     *
     * Finally, we reset 2 second the watchdog timer here because if the
     * main loop is running, nothing is hanging.
     */
    // initialize everything
    init();

    // check if this was a watchdog reset
    if ( ReadEventWDT() )
    {
        // A WDT event did occur
        DisableWDT();
        ClearEventWDT();    // clear the WDT event flag so a subsequent event can set the event bit
        DBG_WriteString("WARNING: Watchdog timer forced a device reset.\r\n");
    }

    EnableWDT(); // enable the WDT

    // Main Processing Loop
    while(1)
    {
        // Check USB for events and handle them appropriately.
        USB_handleEvents();
        // Check for a new command for the PC
        if(USB_getNextCommand()) {
            // run the specified command if we got one
            switch(USB_command.command) {
                case CMD_ping:
                    USB_sendPingReply();
                    break;
                case CMD_status:
                    USB_sendStatus();
                    break;
                case CMD_LED_test:
                    LED_test();
                    USB_sendAck();
                    break;
                case CMD_reset:
                    USB_sendAck();
                    break;
                case CMD_start_sample:
                    SMP_start(USB_command.mdac_value);
                    USB_sendAck();
                    break;
                case CMD_end_sample:
                    SMP_end();
                    USB_sendAck();
                    break;
                case CMD_get_data:
                    USB_sendRaw(SMP_getNextSendBuffer(),1024);
                    break;
                case CMD_set_mdac:
                    MDAC_setValue(USB_command.mdac_value);
                    USB_sendAck();
                    break;
                case CMD_get_version:
                    USB_sendVersion();
                    break;
                default: 
                    break;
            }
        }

        ClearWDT(); // Service the WDT
        
    }

    return 0;
}

