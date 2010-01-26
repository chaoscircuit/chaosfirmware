/**
 * \file usb_device_generic.h
 * \brief Microchip generic device driver header
 *
 * This file defines the USB Generic Device driver's interface.
 */

#ifndef _USB_GENFUNC_H_
#define _USB_GENFUNC_H_

#include <GenericTypeDefs.h>
#include <USB\usb.h>

/** D E V I C E   L A Y E R   I N T E R F A C E ******************************/

/*
 Pointers to these functions must be placed in the application's function-
 driver table.
 */

BOOL USBGenInitialize ( unsigned long flags );
BOOL USBGenEventHandler ( USB_EVENT event, void *data, unsigned int size );

/** P U B L I C  P R O T O T Y P E S *****************************************/

/******************************************************************************
 Function:        BOOL USBGenIsAttached(void)

 PreCondition:    None

 Input:           None

 Output:          Returns TRUE if the generic device is attached to a USB host.
                  Returns FALSE if not attached to a host.

 Side Effects:    None

 Overview:        This routine is used to check if the device is currently
                  attached to a host. 

 Note:            None
 *****************************************************************************/

BOOL USBGenIsAttached( void );


/******************************************************************************
 Function:        BOOL USBGenRxIsBusy(void)

 PreCondition:    None

 Input:           None

 Output:          None

 Side Effects:    None

 Overview:        This routine is used to check if the OUT endpoint is
                  busy (owned by SIE) or not.
                  Typical Usage: if(mUSBGenRxIsBusy())

 Note:            None
 *****************************************************************************/

BOOL USBGenRxIsBusy( void );


/******************************************************************************
 Function:        BOOL USBGenTxIsBusy(void)

 PreCondition:    None

 Input:           None

 Output:          None

 Side Effects:    None

 Overview:        This routine is used to check if the IN endpoint is
                  busy (owned by SIE) or not.
                  Typical Usage: if(mUSBGenTxIsBusy())

 Note:            None
 *****************************************************************************/

BOOL USBGenTxIsBusy( void );


/******************************************************************************
 Function:        byte USBGenGetRxLength(void)

 PreCondition:    None

 Input:           None

 Output:          USBGenGetRxLength returns usbgen_rx_len

 Side Effects:    None

 Overview:        USBGenGetRxLength is used to retrieve the number of bytes
                  copied to user's buffer by the most recent call to
                  USBGenRead function.

 Note:            None
 *****************************************************************************/

BYTE USBGenGetRxLength( void );


/******************************************************************************
 Function:        void USBGenWrite(bytebuffer, byte len)

 Preconditions:   1. USBInitialize must have been called to initialize 
                  the USB SW Stack.

                  2. The host must have configured the system as a USB
                  device that includes the Microchip General function
                  interface. 

                  3. USBGenTxIsBusy() must return false.

 Input:           buffer  : Pointer to the starting location of data bytes
                  len     : Number of bytes to be transferred

 Output:          None

 Side Effects:    If no Tx transfer was started, a new one has been.

 Overview:        Use this macro to transfer data located in data memory.

                  Remember: USBGenTxIsBusy() must return false before user
                  can call this function.
                  Unexpected behavior will occur if this function is called
                  when USBGenTxIsBusy() != 0

                  Typical Usage:
                  if(!USBGenTxIsBusy())
                      USBGenWrite(buffer, 3);

 Note:            None
 *****************************************************************************/

void USBGenWrite(BYTE *buffer, unsigned int len);


/******************************************************************************
 Function:        byte USBGenRead(bytebuffer, byte len)

 Preconditions:   1. USBInitialize must have been called to initialize 
                  the USB SW Stack.

                  2. The host must have configured the system as a USB
                  device that includes the Microchip General function
                  interface. 

                  Input argument 'buffer' should point to a buffer area that
                  is bigger or equal to the size specified by 'len'.

 Input:           buffer  : Pointer to where received bytes are to be stored
                  len     : The number of bytes expected.

 Output:          The number of bytes copied to buffer.

 Side Effects:    Once USBGenRead is called, subsequent retrieval of
                  usbgen_rx_len can be done by calling 
                  USBGenGetRxLength().

 Overview:        USBGenRead copies a string of bytes received through
                  the OUT endpoint to a user's specified location. 
                  It is a non-blocking function. It does not wait
                  for data if there is no data available. Instead it returns
                  '0' to notify the caller that there is no data available.

 Note:            If the actual number of bytes received is larger than the
                  number of bytes expected (len), only the expected number
                  of bytes specified will be copied to buffer.
                  If the actual number of bytes received is smaller than the
                  number of bytes expected (len), only the actual number
                  of bytes received will be copied to buffer.
 *****************************************************************************/

BYTE USBGenRead(BYTE *buffer, unsigned int len);


/** O B S O L E T E **********************************************************/

#define USBGenInitEP
#define mUSBGenRxIsBusy    USBGenRxIsBusy
#define mUSBGenTxIsBusy    USBGenTxIsBusy
#define mUSBGenGetRxLength USBGenGetRxLength


#endif //_USB_GENFUNC_H_
