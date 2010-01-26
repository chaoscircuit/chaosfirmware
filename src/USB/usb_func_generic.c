/**
 * \file usb_func_generic.c
 * \brief Microchip generic device routines
 *
 * This file implements the generic device function.
 */

#include "GenericTypeDefs.h"
#include "USB/usb.h"
#include "usb_func_generic_local.h"

/***************
 * Global Data *
 ***************/
 
PRIVATE GEN_FUNC gGenFunc;  // State structure

 
/***************************
 * Local Utility Functions *
 ***************************/

/* HandleTransferDone
 *************************************************************************
 * This routine sets the appropriate state flags and data when a Tx or Rx
 * transfer finishes.
 */

PRIVATE inline BOOL HandleTransferDone( USB_TRANSFER_EVENT_DATA * xfer )
{

    #ifdef USB_SAFE_MODE
    
    if (xfer == NULL) {
        return FALSE;
    }

    #endif
    
    // Was it our endpoint?
    if ( xfer->flags.field.ep_num == gGenFunc.ep_num)
    {
        // Did a transmit transfer finish?
        if ( xfer->flags.field.direction == 1)  // Transmit
        {
            // Yes, clear the Tx flag.
            gGenFunc.flags &= ~GEN_FUNC_FLAG_TX_BUSY;
            return TRUE;
        }

        // Did a receive transfer finish?
        if ( xfer->flags.field.direction == 0)  // Receive
        {
            // Yes, Set the the Rx-data-available flag & record the size.
            gGenFunc.flags |= GEN_FUNC_FLAG_RX_AVAIL;
            gGenFunc.rx_size = (BYTE)xfer->size;
            return TRUE;
        }
    }

    // Otherwise, it's not ours.
    return FALSE;
}


/*****************************
 * Interface to Device Layer *
 *****************************/

/*************************************************************************
 Function:        USBGenInitialize
 
 Precondition:    USBDEVInitialize has been called and the system has 
                  been enumerated as a device on the USB.
 
 Input:           flags       Initialization flags
 
 Output:          none
 
 Returns:         TRUE if successful, FALSE if not.
 
 Side Effects:    The USB General function driver has been initialized.
 
 Overview:        This routine is a call out from the device layer to
                  the USB general function driver.  It is called when 
                  the system has been configured as a Microchip General
                  USB peripheral device by the host.  Its purpose is to 
                  initialize and activate the USB General function 
                  driver.
 
 Note:            There may be multiple function drivers.  If so, the
                  USB device layer will call the initialize routine
                  for each one of the functions that are in the selected
                  configuration.
 *************************************************************************/

PUBLIC BOOL USBGenInitialize ( unsigned long flags )
{
    // Initialize the Rx size
    gGenFunc.rx_size = 0;

    // Initialize the endpoint used.
    // (EP0 is invalid, default to 1).
    gGenFunc.ep_num  = (BYTE)(flags & 0xF);
    if (gGenFunc.ep_num == 0) {
        gGenFunc.ep_num = 1;
    }

    // Set initialized flag!
    gGenFunc.flags   = GEN_FUNC_FLAG_INITIALIZED;

    return TRUE;
}


/*************************************************************************
 Function:        USBGenEventHandler
 
 Preconditions:   1. USBInitialize must have been called to initialize 
                  the USB SW Stack.
 
                  2. The host must have configured the system as a USB
                  device that includes the Microchip General function
                  interface. 
                  
 Input:           event       Identifies the bus event that occured
 
                  data        Pointer to event-specific data
 
                  size        Size of the event-specific data
 
 Output:          none
 
 Returns:         TRUE if the event was handled, FALSE if not
 
 Side Effects:    Event-specific actions have been taken.
 
 Overview:        This routine is called by the "device" layer to notify
                  the general function of events that occur on the USB.
                  If the event is recognized, it is handled and the 
                  routine returns TRUE.  Otherwise, it is ignored and 
                  the routine returns FALSE.
 *************************************************************************/

