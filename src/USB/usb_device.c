/**
 * \file usb_device.c
 * \brief Microchip USB Device abstration layer
 *
 * This file contains the routines necessary to communicate over USB as a 
 * device (as opposed to a host)
 */

#include <string.h>

#include "GenericTypeDefs.h"
#include "USB/usb.h"

#include "usb_device_local.h"


// Call Trace
//#define ENABLE_CALL_TRACE
#define NUM_CALLS               64
#define NAME_LEN                32
// NUM_CALLS * NAME_LEN = call trace buffer size

// State Trace
//#define ENABLE_STATE_TRACE
#define NUM_STATES              20

// Event Trace
//#define ENABLE_EVENT_TRACE
#define NUM_EVENTS              40

/***************
 * Global Data *
 ***************/
 
// Main device-layer state data structure.
USB_DEVICE_DATA gDEVData;

// Call Trace
#ifdef ENABLE_CALL_TRACE
char gCallTrace[NUM_CALLS][NAME_LEN];
int  gCTIndex;
#define mCALL_TRACE(s)  (memcpy(&gCallTrace[gCTIndex][0],(s),min(strlen((s)),NAME_LEN-1)), \
                         gCallTrace[gCTIndex][NAME_LEN-1]=0,                               \
                         gCTIndex = (gCTIndex + 1) % NAME_LEN)
#else
#define mCALL_TRACE(s)  
#endif

// State Trace
#ifdef ENABLE_STATE_TRACE
struct _state_trace
{
    EP0_STATE     state;
    XFER_FLAGS    flags;
} gStateTrace[NUM_STATES];
int gStateIndex;
#endif

// Event Trace
#ifdef ENABLE_EVENT_TRACE
typedef struct _event_trace
{
    EP0_STATE event;
    void *    data;
    int       size;
} EVENT_TRACE_DATA;
EVENT_TRACE_DATA gEventTrace[NUM_EVENTS];
int              gEventIndex;
#endif

/***************************
 * Local Utility Functions *
 ***************************/

/* InitializeDeviceState
 *************************************************************************
 * This routine clears the device state structure and sets initial values.
 */

PRIVATE BOOL InitializeDeviceState ( unsigned long init_flags )
{
    mCALL_TRACE("InitializeDeviceState");

    // Initialize the device layer
    memset(&gDEVData, 0, sizeof(gDEVData));
    gDEVData.init_flags = init_flags;

    // Initialize device flags.
    #ifdef USB_DEV_SELF_POWERED
    gDEVData.flags = USB_DEVICE_FLAGS_SELF_PWR;
    #endif
    
    // Re-initialize the HAL
    if (!USBHALReinitialize(init_flags)) {
        return FALSE;
    }
    
    // Set device address to 0
    USBHALSetBusAddress(0);

    // Configure EP0
    if (!USBHALSetEpConfiguration (0, USB_DEV_EP0_MAX_PACKET_SIZE, EP0_FLAGS )) {
        // To Do: Set an error condition (unable to config EP0).
        return FALSE;
    }
    
    // Get ready for the first setup packet.
    gDEVData.ep0_state = EP0_WAITING_SETUP;
    USBHALTransferData(XFLAGS(USB_SETUP_PKT), (BYTE *)&gDEVData.ep0_buffer, sizeof(SETUP_PKT));

    USBCompliance_SignalDeviceIsInDefaultState();   // For compliance testing only
    
    return TRUE;

} // InitializeDeviceState


/* FindEpConfig
 *************************************************************************
 * This routine scans the endpoint configuration table to find the default
 * endpoint configuration data.
 *
 * Note: interface number is ignored (since the endpoint number is known)
 *       and alternate interface is assumed to be 0 (the default).
 */
PRIVATE const EP_CONFIG *FindEpConfig ( const EP_CONFIG *ep_cfg_tbl, 
                                        int              num_entries,
                                        UINT16           dev_config,
                                        BYTE             endpoint  )
{
    int i;

    mCALL_TRACE("FindEpConfig");

    // Scan the endpoint configuration table.
    for (i=0; i < num_entries; i++)
    {
        // If we find the endpoint we're seeking...
        if (endpoint == ep_cfg_tbl[i].ep_num)
        {
            // ...Check the device config and alt-interface values.
            if ( dev_config == ep_cfg_tbl[i].config && ep_cfg_tbl[i].alt_intf == 0 )
            {
                // Found the one we wanted!
                return &ep_cfg_tbl[i];
            }
        }
    }
    // Note: Interface number doesn't matter when scanning because
    // because the endpoint number will be unique throughout a 
    // device configuration (with the exception of alternate settings).

    return NULL;

} // FindEpConfig


