/**
 * \file usb_device.h
 * \brief Microchip USB device abstration header
 */
 
#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include "GenericTypeDefs.h"

// *****************************************************************************
// *****************************************************************************
// Section: Application Program Interface
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/* Endpoint Configuration Data

This structure contains the data necessary to configure an endpoint as desired.
The application must define one or more table(s) (arrays) of these structures 
describing all possible endpoint configurations for this device (i.e. every 
endpoint for every interface (and alternate interface setting) for each device 
configuration available.

Note: Within a single device configuration, there may be no duplicate endpoints.
Thus, device config number and endpoint number should be sufficient to identify
a particular endpoint configuration.  However, it is possible for an interface 
to have different alternate settings for single endoint, so it is also necessary
to know the alternate interface number.
*/
 
typedef struct _endpoint_configuration_data
{
    UINT16  max_pkt_size;   // Maximum packet size for this endpoint
    UINT16  flags;          // Configuration flags for this endpoint (see below)
    BYTE    config;         // Configuration number (start at 1)
    BYTE    ep_num;         // Endpoint number.
    BYTE    intf;           // Interface number
    BYTE    alt_intf;       // Alternate interface setting (default=0)
    BYTE    function;       // Index in device function table (see FUNC_DRV)

} EP_CONFIG, *PEP_CONFIG;

// Endpoint Configuration Flags (all others are reserved).
#if defined(__18CXX)
    #define USB_EP_TRANSMIT     0x0400  // Enable EP for transmitting data
    #define USB_EP_RECEIVE      0x0200  // Enable EP for receiving data
    #define USB_EP_HANDSHAKE    0x1000  // Non-isoch endpoints use ACK/NACK
    #define USB_EP_NO_INC       0x0010  // Use for DMA to another device FIFO
#else
    #define USB_EP_TRANSMIT     0x0400  // Enable EP for transmitting data
    #define USB_EP_RECEIVE      0x0800  // Enable EP for receiving data
    #define USB_EP_HANDSHAKE    0x0100  // Non-isoch endpoints use ACK/NACK
    #define USB_EP_NO_INC       0x0010  // Use for DMA to another device FIFO
#endif


// *****************************************************************************
// Section: Device Layer API - Call Out Routines
// *****************************************************************************
/*
The following functions must be implemented by modules outside the USB Device 
support.
*/                                

// *****************************************************************************
/* Function Driver Table Structure

Forward reference (see structure definition, below).
*/
typedef struct _function_driver_table_entry FUNC_DRV, *PFUNC_DRV;


/*******************************************************************************
Function:       const void *USB_DEV_GET_DESCRIPTOR_FUNC ( PDESC_ID desc, 
                        unsigned int *length )

Preconditions:  Assumes that the USB SW stack has been initialized.

Overview:       This function is a "call out" from the USB FW stack that must be
                implemented by the application.  The USB device support will 
                call it in response to GET_DESCRIPTOR setup requests in order to
                provide the host with the desired descriptor data.

Input:          PDESC_ID desc - Data structure identifying which descriptor is 
                                being requested.

Output:         unsigned int *length - Length of the requested descriptor.

Return Values:  A pointer to the descriptor.

Remarks:        Since this routine is implemented by the application, its name
                is identified to the device abstraction layer be defining the
                USB_DEV_GET_DESCRIPTOR_FUNC macro in the "usb_config.h" file.
                
                Example:  
                
                #define USB_DEV_GET_DESCRIPTOR_FUNC USBDEVGetDescriptor
*******************************************************************************/

// Descriptor ID Parameter "desc" Data Type
typedef struct _descriptor_id_data
{
    BYTE   index;   // Descriptor index (if more then one is possible)
    BYTE   type;    // Descriptor type ID (see usb_ch9.h, "USB Descriptors")
    UINT16 lang_id; // Language ID for string descriptors (see USB 2.0 spec)

} DESC_ID, *PDESC_ID;

const void *USB_DEV_GET_DESCRIPTOR_FUNC ( PDESC_ID desc, unsigned int *length );


