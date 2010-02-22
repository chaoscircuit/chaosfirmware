/**
 * \file usb_config.c
 * \brief Microchip USB Configuration routines
 */

#include "GenericTypeDefs.h"
#include "USB/usb.h"

#include "USB/usb_device_generic.h"


/************************
 * USB Descriptor Table *
 ************************/                

#define NUM_LANGS   1
#define LANG_1_ID   0x0409
#define STR_1_LEN   25
#define STR_2_LEN   27
#define STR_3_LEN   10


/* USB Descriptor Table Structures
 *************************************************************************
 * These structures defines the complete set of descriptors necessary for 
 * the USB device portion of this application.
 *
 * To Do: Modify this structure as necessary for the application's device.
 */

#pragma pack(push,1)  // Must not have any padding.

typedef struct _config1_descriptors
{
    USB_CONFIGURATION_DESCRIPTOR    cfg_desc;           // Configuration 1
    USB_INTERFACE_DESCRIPTOR        intf0_desc;         // Config 1, Interface 0
    USB_ENDPOINT_DESCRIPTOR         intf0_ep1_in_desc;  // Endpoint 0 in (Tx)
    USB_ENDPOINT_DESCRIPTOR         intf0_ep1_out_desc; // Endpoint 0 out (Rx)

} CONFIG1_DESC, *PCONFIG1_DESC;

typedef struct _string0_descriptor
{
    USB_STRING_DESCRIPTOR   string;                     // String0 Descriptor
    WORD                    langid[NUM_LANGS];
} STR0_DESC, *PSTR_DESC;

typedef struct _string1_descriptor
{
    USB_STRING_DESCRIPTOR   string;                     // String1 Descriptor
    WORD                    string_data[STR_1_LEN];

} STR1_DESC, *PSTR1_DESC;

typedef struct _string2_descriptor
{
    USB_STRING_DESCRIPTOR   string;                     // String2 Descriptor
    WORD                    string_data[STR_2_LEN];

} STR2_DESC, *PSTR2_DESC;

typedef struct _string3_descriptor
{
    USB_STRING_DESCRIPTOR   string;                     // String3 Descriptor
    WORD                    string_data[STR_3_LEN];

} STR3_DESC, *PSTR3_DESC;

#pragma pack(pop)


// Device Descriptor
USB_DEVICE_DESCRIPTOR dev_desc = 
{    
    sizeof(USB_DEVICE_DESCRIPTOR),  // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,          // DEVICE descriptor type
    0x0200,                         // USB Spec Release Number in BCD format
    0x00,                           // Class Code
    0x00,                           // Subclass code
    0x00,                           // Protocol code
    USB_DEV_EP0_MAX_PACKET_SIZE,    // Max packet size for EP0, see usbcfg.h
    0x0945,                         // Vendor ID
    0x7777,                         // Product ID: PICDEM FS USB (DEMO Mode)
    0x0000,                         // Device release number in BCD format
    0x01,                           // Manufacturer string index
    0x02,                           // Product string index
    0x00,                           // Device serial number string index
    0x01                            // Number of possible configurations
};


// Configuration 1 Descriptors:
CONFIG1_DESC config1 = 
{
    {   /* Configuration Descriptor */
        sizeof(USB_CONFIGURATION_DESCRIPTOR),   // Size of this descriptor in bytes
        USB_DESCRIPTOR_CONFIGURATION,           // CONFIGURATION descriptor type
        sizeof(CONFIG1_DESC),                   // Total length of data for this cfg
        1,                                      // Number of interfaces in this cfg
        USBGEN_CONFIG_NUM,                      // Index value of this configuration
        0,                                      // Configuration string index
        0x01<<7,                                // Attributes, see usbdefs_std_dsc.h
        50                                      // Max power consumption (2X mA)
    },
    {   /* Interface Descriptor */
        sizeof(USB_INTERFACE_DESCRIPTOR),       // Size of this descriptor in bytes
        USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
        USBGEN_INTF_NUM,                        // Interface Number
        0,                                      // Alternate Setting Number
        2,                                      // Number of endpoints in this intf
        0xff,                                   // Class code
        0x00,                                   // Subclass code
        0x00,                                   // Protocol code
        0                                       // Interface string index
    },
    /* Endpoint Descriptors */
    {   /* EP 1 - Out */
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_DESCRIPTOR_ENDPOINT,
        EP_DIR_OUT|USBGEN_EP_NUM,
        EP_ATTR_BULK,
        EP_MAX_PKT_BULK_FS,
        32
    },
    {   /* EP 1 - In */
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_DESCRIPTOR_ENDPOINT,
        EP_DIR_IN|USBGEN_EP_NUM,
        EP_ATTR_BULK,
        EP_MAX_PKT_BULK_FS,
        32
    }
};


