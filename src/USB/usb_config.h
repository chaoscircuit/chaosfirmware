/**
 * \file usb_config.h
 * \brief Microchip USB configuration header file
 */

#ifndef _USB_CONFIG_H_
#define _USB_CONFIG_H_

/**************************
 * USB Role Configuration *
 **************************/

/* USB Role
 *
 * The following macros determine what role this application will play 
 * on the USB - whether it is a USB peripheral device (USB_SUPPORT_DEVICE),
 * a USB host (USB_SUPPORT_HOST) or a dual-role device (USB_SUPPORT_OTG).
 */
#define USB_SUPPORT_DEVICE


/* USB_DEV_INTERRUPT_DRIVEN
 *
 * This macro enables interrupt-driven mode of the USB Device stack.  
 * When it is defined, the "USBTasks()" routine (and any event-handling
 * routines that it calls) will be called from within an ISR context.
 * This eliminates the need for the application to call "USBTasks()" 
 * from its main loop.
 */
//#define USB_DEV_INTERRUPT_DRIVEN


/* USB_DEV_EVENT_HANDLER
 *
 * This macro defines the name of the bus-event-handling function for the
 * device-side support layer.  If devicd functionality is not supported
 * it may be NULL.
 */
 
#define USB_DEV_EVENT_HANDLER   USBDEVHandleBusEvent


/* USB_HOST_EVENT_HANDLER
 *
 * This macro defines the name of the bus-event-handling function for the
 * host-side support layer.  If host functionality is not supported it 
 * may be NULL.
 */

#define USB_HOST_EVENT_HANDLER  NULL


/******************************************
 * USB Endpoint & Interface Configuration *
 ******************************************/

/* USB_DEV_HIGHEST_EP_NUMBER
 *
 * This macro defines the highest endpoint number used.
 *
 * Note: The device layer will allocate RAM for an array of pointers 
 * for every endpoint (except endpoint 0).
 */
 
#define USB_DEV_HIGHEST_EP_NUMBER   1


/* USB_DEV_SUPPORTS_ALT_INTERFACES
 *
 * Allows the device layer to support peripheral devices that have
 * alternate interfaces.  This support requires an additional 60 bytes
 * of RAM space.
 */

//#define USB_DEV_SUPPORTS_ALT_INTERFACES


/* USB_DEV_EP0_MAX_PACKET_SIZE
 *
 * This macro defines the maximum packet size allowed for endpoint 0.
 * It must be defined as either 8, 16, 32, or 64 bytes.
 *
 * Note: The device layer (if supported) will statically allocate an 
 * endpoint buffer of this size.
 */
 
#define USB_DEV_EP0_MAX_PACKET_SIZE 8


/******************
 * User Functions *
 ******************/

/* USB_DEV_GET_DESCRIPTOR_FUNC
 *
 * This macro defines the name of the routine that provides the 
 * descriptors to the device layer.  This routine must be implemented by
 * the application.  The signature of the function must match that 
 * defined by the USBDEVGetDescriptorCallout type in device.h
 */
 
#define USB_DEV_GET_DESCRIPTOR_FUNC USBDEVGetDescriptor


/* USB_DEV_GET_EP_CONFIG_TABLE_FUNC
 * 
 * This macro defines the name of the routine that provides a pointer
 * to the endpoint configuration table used to configure endpoints 
 * as desired when operating in device mode.
 *
 * Note: It may be NULL if the application does not support device 
 * functionality (if host only).
 */
 
#define USB_DEV_GET_EP_CONFIG_TABLE_FUNC USBDEVGetEpConfigurationTable
 

/* USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC
 *
 * This macro defines the name of the routine that provides the pointer
 * to the function-driver table.
 *
 * Note: It may be NULL if the application does not support device 
 * functionality (if host only).
 */
 
#define USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC USBDEVGetFunctionDriverTable


/**************
 * Miscellany *
 **************/
 
/* USB_DEV_SELF_POWERED
 *
 * This should be defined if the system is self powered when acting as a
 * USB peripheral device.
 *
 * Note: Must match the information provided in the descriptors.
 */
 
#define USB_DEV_SELF_POWERED


/* USB_DEV_SUPPORT_REMOTE_WAKEUP
 *
 * This should be defined if the system is to support remotely waking
 * up a host when acting as a device.
 */
 
//#define USB_DEV_SUPPORT_REMOTE_WAKEUP


/* USB_SAFE_MODE
 *
 * Define this macro to enable parameter and bounds checking in various
 * places throughout the USB SW stack.  This feature can be removed for 
 * efficiency by not defining this label once careful testing and debugging
 * have been done.
 */
 
#define USB_SAFE_MODE

#if defined(__18CXX)
    #define LANGID_LENGTH 1
    #define STRING_LENGH 1
    #define PHYSICAL_INFO_LENGH 1
#else
    #define LANGID_LENGTH 0
    #define STRING_LENGH 0
    #define PHYSICAL_INFO_LENGH 0
#endif

/**************************************
 * Application Specific Configuration *
 **************************************/

// Emulate PIC18 Demo Board Descriptors:
#define EMULATE_PIC18_DEMO

// Generic Driver configuration Number
#define USBGEN_CONFIG_NUM   1

// Generic Driver Interface Number
#define USBGEN_INTF_NUM     0

// Generic Driver Enpoint
#define USBGEN_EP_NUM       1

// Demo Buffer Size
#define USBGEN_EP_SIZE      1024


#endif // _USB_CONFIG_H_
/*************************************************************************
 * EOF
 */