/*******************************************************************************
Function:       const EP_CONFIG *USB_DEV_GET_EP_CONFIG_TABLE_FUNC ( 
                        int *num_entries )

Preconditions:  None

Overview:       This function is a "call out" from the USB FW stack that must be
                implemented by the application.  The USB device support will 
                call it when it needs the endpoint configuration table.

Input:          None - None

Output:         int *num_entries - Number of entries in the endpoint 
                                   configuration table.

Return Values:  A pointer to the endpoint configuration table.

Remarks:        Since this routine is implemented by the application, its name
                is identified to the device abstraction layer be defining the
                USB_DEV_GET_EP_CONFIG_TABLE_FUNC macro in the "usb_config.h" 
                file.
                
                Example:  
                
                #define USB_DEV_GET_EP_CONFIG_TABLE_FUNC \
                        USBDEVGetEpConfigurationTable
*******************************************************************************/

const EP_CONFIG *USB_DEV_GET_EP_CONFIG_TABLE_FUNC ( int *num_entries );


/*******************************************************************************
Function:       const FUNC_DRV *USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC ( void )

Preconditions:  None

Overview:       This function is a "call out" from the USB FW stack that must 
                be implemented by the application.  The USB device support will
                call it when it needs get the function driver table.
                
Input:          None - None

Output:         None - None

Return Values:  A pointer to the function driver table.

Remarks:        Since this routine is implemented by the application, its name
                is identified to the device abstraction layer be defining the
                USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC macro in the "
                usb_config.h" file.
                
                Example:  
                
                #define USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC \
                        USBDEVGetFunctionDriverTable
*******************************************************************************/

const FUNC_DRV *USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC ( void );


// *****************************************************************************
// *****************************************************************************
// Section: Function Driver Interface
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
Function:       typedef BOOL <App-Defined name> ( unsigned long flags )

Preconditions:  USBInitialize has been called, the USB FW stack is operating in
                device mode (see USB_SUPPORT_DEVICE), and the system has been 
                enumerated on the USB by the host.

Overview:       This is actually a data-type definition that defines the
                function-call signature for a routine.  The routine is a call 
                out from the device layer to a USB function driver.  It is 
                called when the system has been configured as a USB peripheral 
                device by the host.  Its purpose is to initialize and activate 
                the function driver.
                
Input:          unsigned long flags - Function-driver-specific initialization
                                      flags or data.  This is the "flags" value
                                      placed in the function driver table entry
                                      (see FUNC_DRV).

Output:         None - None

Return Values:  TRUE  - If the function driver was able be successfully 
                        initialized.
                FALSE - If the function driver could not be initialized 
                        successfully.

Remarks:        Since there may be multiple function drivers.  A table-driven
                method is used to allow the device abstraction layer to call 
                the appropriate initialization routine (or routines for a 
                multi-function device).  Thus, the name of this routine must
                be place into an application-defined table that is identified
                to the device abstraction layer using the 
                USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC macro.

                The device layer also calls out to each function driver's event 
                handling routine.  (See the FUNC_DRV data type and the 
                USB_EVENT_HANDLER function-pointer type in usb_common.h.)

                Example:

                const FUNC_DRV gDevFuncTable[] = 
                {
                    {   // Generic Function Driver
                        USBGenInitialize,
                        USBGenEventHandler,
                        USBGEN_EP_NUM
                    }
                };

                const FUNC_DRV *USBDEVGetFunctionDriverTable ( void )
                {
                    return gDevFuncTable;
                
                }

                #define USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC \
                        USBDEVGetFunctionDriverTable
*******************************************************************************/

typedef BOOL (*USBDEV_INIT_FUNCTION_DRIVER) ( unsigned long flags );



// *****************************************************************************
/* Function Driver Table Structure

This structure is used to define an entry in the function-driver table.  Each 
entry provides the information that the device layer needs to track a particular 
USB device function, including pointers to the interface routines that the USB 
function must implement.
*/

struct _function_driver_table_entry
{
    USBDEV_INIT_FUNCTION_DRIVER Initialize;     // Initialization-Routine Pointer
    USB_EVENT_HANDLER           EventHandler;   // Event-Handling-Routine Pointer
    BYTE                        flags;          // Initialization "flags" Parameter

};


// *****************************************************************************
// Section: Device Layer FDI Call "In" Routines (called directly)
// *****************************************************************************
/*
The following routines can be called by USB Device "Function" drivers to access
the USB and exchange data with the host.
*/                                