// String Descriptors:

STR0_DESC string0 =
{
    {   // Language ID: English
        sizeof(STR0_DESC),
        USB_DESCRIPTOR_STRING
    },
    {LANG_1_ID}
};
    

STR1_DESC string1 =
{
    {   // Vendor Description
        sizeof(STR1_DESC),
        USB_DESCRIPTOR_STRING
    },
    {'T','a','y','l','o','r',' ','U','n','i',
    'v','e','r','s','i','t','y',' ',' ',' ',' ',
    ' ',' ',' ',' '}
};


STR2_DESC string2 =
#ifdef EMULATE_PIC18_DEMO
{
    {   // Device Description
        sizeof(STR2_DESC),
        USB_DESCRIPTOR_STRING
    },
    {'E','l','e','c','t','r','o','n','i','c',' ','C','h',
     'a','o','s',' ','S','y','s','t','e','m',' ','v','2','n'}
};
#else
{
    {   // Device Description
        sizeof(STR2_DESC),
        USB_DESCRIPTOR_STRING
    },
    {'C','h','a','o','s',' ','S','y','s','t','e','m',' ',' ',' ',' ',' '}
};
#endif


STR3_DESC string3 =
{
    {   // Serial Number
        sizeof(STR3_DESC),
        USB_DESCRIPTOR_STRING
    },
    {'0','0','0','0','0','0','0','0','0','0'}
};


/*******************************************************
 * USB_DEV_GET_DESCRIPTOR_FUNC Function Implementation *
 *******************************************************/

/* GetConfigurationDescriptor
 *************************************************************************
 * Returns a pointer to the requested configuration descriptor & provides
 * the total size of the configuration descriptor set.
 */

static inline const void *GetConfigurationDescriptor( BYTE config, unsigned int *length )
{
    switch (config)
    {
    case 0: // Configuration 1 (default)
        *length = sizeof(config1);
        return &config1;
   
    default:
        return NULL;
    }

} // GetConfigurationDescriptor


/* GetStringDescriptor
 *************************************************************************
 * Returns a pointer to the requested string descriptor and provides it's
 * size.
 */

static inline const void *GetStringDescriptor( PDESC_ID desc, unsigned int *length )
{
    // Check language ID
    if (desc->lang_id != LANG_1_ID) {
        return NULL;
    }

    // Get requested string
    switch(desc->index)
    {
    case 0: // String 0
        *length = sizeof(string0);
        return &string0;

    case 1: // String 1
        *length = sizeof(string1);
        return &string1;

    case 2: // String 2
        *length = sizeof(string2);
        return &string2;

    case 3: // String 3
        *length = sizeof(string3);
        return &string3;

    default:
        return NULL;
    }

} // GetStringDescriptor


/*************************************************************************
 * Function:        USBDEVGetDescriptor
 *
 * Precondition:    Assumes that the USB SW stack has been initialized.
 *
 * Input:           type    Type of USB Descriptor desired.
 *
 *                  index   Which descriptor of that type is desired.
 *
 * Output:          length  Length of the descriptor in bytes.
 *
 * Returns:         A pointer to the descriptor.
 *
 * Side Effects:    none
 *
 * Overview:        This function is a "call out" from the USB SW stack
 *                  that must be implemented by the application.  The 
 *                  USB device support will call it in response to 
 *                  GET_DESCRIPTOR setup requests in order to provide the
 *                  host with the desired descriptor data.
 *
 * Notes:           Must be implemented by application.  
 *
 *                  Define the USB_DEV_GET_DESCRIPTOR_FUNC macro in the
 *                  USB configuration file to the name of this routine.
 *************************************************************************/