#ifdef USB_DEV_SUPPORTS_ALT_INTERFACES


/* FindEpConfigByInterface
 *************************************************************************
 * This routine scans the endpoint configuration table to find the 
 * endpoint configuration data for the given alternate interface.
 *
 */
PRIVATE const EP_CONFIG *FindEpConfigByInterface ( const EP_CONFIG *ep_cfg_tbl,
                                                   int              num_entries,
                                                   UINT16           dev_config,
                                                   BYTE             interface,
                                                   BYTE             alt_intf,
                                                   BYTE             endpoint  )
{
    int i;

    mCALL_TRACE("FindEpConfigByInterface");

    // Scan the endpoint configuration table.
    for (i=0; i < num_entries; i++)
    {
        // If we find the endpoint we're seeking...
        if (endpoint == ep_cfg_tbl[i].ep_num)
        {
            // ...Check the device config and alt-interface values.
            if ( dev_config == ep_cfg_tbl[i].config   &&
                 interface  == ep_cfg_tbl[i].intf     &&
                 alt_intf   == ep_cfg_tbl[i].alt_intf )
            {
                // Found the one we wanted!
                return &ep_cfg_tbl[i];
            }
        }
    }
    return NULL;

} // FindEpConfigByInterface


#endif // USB_DEV_SUPPORTS_ALT_INTERFACES


/* PassEventToAllFunctions
 *************************************************************************
 * This routine will loop through the function driver(s) event handling
 * routines, passing the event to each one of them until until it is 
 * either handled successfully.  If it is not handled, this routine
 * returns a failure.
 */

PRIVATE BOOL PassEventToAllFunctions( USB_EVENT event, void *data, int size )
{
    int             i;
    const FUNC_DRV *func_tbl;
    BOOL            handled = FALSE;

    mCALL_TRACE("PassEventToAllFunctions");

    // Access the function driver table
    func_tbl = USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC();

    // Scan the function map.
    for (i=0; i < (sizeof(UINT32)*8); i++)
    {
        // If we're using this function... 
        if ( gDEVData.function_map & (1 << i) )
        {
            // ...call its event handling routine.
            if (func_tbl[i].EventHandler(event, data, size)) {
                handled = TRUE;
            }
        }
    }

    return handled;

} // PassEventToAllFunctions


/***********************************
 * Standard Device Request Support *
 ***********************************/

/* HandleGetDescriptorRequest
 *************************************************************************
 * This routine finds the requested descriptor and sends it to the host.
 */

PRIVATE BOOL HandleGetDescriptorRequest ( PSETUP_PKT pkt )
{
    void         *desc;
    unsigned int  size;
    BOOL          success;
    
    mCALL_TRACE("HandleGetDescriptorRequest");

    // Handle GET_DESCRIPTOR request:

    #ifndef USB_DEV_GET_DESCRIPTOR_FUNC
        #error "Application layer must define USB_DEV_GET_DESCRIPTOR_FUNC"
    #endif
    
    // Get the descriptor from the application.
    desc = (void *) USB_DEV_GET_DESCRIPTOR_FUNC((PDESC_ID)&pkt->wValue, &size);

    // Stall the pipe if an unsupported desriptor is requested.
    if (desc == NULL || size == 0)
    {
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
        return FALSE;
    }

    // Send the smaller of the requested or actual length of the descriptor.
    size = min(size, pkt->wLength);

    // Send it to the host.
    gDEVData.ep0_state = EP0_SENDING_DESC;
    success = USBHALTransferData(XFLAGS(USB_SETUP_DATA|USB_TRANSMIT), desc, size);

    // Prepare now to receive the status packet in case host cuts us short.
    gDEVData.ep0_state     = EP0_WAITING_RX_STATUS;
    return success && USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_RECEIVE), NULL, 0);    

} //HandleGetDescriptorRequest


/* HandleGetConfigurationRequest
 *************************************************************************
 * This routine sends the current configuration value to the host.
 */

PRIVATE BOOL HandleGetConfigurationRequest ( void )
{
    BOOL          success;
    
    mCALL_TRACE("HandleGetConfigurationRequest");

    // Transmit the current configuration number.
    gDEVData.ep0_state     = EP0_WAITING_RX_STATUS;
    gDEVData.ep0_buffer[0] = gDEVData.dev_config;
    success= USBHALTransferData(XFLAGS(USB_SETUP_DATA|USB_TRANSMIT), &gDEVData.ep0_buffer, sizeof(BYTE));

    if (success)
    {
        // Prepare now to receive the status packet in case host cuts us short.
        return USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_RECEIVE), NULL, 0);
    }
    
    return FALSE;

} //HandleGetConfigurationRequest


