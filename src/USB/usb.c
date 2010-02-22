/**
 * \file usb.c
 * \brief Interface to the USB library
 */

#include "usb.h"

BYTE USB_send_buf[64];
struct USB_command_packet USB_command;

void USB_init() {
    /**
     * Initialize the USB stack
     */
    USBDEVInitialize(0);
}

int USB_getNextCommand(void) {
    /**
     * Get the next command from the PC
     */
    return(USBGenRead((byte*)&USB_command,8) == 8);
}

void USB_sendAck() {
    /** 
     * Send 1 byte acknowledgement packet to the PC
     */
    if(!mUSBGenTxIsBusy()) {
        USB_send_buf[0] = 0x01;
        USBGenWrite(USB_send_buf,1);
    }
    return;
}


void USB_sendVersion() {
    /** 
     * Send version number to the PC
     */
    if(!mUSBGenTxIsBusy()) {
        *(int*)&USB_send_buf[0] = VERSION;
        USBGenWrite(USB_send_buf,4);
    }
    return;
}

void USB_sendRaw(byte* address, int length) {
    /**
     * Send raw data over USB
     */
    USBGenWrite(address, length);
}

void USB_sendStatus() {
    /**
     * Send a status packet over USB
     */
    if(!mUSBGenTxIsBusy()) {
        *(int*)&USB_send_buf[0] = MDAC_value;
        USB_sendRaw(USB_send_buf,4);
    }
}

void USB_sendPingReply() {
    /**
     * Send a reply to a ping request
     *
     * The ping size is pulled from the command retrieved during the 
     * last call of getNextCommand.
     *
     * This just sends a packet of ping_size bytes filled with 0x55s
     */
     
    int i;
     
    if(!mUSBGenTxIsBusy()) {
        if ( USB_command.ping_size > 64 ) {
            USB_command.ping_size = 64;
        }		
        for(i = 0; i<USB_command.ping_size; i++) {			
            USB_send_buf[i] = 0x55;
        }
        USB_sendRaw(USB_send_buf,USB_command.ping_size);
    }
}

void USB_handleEvents() {
    /**
     * Handle processing for the USB module
     */
    USBHALHandleBusEvent();
}
