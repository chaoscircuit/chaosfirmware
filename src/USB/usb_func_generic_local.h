/**
 * \file usb_func_generic_local.h
 * \brief Microchip generic function driver local header
 * 
 * This file contains declarations local to the generic USB device function.
 */

#ifndef _USBGEN_LOCAL_H_
#define _USBGEN_LOCAL_H_

#include <usb\usb.h>

/* Generic USB Function Data
 *************************************************************************
 * This structure maintains the data necessary to manage the generic USB
 * device function.
 */
 
typedef struct _generic_usb_fun_data
{
    BYTE    flags;      // Current state flags.
    BYTE    rx_size;    // Number of bytes received.
    BYTE    ep_num;     // Endpoint number.

} GEN_FUNC, *PGEN_FUNC;

// Generic USB Function State Flags:
#define GEN_FUNC_FLAG_TX_BUSY       0x01    // Tx is currently busy
#define GEN_FUNC_FLAG_RX_BUSY       0x02    // Rx is currently busy
#define GEN_FUNC_FLAG_RX_AVAIL      0x04    // Data has been received
#define GEN_FUNC_FLAG_INITIALIZED   0x80    // Function initialized


#endif  // _USBGEN_LOCAL_H_
/*************************************************************************
 * EOF usbgen_local.h
 */