/* HandleDeviceConfigRequest
 *************************************************************************
 * This routine performs the actions necessary to select the given device
 * configuration.  It gets the endpoint configuration and function driver
 * information from the application, configures the endpoints for the 
 * given device configuration, links the endpoints to the function 
 * driver(s), and initializes the functions.
 */

PRIVATE BOOL HandleDeviceConfigRequest ( BYTE config )
{
    int               i;
    int               num_entries;
    const EP_CONFIG  *ep_cfg_tbl;
    const EP_CONFIG  *ep_cfg;
    const FUNC_DRV   *func_tbl;
    UINT32            func_map;
    BOOL              success = FALSE;


    mCALL_TRACE("HandleDeviceConfigRequest");

    // Preserve the configuration number.
    gDEVData.dev_config = config;

    // Watch for the unconfigured, confguration. :)
    if (config == 0)
    {
        // Stay in the "address"
        success = TRUE;
    }
    else    // Try to configure the device.
    {
        // Get the endpoint config and function driver tables from the app.
        ep_cfg_tbl = USB_DEV_GET_EP_CONFIG_TABLE_FUNC(&num_entries);

        // Zero out the function driver map.
        func_map = 0;


        // Scan config data for all endpoints (except 0)
        for (i=0; i < USB_DEV_HIGHEST_EP_NUMBER; i++) 
        {
            // Get the endpoint configuration data.
            //
            // Note: When the device is configured, the default
            // interface (alternate interface 0) is chosen.
            ep_cfg = FindEpConfig(ep_cfg_tbl, num_entries, config, i+1);

            // If this endpoint is used...
            if (ep_cfg) 
            {
                // ...track the function driver for it,
                func_map |= 1 << ep_cfg->function;
                gDEVData.func_drv[i] = ep_cfg->function;

                #ifdef USB_DEV_SUPPORTS_ALT_INTERFACES

                // Record the interface data
                gDEVData.interface[i] = ep_cfg->intf;
                gDEVData.alt_intf[i]  = ep_cfg->alt_intf;

                #endif

                // Configure the hardware.
                if (!USBHALSetEpConfiguration(i+1, ep_cfg->max_pkt_size, 
                                              ep_cfg->flags )) {
                    return FALSE;
                }
            }
         }

        // Save the function map.
        gDEVData.function_map = func_map;


        // After all the endpoints have been configured properly,
        // we can initialize the function driver(s).
        func_tbl = USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC();
        for (i=0; i < sizeof(UINT32); i++)
        {
            // If we're using this function 
            if ( func_map & (1 << i) )
            {
                // Initialize the driver
                if (func_tbl[i].Initialize(func_tbl[i].flags)) {
                    success = TRUE;
                } else {
                    return FALSE;
                }
            }
        }
    }


    // If able to configure...
    if (success)
    {
        // Send the status packet.
        gDEVData.ep0_state = EP0_WAITING_TX_STATUS;
        if (!USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_TRANSMIT), NULL, 0)){
            return FALSE;
        }
    }
    else 
    {
        // Stall the status stage.
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
        return FALSE;
    }

    // Success
    return success;

} // HandleDeviceConfigRequest



 
/* HandleSetInterfaceRequest
 *************************************************************************
 * This routine performs the actions necessary to select an alternate 
 * interface.  It gets the endpoint configuration and function driver
 * information from the application, configures the endpoints for the 
 * given interface configuration, links the endpoints to the function 
 * driver(s), and initializes the functions.
 */

