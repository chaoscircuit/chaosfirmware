/**
 * \file mdac.c
 * \brief Routines for controlling the MDAC
 */

#include "mdac.h"
#include "globals.h"

#define SPI_PORT IOPORT_G
#define SS2 BIT_9
#define SDO BIT_8
#define SCLK BIT_6

#define SEND_DELAY 4
#define BIT_DELAY 1
#define DELAY 1

#define CLK_HIGH PORTWrite(SPI_PORT, SCLK)
#define CLK_LOW PORTClearBits(SPI_PORT, SCLK)

#define RESET_VALUE 4095
#define SW_SPI

int MDAC_value;

static void delay(long time);

void MDAC_setValue(word value) {
    /**
     * Set the MDAC value
     */
    // Update the MDAC value variable
    MDAC_value = value;
    
    // Check the bounds
    if(MDAC_value > 4095) {
        MDAC_value = 4095;
    }
    
    if(MDAC_value < 0) {
        MDAC_value = 0;
    }
    
    // Update the MDAC
    MDAC_send(0x1000 | MDAC_value);
    DBG_WriteInt(MDAC_value);
    #ifdef DEBUG
        // This line tells the MDAC to read back the value.
        MDAC_send(0x2000);
    #endif
}

void MDAC_increment(enum MDAC_StepSize size) {
    /**
     * Increment the MDAC value
     */
    MDAC_setValue(MDAC_value + size);
}

void MDAC_decrement(enum MDAC_StepSize size) {
    /**
     * Decrement the MDAC value
     */
    MDAC_setValue(MDAC_value - size);
}

void MDAC_resetValue() {
    /**
     * Reset the MDAC value
     */
    MDAC_setValue(RESET_VALUE);
}

#ifdef SW_SPI
void MDAC_init(void) {
    /**
     * Initialize the MDAC
     *
     * This sets the MDAC to its reset value and inits the SPI module
     */
    //Initialize MDAC value
    MDAC_value = RESET_VALUE;
    
    //Initialize Ports for SPI
    PORTSetPinsDigitalOut(SPI_PORT, SS2 | SDO | SCLK);
    PORTWrite(SPI_PORT, SS2);
    MDAC_send(0x9000); // Daisy Chain disable
    MDAC_send(0x1000 | MDAC_value); // Set MDAC value to 0
}

void MDAC_send(word data) {
    /**
     * Send a command to the MDAC
     *
     * This is done using SPI.
     */
    int i;
    // Set slave select low (select the MDAC)
    PORTClearBits(SPI_PORT, SS2);
    delay(SEND_DELAY);
    
    // Set the clock and data lines low
    PORTClearBits(SPI_PORT, SCLK | SDO);
    delay(BIT_DELAY);
    
    // Loop through the data to send
    for(i = 0; i < 16; i++) {
        delay(BIT_DELAY);
        
        // Set the clock high
        CLK_HIGH;
        
        // Send the MSB
        if(((data >> 15) & 0x01) == 1) {
            PORTWrite(SPI_PORT, SDO);
        } else {
            PORTClearBits(SPI_PORT, SDO);
        }
        
        // Shift the data up
        data = data  << 1;
        delay(BIT_DELAY);
        
        // Set clock low to advance the bit to the MDAC
        CLK_LOW;
    }
    delay(BIT_DELAY);
    
    // Set slave select high to lock final value into MDAC
    PORTWrite(SPI_PORT, SS2);
}

static void delay(long time) {
    /**
     * Simple for loop delay routine
     */
     long i;
    long d = time * DELAY;
     for (i=0; i<d; i++){
     }
}
#endif

#ifndef SW_SPI 

void MDAC_init(void) {
    /**
    * Initializes the MDAC by setting up spi and sending command to the
    * MDAC to not forward messages.
    */
    SpiChnOpen(2, SPI_CON_ON | SPI_CON_MSTEN | SPI_CON_MODE16 | SPI_CON_SMP, 64);
    MDAC_send(0x9000); // Daisy Chain disable
}

void MDAC_send(word data) {
    /**
    * Sends a word (16 bits) of data to the MDAC
    */
    PORTClearBits(SPI_PORT, SS2);
    SpiChnPutC(2, data);
    while(!SpiChnDataRdy(2)){};
    PORTWrite(SPI_PORT, SS2);
}
#endif