/*******************************************************************************
Function:       BOOL USBDEVSignalResume ( void )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack.

Overview:       This routine sends USB resume signaling to the host.

Input:          None - None

Output:         None - None

Return Values:  TRUE  - If resume signaling was enabled.
                FALSE - If resume signaling was disabled.

Remarks:        This routine starts resume signaling.  The HAL will handle
                turning it off when appropriate.
*******************************************************************************/

BOOL USBDEVSignalResume ( void );


/*******************************************************************************
Function:       unsigned long USBDEVGetLastError ( void )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack.

Overview:       This routine provides a bitmap of the most recent error 
                conditions to occur.

Input:          None - None

Output:         None - None

Return Values:  USBDEV_PID_ERR  - Packet ID Error
                USBDEV_CRC16    - Data packet CRC error
                USBDEV_DFN8     - Data field size not n*8 bits
                USBDEV_BTO_ERR  - Bus turn-around timeout
                USBDEV_DMA_ERR  - DMA error, unable to read/write memory
                USBDEV_BTS_ERR  - Bit-stuffing error
                USBDEV_XFER_ID  - Unable to identify transfer EP
                USBDEV_NO_EP    - Invalid endpoint number
                USBDEV_DMA_ERR2 - Error starting DMA transaction

Remarks:        Although record of the error state is cleared, nothing is done
                to fix the condition or recover from the error.  The caller 
                must take appropriate steps.
*******************************************************************************/

#define USBDEVGetLastError USBHALGetLastError   // Implemented by the HAL

unsigned long USBDEVGetLastError ( void );

// USBDEVGetLastError Error Bits (defined by the HAL).
#define USBDEV_PID_ERR  USBHAL_PID_ERR  // Packet ID Error
#define USBDEV_CRC16    USBHAL_CRC16    // Data packet CRC error
#define USBDEV_DFN8     USBHAL_DFN8     // Data field size not n*8 bits
#define USBDEV_BTO_ERR  USBHAL_BTO_ERR  // Bus turn-around timeout
#define USBDEV_DMA_ERR  USBHAL_DMA_ERR  // DMA error, unable to read/write memory
#define USBDEV_BTS_ERR  USBHAL_BTS_ERR  // Bit-stuffing error
#define USBDEV_XFER_ID  USBHAL_XFER_ID  // Unable to identify transfer EP
#define USBDEV_NO_EP    USBHAL_NO_EP    // Invalid endpoint number
#define USBDEV_DMA_ERR2 USBHAL_DMA_ERR2 // Error starting DMA transaction


/*******************************************************************************
Function:       BOOL USBDEVTransferData ( TRANSFER_FLAGS flags, void *buffer, 
                        unsigned int size )

Preconditions:  1. USBInitialize must have been called.  
                2. The endpoint through which the data will be transferred 
                   must be configured as appropriate.
                3. The bus must have been enumerated.  Except for EP0 transfers.

Overview:       This routine prepares to transfer data on the USB.  The actual 
                transfer will not occur until the host peforms request to the 
                given endpoint.  

Input:          TRANSFER_FLAGS flags - Flags consists of the endpoint number OR'd
                                       with one or more flags indicating transfer
                                       direction and such (see "Data Transfer 
                                       Macros" in USBCommon.h):

                                       7 6 5 4 3 2 1 0 - Description
                                       | | | | \_____/
                                       | | | |    +----- Endpoint Number
                                       | | | +---------- Short or zero-size pkt
                                       | | +------------ Data Toggle 0/1
                                       | +-------------- Force Data Toggle
                                       +---------------- 1=Transmit/0=Receive

                unsigned int size    - Number of bytes of data to transfer.


Output:         void *buffer - Buffer to receive data.

Return Values:  TRUE  - If the device was able to successfully start the data
                        transfer.
                FALSE - If it was not able to start the transfer.

Remarks:        The device layer will continue the data transfer, keeping track
                of the buffer address and data remaining details internally.  
                The caller will receive notification that the transfer has 
                completed when the EVENT_TRANSFER event is passed into the 
                USB_EVENT_HANDLER call-out function.
*******************************************************************************/

#define USBDEVTransferData USBHALTransferData   // Implemented by the HAL

BOOL USBDEVTransferData ( TRANSFER_FLAGS    flags, 
                          void             *buffer, 
                          unsigned int      size      );