PRIVATE BOOL HandleSetInterfaceRequest ( BYTE interface, BYTE alt_interface )
{

#ifdef USB_DEV_SUPPORTS_ALT_INTERFACES

    int                 i;
    int                 num_entries;
    const EP_CONFIG    *ep_cfg_tbl;
    const EP_CONFIG    *ep_cfg;
    const FUNC_DRV     *func_tbl;
    UINT32              func_map;
    BOOL                success = TRUE;


    mCALL_TRACE("HandleSetInterfaceRequest");

    // Get the endpoint config and function driver tables from the app.
    ep_cfg_tbl = USB_DEV_GET_EP_CONFIG_TABLE_FUNC(&num_entries);

    // Zero out the function driver init map.
    func_map = 0;


    // Scan for config data for all endpoints in this interface
    for (i=0; i < USB_DEV_HIGHEST_EP_NUMBER; i++) 
    {
        // Get the endpoint configuration data.
        //
        // Note: When the device is configured, the default
        // interface (alternate interface 0) is chosen.
        ep_cfg = FindEpConfigByInterface(ep_cfg_tbl, num_entries, 
                                         gDEVData.dev_config, 
                                         interface, alt_interface, i+1);

        // If this endpoint is in the interface...
        if (ep_cfg) 
        {
            //...track the function driver for it.
            func_map |= 1 << ep_cfg->function;
            gDEVData.func_drv[i] = ep_cfg->function;

            // Record the interface data
            gDEVData.interface[i] = interface;
            gDEVData.alt_intf[i]  = alt_interface;

            // Configure the hardware.
            if (!USBHALSetEpConfiguration(i+1, ep_cfg->max_pkt_size, 
                                          ep_cfg->flags )) {
                success = FALSE;
            }
        }
     }

    if (success)
    {
        // Save the function map.
        gDEVData.function_map = func_map;


        // After the interface's endpoints have been re-configured 
        // properly, we can initialize the function driver(s).
        func_tbl   = USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC();
        for (i=0; i < (sizeof(UINT32)*8); i++)
        {
            // If we're using this function 
            if ( func_map & (1 << i) )
            {
                // Initialize the driver
                if (!func_tbl[i].Initialize(func_tbl[i].flags)) {
                    return FALSE;
                }
            }
        }
    }

    // If able to set interface...
    if (success)
    {
        // Send the status packet.
        gDEVData.ep0_state = EP0_WAITING_TX_STATUS;
        if (!USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_TRANSMIT), NULL, 0)){
            return FALSE;
        }
    }
    else 
    {
        // Stall the status stage.
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
        return FALSE;
    }

    return success;

#else   // If USB_DEV_SUPPORTS_ALT_INTERFACES not defined, stall the request.

    // Stall the status stage.
    gDEVData.ep0_state = EP0_STALLED;
    USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
    USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    return FALSE;

#endif // USB_DEV_SUPPORTS_ALT_INTERFACES

} // HandleSetInterfaceRequest


/* HandleGetInterfaceRequest
 *************************************************************************
 * This routine finds the alternate setting for the given interface and
 * send the setting to the host.
 */

PRIVATE BOOL HandleGetInterfaceRequest ( BYTE interface )
{

#ifdef USB_DEV_SUPPORTS_ALT_INTERFACES

    int     i;
    BOOL    success = FALSE;


    mCALL_TRACE("HandleGetInterfaceRequest");

    // Find the first endpoint using that interface.
    for (i=0; i < USB_DEV_HIGHEST_EP_NUMBER; i++) 
    {
        if (gDEVData.interface[i] == interface)
        {
            // Send the alternate interface number currently in use.
            gDEVData.ep0_state     = EP0_WAITING_TX_STATUS;
            gDEVData.ep0_buffer[0] = gDEVData.alt_intf[i];
            return USBHALTransferData(XFLAGS(USB_SETUP_DATA|USB_TRANSMIT),
                                      &gDEVData.ep0_buffer, sizeof(BYTE));
        }
    }

    // If able to set interface...
    if (success)
    {
        // Send the status packet.
        gDEVData.ep0_state = EP0_WAITING_TX_STATUS;
        if (!USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_TRANSMIT), NULL, 0)){
            success = FALSE;
        }
    }
    else 
    {
        // Stall the status stage.
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
        success = FALSE;
    }

    return success;

#else   // If USB_DEV_SUPPORTS_ALT_INTERFACES not defined, stall the request.

    // Stall the status stage.
    gDEVData.ep0_state = EP0_STALLED;
    USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
    USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    return FALSE;

#endif // USB_DEV_SUPPORTS_ALT_INTERFACES

} // HandleGetInterfaceRequest




/* HandleNonstandardRequests
 *************************************************************************
 * Handles requests that are not for the device layer by passing them
 * along to the higher layers.
 */

PRIVATE BOOL HandleNonstandardRequests ( PSETUP_PKT pkt )
{
    mCALL_TRACE("HandleNonstandardRequests");

    // Pass it to the higher layers.
    if (PassEventToAllFunctions(EVENT_SETUP, pkt, sizeof(SETUP_PKT)))
    {
        // Get ready for the next setup packet.
        gDEVData.ep0_state = EP0_WAITING_SETUP;
        return USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    }

    // Waiting for function layer to finish handling request..
    gDEVData.ep0_state = EP0_WAITING_FUNC;
    return FALSE;

} // HandleNonstandardRequests


/* HandleGetStatusRequest
 *************************************************************************
 * This routine provides status information about the device when 
 * requested by the host.
 */