PUBLIC BOOL USBGenEventHandler ( USB_EVENT event, void *data, unsigned int size )
{

    // Abort if not initialized.
    if ( !(gGenFunc.flags & GEN_FUNC_FLAG_INITIALIZED) ) {
        return FALSE;
    }

    // Handle specific events.
    switch (event)
    {
    case EVENT_TRANSFER:    // A USB transfer has completed.

        #ifdef USB_SAFE_MODE
        if (size == sizeof(USB_TRANSFER_EVENT_DATA)) {
            return HandleTransferDone((USB_TRANSFER_EVENT_DATA *)data);
        } else {
            return FALSE;
        }
        #else
        return HandleTransferDone((USB_TRANSFER_EVENT_DATA *)data);
        #endif

    case EVENT_SUSPEND:     // Device-mode suspend/idle event received
    case EVENT_DETACH:      // USB cable has been detached
        
        // De-initialize the general function driver.
        gGenFunc.flags   = 0;
        gGenFunc.rx_size = 0;
        return TRUE;

    case EVENT_RESUME:    // Device-mode resume received, re-initialize
        return USBGenInitialize(gGenFunc.ep_num);

        
    case EVENT_BUS_ERROR:   // Error on the bus, call USBDEVGetLastError()
        USBDEVGetLastError();
        // To Do: Capture the error and do something about it.
        return TRUE;

    default:            // Unknown event
        return FALSE;
    }

} // USBGenEventHandler


/******************************
 * Public Interface Functions *
 ******************************/

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

BOOL USBGenIsAttached( void )
{
    return gGenFunc.flags & GEN_FUNC_FLAG_INITIALIZED;
}


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
PUBLIC inline BOOL USBGenRxIsBusy( void )
{
    return gGenFunc.flags & GEN_FUNC_FLAG_RX_BUSY;
}


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
PUBLIC inline BOOL USBGenTxIsBusy( void )
{
    return gGenFunc.flags & GEN_FUNC_FLAG_TX_BUSY;
}


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
PUBLIC inline BYTE USBGenGetRxLength( void )
{
    return gGenFunc.rx_size;
}


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
PUBLIC void USBGenWrite( BYTE *buffer, unsigned int len )
{
    // Abort if not initialized.
    if ( !(gGenFunc.flags & GEN_FUNC_FLAG_INITIALIZED) ) {
        return;
    }

    #ifdef USB_SAFE_MODE
    
    // If there's not currently a Tx transfer in progress
    if ( !(gGenFunc.flags & GEN_FUNC_FLAG_TX_BUSY) )
    {

    #endif
    
        // Mark Tx as busy
        gGenFunc.flags |= GEN_FUNC_FLAG_TX_BUSY;

        // Call the device layer to start the data transfer.
        USBDEVTransferData(XFLAGS(USB_TRANSMIT|gGenFunc.ep_num), buffer, (unsigned int)len);


    #ifdef USB_SAFE_MODE
    
    }

    #endif
}


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
PUBLIC BYTE USBGenRead( BYTE *buffer, unsigned int len )
{
    // Abort if not initialized.
    if ( !(gGenFunc.flags & GEN_FUNC_FLAG_INITIALIZED) ) {
        return 0;
    }

    // If the Rx is busy...
    if (gGenFunc.flags & GEN_FUNC_FLAG_RX_BUSY)
    {
        // Report data available if we have any.
        if (gGenFunc.flags & GEN_FUNC_FLAG_RX_AVAIL)
        {
            // clear flags
            gGenFunc.flags &= ~(GEN_FUNC_FLAG_RX_BUSY|GEN_FUNC_FLAG_RX_AVAIL);
            return gGenFunc.rx_size;
        }
    }
    else    // Start a new Rx transfer
    {
        // Set busy flag & clear the old size.
        gGenFunc.flags   |= GEN_FUNC_FLAG_RX_BUSY;
        gGenFunc.rx_size  = 0;

        // Call the device layer to start the data transfer.
        USBDEVTransferData(XFLAGS(USB_RECEIVE|gGenFunc.ep_num), buffer, (unsigned int)len);
    }

    // Return 0 any time we don't have data.
    return 0;
}


/*************************************************************************
 * EOF usbgen.c
 */
                                                