/*******************************************************************************
Function:       BOOL USBDEVStallPipe( TRANSFER_FLAGS pipe )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack.

Overview:       This routine stalls the given endpoint.

Input:          TRANSFER_FLAGS pipe - Uses the TRANSFER_FLAGS (see USBCommon.h)
                                      format to identify the endpoint and 
                                      direction making up the pipe to stall.
                                      
                                      Note: Only ep_num and direction fields are 
                                      required.

Output:         None - None

Return Values:  TRUE  - If able to stall the endpoint.
                FALSE - If not able to stall the endpoint.

Remarks:        None
*******************************************************************************/

#define USBDEVStallPipe USBHALStallPipe     // Implemented by the HAL

BOOL USBDEVStallPipe( TRANSFER_FLAGS pipe );


/*******************************************************************************
Function:       BOOL USBDEVUnstallPipe( TRANSFER_FLAGS pipe )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack and USBDEVStallPipe must have been called on the given 
                pipe.

Overview:       This routine clears the stall condition for the given pipe.

Input:          TRANSFER_FLAGS pipe - Uses the TRANSFER_FLAGS (see USBCommon.h)
                                      format to identify the endpoint and 
                                      direction making up the pipe to stall.
                                      
                                      Note: Only ep_num and direction fields are 
                                      required.

Output:         None - None

Return Values:  TRUE  - If able to clear the stall condition.
                FALSE - If not able clear the stall condition.

Remarks:        None
*******************************************************************************/

#define USBDEVUnstallPipe USBHALUnstallPipe     // Implemented by the HAL

BOOL USBDEVUnstallPipe( TRANSFER_FLAGS pipe );


/*******************************************************************************
Function:       BOOL USBDEVInitialize ( unsigned long flags )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack and USBDEVStallPipe must have been called on the given 
                pipe.

Overview:       This routine performs the basic initialization of the USB 
                device-side FW stack, including initialization of the HAL.

Input:          unsigned long flags - Reserved

Output:         None - None

Return Values:  TRUE  - If successful.
                FALSE - If not successful.

Remarks:        This routine should not be called directly.  It is normallly 
                called indirectly, through the "USBInitialize" interface.
*******************************************************************************/

BOOL USBDEVInitialize ( unsigned long flags );


// *****************************************************************************
// *****************************************************************************
// Section: Device Abstraction Layer Interface to the HAL
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
Function:       BOOL USBDEVHandleBusEvent ( USB_EVENT event, void *data, 
                        int size )

Preconditions:  USBInitialize must have been called to initialize the USB FW 
                stack.

Overview:       This routine is a called by the HAL to handle bus events as 
                they occur.  Events are identified by the "event" parameter
                and may have associated data (such as a setup request packet).

Input:          USB_EVENT event - Identifies the bus event that occured
                void *data      - Pointer to event-specific data
                int size        - Size of the event-specific data

Output:         None - None

Return Values:  TRUE  - If the event has been handled
                FALSE - If the event has not been completely handled

Remarks:        This routine should not be called by the application.  It is
                only called by the HAL.
*******************************************************************************/

BOOL USBDEVHandleBusEvent ( USB_EVENT event, void *data, int size );


/*******************************************************************************
Function:       BOOL HandleDeviceRequest ( void )

Preconditions:  1. USBInitialize must have been called.  
                2. The device must have attached to the USB.
                3. Endpoint 0 must have been appropriately configured.
                4. A setup request must have been received.

Overview:       This routine is a called by the HAL to handle setup requests 
                when they are received.  It handles standard device requests 
                itself and calls device-function driver(s) to handle all others.

Input:          USB_EVENT event - Identifies the bus event that occured
                void *data      - Pointer to event-specific data
                int size        - Size of the event-specific data

Output:         None - None

Return Values:  TRUE  - If the request has been handled
                FALSE - If the request has not been completely handled

Remarks:        This routine should not be called by the application.  It is
                only called by the HAL.
*******************************************************************************/

BOOL HandleDeviceRequest ( void );


/*******************************************************************************
Compliance Testing Macros.  Define these as desired to facilitate USB 
compliance testing.
*******************************************************************************/

// Called immediately after the device enters the default state.
#ifndef USBCompliance_SignalDeviceIsInDefaultState
    #define USBCompliance_SignalDeviceIsInDefaultState()
#endif


#endif  // _USB_DEVICE_H_
/*************************************************************************
 * EOF
 */