PRIVATE BOOL HandleGetStatusRequest ( PSETUP_PKT pkt )
{
    UINT16  status;             // Status to return to host
    BOOL    success = FALSE;    // Assume failure

    mCALL_TRACE("HandleGetStatusRequest");

    // Status depends on intended recipient
    switch(pkt->requestInfo.recipient)
    {
    case USB_SETUP_RECIPIENT_DEVICE:
        // Grab device status out of flags bits.
        status = 0;
        if (gDEVData.flags & USB_DEVICE_FLAGS_SELF_PWR)
            status |= 1;
        if (gDEVData.flags & USB_DEVICE_FLAGS_REMOTE_WAKE)
            status |= 2;
        success = TRUE;
        break;

    case USB_SETUP_RECIPIENT_INTERFACE:
        // Always zero status from interface.
        status = 0;
        success = TRUE;
        break;

    case USB_SETUP_RECIPIENT_ENDPOINT:
        // Check to see if the endpoint has been stalled.
        status  = (UINT16) USBHALEndpointHasBeenStalled((TRANSFER_FLAGS)((BYTE)pkt->wIndex) );
        success = TRUE;
        break;

    case USB_SETUP_RECIPIENT_OTHER:
        // Pass it to the higher layers.
        success = HandleNonstandardRequests(pkt);
        break;

    default:
        success = FALSE;
        break;
    }

    if (success)
    {
        // Send the requested status.
        *((UINT16 *)&gDEVData.ep0_buffer) = status;
        success = USBHALTransferData(XFLAGS(USB_SETUP_DATA|USB_TRANSMIT), &gDEVData.ep0_buffer, sizeof(UINT16));

        // Receive the status stage
        if (success)
        {
            gDEVData.ep0_state = EP0_WAITING_RX_STATUS;
            if (!USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_RECEIVE), NULL, 0)){
                success = FALSE;
            }
        }
    }

    if (!success)
    {
        // Stall the status stage.
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    }

    return success;

} // HandleGetStatusRequest


/* SelectFeature
 *************************************************************************
 * This handles the SET_FEATURE and CLEAR_FEATURE standard USB device 
 * requests.
 */

PRIVATE BOOL SelectFeature ( BOOL set_feature, UINT16 feature, UINT16 target  )
{
    BOOL    success = FALSE;

    mCALL_TRACE("SelectFeature");

    switch (feature)
    {
    case USB_FEATURE_ENDPOINT_HALT:
        if (set_feature) {
            if ( 0 == (target & USB_EP_NUM_MASK) ) {
                gDEVData.ep0_state = EP0_STALLED;
            }
            // Stall the endpoint
            USBHALStallPipe  ( (TRANSFER_FLAGS)((BYTE)(target & (USB_TRANSMIT|USB_EP_NUM_MASK))) );
        } else {
            // Unstall the endpoint
            USBHALUnstallPipe( (TRANSFER_FLAGS)((BYTE)(target & (USB_TRANSMIT|USB_EP_NUM_MASK))) );
        }
        success = TRUE;

    #ifdef USB_DEV_SUPPORT_REMOTE_WAKEUP
    
    case USB_FEATURE_DEVICE_REMOTE_WAKEUP:
        if (set_feature) {
            // Set the remote-wakeup enable flag
            gDEVData.flags |= USB_DEVICE_FLAGS_REMOTE_WAKE;
            success = PassEventToAllFunctions(EVENT_SET_WAKE_UP, NULL, 0);
        } else {
            // Clear the remote-wakeup enable flag
            gDEVData.flags &= ~USB_DEVICE_FLAGS_REMOTE_WAKE;
            success = PassEventToAllFunctions(EVENT_CLEAR_WAKE_UP, NULL, 0);
        }

    #endif

    case USB_FEATURE_TEST_MODE: // Not supported (high speed only)
    default:
        break;;
    }

    if (success)
    {
        // Send the status packet.
        gDEVData.ep0_state = EP0_WAITING_TX_STATUS;
        if (!USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_TRANSMIT), NULL, 0)){
            success = FALSE;
        }
    }
    else
    {
        // Stall the status stage.
        gDEVData.ep0_state = EP0_STALLED;
        USBHALStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    }

    return success;

} // SelectFeature


/*************************************************************************
 * Function:        HandleDeviceRequest
 *
 * Preconditions:   1. USBHALInitialize must have been called to 
 *                     initialize the USB HAL.  
 *                  2. The device must have attached to the USB.
 *                  3. Endpoint 0 must have been appropriately configured.
 *                  4. A setup request must have been received.
 *                  
 * Input:           pkt     A pointer to the setup packet received.
 *
 * Output:          none
 *
 * Returns:         TRUE if the request was handled, FALSE if not
 *
 * Side Effects:    request-specific actions have been taken.
 *
 * Overview:        This routine is a called by USBDEVHandleBusEvent to 
 *                  handle setup requests when they are received.  It 
 *                  handles standard device requests itself and calls 
 *                  device-function driver(s) to handle all others.
 *************************************************************************/

