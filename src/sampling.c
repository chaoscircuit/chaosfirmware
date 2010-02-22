/**
 * \file sampling.c
 * \brief Routines related to sampling data from the chaos circuit
 */

#include "sampling.h"

BYTE SMP_SEND_BUF[8]; 

BYTE SMP_BUFFER_STATE[SMP_NUM_BUFFERS];
BYTE SMP_BUFFER[SMP_BUFFER_SIZE * SMP_NUM_BUFFERS];
int SMP_SAMPLE_BUFFER_NUM;
int SMP_SEND_BUFFER_NUM;
int SMP_MODE;
int SMP_PACKET_OFFSET;
int SMP_PACKET_ID;
int SMP_LAST_TRANSMISSION;

void SMP_init(void) {
    /**
     * Initialize the sampling module
     *
     * This function does very little. It simply sets a few variables to
     * approriate starting values and put the device in demonstration 
     * mode.
     */
    SMP_MODE = DEMONSTRATION;
    SMP_LAST_TRANSMISSION = 0;
}

void SMP_start(word mdac_value) {
    /**
     * Start a sample
     *
     * Starting a sample basically consists of the following:
     *
     * 1. Clear out the buffers and set state as not ready to send
     *
     * 2. Set the MDAC value if provided
     *
     * 3. Send an acknowledgement to the PC
     *
     * 4. Enter sampling mode
     */
    int i;
    
    // Clear out the buffers
    for ( i = 0 ; i < SMP_NUM_BUFFERS; i++ ) {
        SMP_BUFFER_STATE[i] = 0x00;
    }
    SMP_SAMPLE_BUFFER_NUM = 0;
    SMP_SEND_BUFFER_NUM = 0;
    SMP_PACKET_ID = 0;
    SMP_PACKET_OFFSET = 4;
    // set first id to 0
    SMP_BUFFER[0] = 0x00;
    SMP_BUFFER[1] = 0x00;
    SMP_BUFFER[2] = 0x00;
    SMP_BUFFER[3] = 0x00;
    
    // only set the mdac value if a valid value was provided
    if(mdac_value <= 4095 && mdac_value >= 0) {
        MDAC_setValue(mdac_value);
    }    
    
    // enter sampling mode
    SMP_LAST_TRANSMISSION = 0;
    SMP_MODE = SAMPLING;
    mDemonstration_LED_Off();
    ENC_intDisable();
}

void SMP_sendData(void) {
    /**
     * Send sampled data over USB
     * 
     * Waits for a buffer to be ready to send and then sends it
     */
    SMP_LAST_TRANSMISSION = 0;
    // write the data buffer to the USB
    if(!mUSBGenTxIsBusy()) {
        // wait for the send buffer to be ready to send
        while(!(SMP_BUFFER_STATE[SMP_SEND_BUFFER_NUM] & SMP_BUF_RTS));
        // send it
        USBGenWrite((BYTE*)(SMP_BUFFER+(SMP_SEND_BUFFER_NUM*1024)),1024);
        // unset RTS for the previous buffer
        SMP_BUFFER_STATE[(SMP_SEND_BUFFER_NUM+SMP_NUM_BUFFERS-1)%SMP_NUM_BUFFERS] &= ~SMP_BUF_RTS;
    } else {
        // ERROR
    }

    // move to the next buffer
    SMP_SEND_BUFFER_NUM = (SMP_SEND_BUFFER_NUM + 1) % SMP_NUM_BUFFERS;
}

byte* SMP_getNextSendBuffer(void) {
    /**
    * Sends a set of data over the USB buffer
    */
    byte* send_buffer;
    
    // reset the USB watchdog
    SMP_LAST_TRANSMISSION = 0;

    // wait for the send buffer to be ready to send
    while(!(SMP_BUFFER_STATE[SMP_SEND_BUFFER_NUM] & SMP_BUF_RTS));

    // get the buffer start address
    send_buffer = SMP_BUFFER+(SMP_SEND_BUFFER_NUM*1024);

    // unset RTS for the previous buffer
    SMP_BUFFER_STATE[(SMP_SEND_BUFFER_NUM+SMP_NUM_BUFFERS-1)%SMP_NUM_BUFFERS] &= ~SMP_BUF_RTS;

    // move to the next buffer
    SMP_SEND_BUFFER_NUM = (SMP_SEND_BUFFER_NUM + 1) % SMP_NUM_BUFFERS;
    
    return send_buffer;
}

void SMP_end(void) {
    /**
     * End a sample
     */
    
    // switch back to demonstration mode
    SMP_gotoDemonstrationMode();
}

void SMP_gotoDemonstrationMode(void) {
    /**
     * Go to demonstration mode
     */
    int i;
    for ( i = 0 ; i < SMP_NUM_BUFFERS; i++ ) {
        SMP_BUFFER_STATE[i] = 0x00;
    }
    SMP_SAMPLE_BUFFER_NUM = 0;
    SMP_SEND_BUFFER_NUM = 0;
    SMP_PACKET_ID = 0;
    SMP_PACKET_OFFSET = 4;
    // set first id to 0
    SMP_BUFFER[0] = 0x00;
    SMP_BUFFER[1] = 0x00;
    SMP_BUFFER[2] = 0x00;
    SMP_BUFFER[3] = 0x00;

    SMP_MODE = DEMONSTRATION;
    mDemonstration_LED_On();
    ENC_intEnable();
}
