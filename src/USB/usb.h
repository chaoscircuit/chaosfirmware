/**
 * \file usb.h
 * \brief Header file for usb.c
 */
 
#ifndef _USB_H_
#define _USB_H_

#include <plib.h>
#include "usb/usb_common.h"         // Common USB library definitions
#include "usb_config.h"             // Must be defined by the application
#include "usb/usb_ch9.h"            // USB device framework definitions
#include "usb/usb_hal.h"            // Hardware Abstraction Layer interface
#include "usb/usb_device.h"         // USB Device abstraction layer interface
#include "usb/usb_device_generic.h"
#include "usb/usb_commands.h"

#include "led.h"
#include "sampling.h"
#include "mdac.h"
#include "globals.h"

struct USB_command_packet {
    /// Command to run
    unsigned char command;
    /// Used for a ping request
    unsigned char ping_size;
    unsigned char unused_char1;
    unsigned char unused_char2;
    /// Used for sampling requests
    short int mdac_value;
    short int unused_short1;
};

extern struct USB_command_packet USB_command;

void USB_init(void);
int USB_getNextCommand(void);
void USB_sendAck(void);
void USB_sendRaw(byte* address, int length);
void USB_sendStatus();
void USB_sendPingReply();
void USB_handleEvents();

#endif