BOOL HandleDeviceRequest ( void )
{
    PSETUP_PKT pkt


    mCALL_TRACE("HandleDeviceRequest");

    // Get EP0 setup packet buffer.
    pkt = (PSETUP_PKT)&gDEVData.ep0_buffer;

    // If it's not a standard request, pass it along.
    if (pkt->requestInfo.type != USB_SETUP_TYPE_STANDARD) {
        return HandleNonstandardRequests(pkt);
    }

    // Handle all standard requests
    switch (pkt->bRequest)
    {
    case USB_REQUEST_GET_DESCRIPTOR:

        // Send the requested descriptor to the host.
       return HandleGetDescriptorRequest(pkt);

    case USB_REQUEST_SET_ADDRESS:

        // Save the address
        gDEVData.flags &= USB_DEVICE_FLAGS_ADDR_MASK;
        gDEVData.flags |=(BYTE)(pkt->wValue & USB_DEVICE_FLAGS_ADDR_MASK);

        // Send the status packet
        gDEVData.ep0_state = EP0_WAITING_SET_ADDR;
        return USBHALTransferData(XFLAGS(USB_SETUP_STATUS|USB_TRANSMIT), NULL, 0);

    case USB_REQUEST_GET_CONFIGURATION:

        return HandleGetConfigurationRequest();

    case USB_REQUEST_SET_CONFIGURATION:

        // Configure the endpoints & function drivers.
        return HandleDeviceConfigRequest ((BYTE)pkt->wValue);
    
    case USB_REQUEST_SET_INTERFACE:
    
        // Reconfigure the endpoints for the given interface.
        return HandleSetInterfaceRequest((BYTE)pkt->wIndex, (BYTE)pkt->wValue);

    case USB_REQUEST_GET_INTERFACE:

        // Handle GET_INTERFACE request
        return HandleGetInterfaceRequest((BYTE)pkt->wIndex);    

    case USB_REQUEST_GET_STATUS:

        // Supply the requested status or pass it along.
        return HandleGetStatusRequest(pkt);

    case USB_REQUEST_SET_FEATURE:

        // Handle SET_FEATURE request
        return SelectFeature(TRUE, pkt->wValue, pkt->wIndex);

    case USB_REQUEST_CLEAR_FEATURE:

        // Handle CLEAR_FEATURE request
        return SelectFeature(FALSE, pkt->wValue, pkt->wIndex);

    case USB_REQUEST_SYNCH_FRAME:
        
        // Pass it to the higher layers.
        return HandleNonstandardRequests(pkt);

    case USB_REQUEST_SET_DESCRIPTOR:

        // Not Supported
        gDEVData.ep0_state = EP0_STALLED;
        return USBDEVStallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
    
    default:

        // Not supported by device layer.
        // Stall EP0 (will cause host to reset us).
        gDEVData.ep0_state = EP0_STALLED;
        return USBDEVStallPipe(XFLAGS(USB_EP0|USB_RECEIVE));
    }

    return FALSE;

} // HandleDeviceRequest


/*********************
 * Bus Event Support *
 *********************/

/* HandleDeviceAttach
 *************************************************************************
 * This routine performs the steps necessary to prepare for USB periheral
 * device operation when a cable has been attached.  It configures 
 * endpoint 0 so that it is ready to use for control transfers, and 
 * starts a receive transfer to be ready to accept the first setup packet.
 */

PRIVATE BOOL HandleDeviceAttach ( void )

{
    mCALL_TRACE("HandleDeviceAttach");

    gDEVData.flags |= USB_DEVICE_FLAGS_ATTACHED;

    // Configure EP0
    if (!USBHALSetEpConfiguration (0, USB_DEV_EP0_MAX_PACKET_SIZE, EP0_FLAGS )) {
        // To Do: Set an error condition (unable to config EP0).
        return FALSE;
    }

    // Connect to the bus
    USBHALControlUsbResistors(USB_HAL_DEV_CONN_FULL_SPD);


    return TRUE;

} // HandleDeviceAttach

/*
 * Note: HandleDeviceAttach assumes that the EP0 buffer is big enough
 * to hold a complete setup packet.
 */
#if USB_DEV_EP0_MAX_PACKET_SIZE < 8
#error "Error!  EP0 max packet size must be big enough to hold a setup packet"
#endif


/* HandleDataTransferEvent
 *************************************************************************
 * This routine handles a data transfer events.  It identifies EP0/setup 
 * transfers and directs them to the device request handler.  All other
 * data transfers are sent to the function-driver(s).
 */
 