const void *USBDEVGetDescriptor ( PDESC_ID desc, unsigned int *length )
{
    switch (desc->type)
    {
    case USB_DESCRIPTOR_DEVICE:        // Device Descriptor
        *length = sizeof(dev_desc);
        return &dev_desc;

    case USB_DESCRIPTOR_CONFIGURATION: // Configuration Descriptor
        return GetConfigurationDescriptor(desc->index, length);

    case USB_DESCRIPTOR_STRING:        // String Descriptor
        return GetStringDescriptor(desc, length);

    // Fail all un-supported descriptor requests:
    default:
        return NULL;
    }

} // USBDEVGetDescriptor


/********************************
 * Endpoint Configuration Table *
 ********************************
 *
 * This table defines all supported endpoint configurations for this 
 * application.  The configurations must match with the information 
 * provided to the host in the descriptors (defined above).
 *
 * Note: This table should not include an entry for endpoint 0.
 */

const EP_CONFIG gEpConfigTable[] =
{
    {   // EP1 - In & Out
        EP_MAX_PKT_BULK_FS,     // Maximum packet size for this endpoint
        USB_EP_TRANSMIT |       // Configuration flags for this endpoint
        USB_EP_RECEIVE  |
        USB_EP_HANDSHAKE,   
        USBGEN_CONFIG_NUM,      // Configuration number
        USBGEN_EP_NUM,          // Endpoint number.
        USBGEN_INTF_NUM,        // Interface number
        0,                      // Alternate interface setting
        0                       // Index in device function table (see below)
    }
};

/************************************************************
 * USB_DEV_GET_EP_CONFIG_TABLE_FUNC Function Implementation *
 ************************************************************/

/*************************************************************************
 * Function:        USBDEVGetEpConfigurationTable
 *
 * Precondition:    none
 *
 * Input:           none
 *
 * Output:          num_entries     Number of entries in the endpoint 
 *                                  configuration table.
 *
 * Returns:         A pointer to the endpoint configuration table.
 *
 * Side Effects:    none
 *
 * Overview:        This function is a "call out" from the USB SW stack
 *                  that must be implemented by the application.  The 
 *                  USB device support will call it when it needs the
 *                  endpoint configuration table.
 *
 * Notes:           Must be implemented by application.
 *
 *                  Define the USB_DEV_GET_EP_CONFIG_TABLE_FUNC macro in 
 *                  the USB configuration file to the name of this routine.
 *************************************************************************/

inline const EP_CONFIG *USBDEVGetEpConfigurationTable ( int *num_entries )
{
    // Provide the number of entries
    *num_entries = sizeof(gEpConfigTable)/sizeof(EP_CONFIG);

    // Provide the table pointer.
    return gEpConfigTable;

} // USBDEVGetEpConfigurationTable


/******************************
 * USB Device Functions Table *
 ******************************
 *
 * This table provides information about the USB device functions that are
 * supported by this application.
 *
 * Note: This table cannot have more then 32 entries in it.
 */

const FUNC_DRV gDevFuncTable[] = 
{
    {   // Generic Function Driver
        USBGenInitialize,           // Init routine
        USBGenEventHandler,         // Event routine
        USBGEN_EP_NUM               // Endpoint Number (bottom 4 bits)
    }
};


/*********************************************************
 * USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC Implementation *
 *********************************************************/

/*************************************************************************
 * Function:        USBDEVGetFunctionDriverTable
 *
 * Precondition:    none
 *
 * Input:           none
 *
 * Output:          none
 *
 * Returns:         A pointer to the function driver table.
 *
 * Side Effects:    none
 *
 * Overview:        This function is a "call out" from the USB SW stack
 *                  that must be implemented by the application.  The 
 *                  USB device support will call it when it needs get 
 *                  the function driver table.
 *
 * Notes:           Must be implemented by application.
 *
 *                  Define the USB_DEV_GET_FUNCTION_DRIVER_TABLE_FUNC 
 *                  macro in the USB configuration file to the name of 
 *                  this routine.
 *************************************************************************/

inline const FUNC_DRV *USBDEVGetFunctionDriverTable ( void )
{
    return gDevFuncTable;

} // USBDEVGetFunctionDriverTable


/*************************************************************************
 * EOF
 */