PRIVATE BOOL HandleDataTransferEvent ( USB_TRANSFER_EVENT_DATA *xfer )
{
    const FUNC_DRV *func_tbl;

    mCALL_TRACE("HandleDataTransferEvent");

    // Debug Only
    #ifdef ENABLE_STATE_TRACE
    gStateTrace[gStateIndex].state = gDEVData.ep0_state;
    gStateTrace[gStateIndex].flags = xfer->flags;
    gStateIndex = (gStateIndex + 1) % NUM_STATES;
    #endif

    // Is it an endpoint 0 transfer that just completed?
    if (xfer->flags.field.ep_num == 0)
    {
        // Rx Done
        if ( xfer->flags.field.direction == 0)  // Receive
        {
            // Handle state changes
            switch (gDEVData.ep0_state)
            {
            case EP0_WAITING_SETUP:

                // If we were waiting for a setup packet and something else 
                // happened it's not ours, pass it on.
                if (PassEventToAllFunctions(EVENT_TRANSFER, xfer, sizeof(USB_TRANSFER_EVENT_DATA))) {
                    break;
                }
                return TRUE;

            case EP0_STALLED:
                USBHALUnstallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
                if (xfer->pid != 0x0D) while (1); // To Do: Remove
                break;

            case EP0_WAITING_RX_STATUS:         // Status stage complete
                USBHALFlushPipe(XFLAGS(USB_EP0|USB_TRANSMIT));   // Flush the TX pipe 
                break;                          // Prepare for next setup packet

            case EP0_SENDING_DESC:              // Host stopped transfer early
                USBHALFlushPipe(XFLAGS(USB_EP0|USB_TRANSMIT));   // Flush the TX pipe
                break;                          // prepare for the next setup packet

            case EP0_WAITING_TX_STATUS:
                while(1);

            case EP0_WAITING_FUNC:

                // Pass the event to the function layer.
                if (PassEventToAllFunctions(EVENT_TRANSFER, xfer, sizeof(USB_TRANSFER_EVENT_DATA))) {
                    break;      // If handled, we're ready for next setup packet.
                }
                return TRUE;    // Otherwise, keep waiting.

            default:
                while(1);
            }
        }
        else // Tx done
        {
            // Handle state changes
            switch (gDEVData.ep0_state)
            {
            case EP0_WAITING_SETUP:
                return TRUE;

            case EP0_WAITING_SET_ADDR:          // Handle SET_ADDRESS request
                USBHALSetBusAddress((BYTE)(gDEVData.flags & USB_DEVICE_FLAGS_ADDR_MASK));
                break;
                
            case EP0_SENDING_DESC:              // Done sending data, wait for status stage
                gDEVData.ep0_state = EP0_WAITING_RX_STATUS;
                return TRUE;                    // Rx transfer already started

            case EP0_WAITING_RX_STATUS:
                return TRUE;

            case EP0_WAITING_TX_STATUS:         // Schedule the next setup packet
                break;

            case EP0_WAITING_FUNC:

                // Pass the event to the function layer.
                if (PassEventToAllFunctions(EVENT_TRANSFER, xfer, sizeof(USB_TRANSFER_EVENT_DATA))) {
                    break;      // If handled, we're ready for next setup packet.
                }
                return TRUE;    // Otherwise, keep waiting.

            default:
                while(1);
            }
        }

        
        // Start a receive transfer to be ready for the next setup packet. 
        gDEVData.ep0_state = EP0_WAITING_SETUP;
        return USBHALTransferData(XFLAGS(USB_SETUP_PKT), &gDEVData.ep0_buffer, sizeof(SETUP_PKT));
    }
    else
    {
        // Access the function driver table
        func_tbl = USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC();
        
        // Pass the event to the function driver to which the endpoint belongs.
        #ifdef USB_SAFE_MODE
        if (func_tbl != NULL              && 
            xfer->flags.field.ep_num > 0  && 
            xfer->flags.field.ep_num <= USB_DEV_HIGHEST_EP_NUMBER)
        {
            // Index into the function driver table & call the function's event handler.
            func_tbl = &func_tbl[gDEVData.func_drv[xfer->flags.field.ep_num-1]];
            if (func_tbl != NULL) {
                return func_tbl->EventHandler(EVENT_TRANSFER, xfer, sizeof(USB_TRANSFER_EVENT_DATA));
            } else {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
        #else
        // Index into the function driver table & call the function's event handler.
        func_tbl = &func_tbl[gDEVData.func_drv[xfer->flags.field.ep_num-1]];
        return func_tbl->EventHandler(EVT_XFER, xfer, sizeof(USB_TRANSFER_EVENT_DATA));
        #endif

    }

} // HandleDataTransferEvent


/* HandleReset
 *************************************************************************
 * This routine handles a USB device reset.  It clears the context data
 * structure, sets the device address to 0, and prepares for the first 
 * control transfer.
 */

PRIVATE BOOL HandleReset ( void )
{
    mCALL_TRACE("HandleReset");

    // Set device to default state
    return InitializeDeviceState(gDEVData.init_flags);

} // HandleReset
 

/*************************************************************************
 * Function:        USBDEVHandleBusEvent
 *
 * Preconditions:   1. USBHALInitialize must have been called to 
 *                     initialize the USB HAL.  
 *                  
 * Input:           event       Identifies the bus event that occured
 *
 *                  data        Pointer to event-specific data
 *
 *                  size        Size of the event-specific data
 *
 * Output:          none
 *
 * Returns:         TRUE if the event was handled, FALSE if not
 *
 * Side Effects:    Event-specific actions have been taken.
 *
 * Overview:        This routine is a called by the HAL to handle bus 
 *                  events as they occur.  Events are identified by the 
 *                  "event" parameter and may have associated data (such 
 *                  as a setup request packet).
 *************************************************************************/

PUBLIC BOOL USBDEVHandleBusEvent ( USB_EVENT event, void *data, int size )
{
    mCALL_TRACE("USBDEVHandleBusEvent");

    #ifdef ENABLE_EVENT_TRACE
    gEventTrace[gEventIndex].event = event;
    gEventTrace[gEventIndex].data  = data;
    gEventTrace[gEventIndex].size  = size;
    gEventIndex = (gEventIndex + 1) % (sizeof(gEventTrace)/sizeof(EVENT_TRACE_DATA));
    #endif
    

    switch (event)
    {   
    case EVENT_TRANSFER:    // A USB transfer has completed (has data)
        return HandleDataTransferEvent((USB_TRANSFER_EVENT_DATA *)data);

    case EVENT_SOF:         // Start of frame (device doesn't care)
        return PassEventToAllFunctions(EVENT_SOF, NULL, 0);

    case EVENT_RESUME:      // Device-mode resume received
        gDEVData.flags &= ~USB_DEVICE_FLAGS_SUSPENDED;
        return PassEventToAllFunctions(EVENT_RESUME, NULL, 0);

    case EVENT_SUSPEND:     // Device-mode suspend/idle event received
        gDEVData.flags |= USB_DEVICE_FLAGS_SUSPENDED;
        return PassEventToAllFunctions(EVENT_SUSPEND, NULL, 0);

    case EVENT_RESET:       // Device-mode bus reset received
        return HandleReset();

    case EVENT_DETACH:      // USB cable has been detached
        gDEVData.flags &= ~USB_DEVICE_FLAGS_ATTACHED;
        return PassEventToAllFunctions(EVENT_DETACH, NULL, 0);

    case EVENT_ATTACH:      // USB cable has been attached
        return HandleDeviceAttach();

    case EVENT_STALL:       // A stall has occured
        if (gDEVData.ep0_state == EP0_STALLED) {
            return USBHALUnstallPipe(XFLAGS(USB_EP0|USB_TRANSMIT));
        } else {
            return PassEventToAllFunctions(EVENT_STALL, NULL, 0);
        }

    case EVENT_BUS_ERROR:   // Error on the bus, call USBHALGetLastError()
        return PassEventToAllFunctions(EVENT_BUS_ERROR, NULL, 0);

    case EVENT_NONE:
    default:
        return FALSE;

    }

} // USBDEVHandleBusEvent


/******************************
 * Device Interface Functions *
 ******************************/

/*************************************************************************
 * Function:        USBDEVInitialize
 *
 * Precondition:    none
 *
 * Input:           flags       USB Initialization flags, passed to HAL.
 *
 * Output:          none
 *
 * Returns:         TRUE if successful, FALSE if not.
 *
 * Side Effects:    The USB Device-side SW stack was initialized for the 
 *                  requested bus.
 *
 * Overview:        This call performs the basic initialization of the USB 
 *                  device-side SW stack, including initialization of the 
 *                  HAL.
 *************************************************************************/

PUBLIC BOOL USBDEVInitialize ( unsigned long flags )
{
    mCALL_TRACE("USBDEVInitialize");

    // Initialize the device
    if (InitializeDeviceState(flags))
    {
        // Initialize the HAL
        if (USBHALInitialize(flags))
        {
            return TRUE;
        }
    }

    return FALSE;

} // USBDEVInitialize


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

BOOL USBDEVSignalResume ( void )
{
    if (gDEVData.flags & USB_DEVICE_FLAGS_REMOTE_WAKE)
    {
        USBHALSignalResume();
    }

    return FALSE;

} // USBDEVSignalResume


/*************************************************************************
 * EOF
 */

