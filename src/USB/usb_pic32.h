/**
 * \file usb_pic32.h
 * \brief Microchip header to define PIC32 specific USB items
 */

#define USTAT_EP0_PP_MASK   ~0x04
#define USTAT_EP_MASK       0xFC
#define USTAT_EP0_OUT       0x00
#define USTAT_EP0_OUT_EVEN  0x00
#define USTAT_EP0_OUT_ODD   0x04

#define USTAT_EP0_IN        0x08
#define USTAT_EP0_IN_EVEN   0x08
#define USTAT_EP0_IN_ODD    0x0C

//******************************************************************************
// USB Endpoint Control Registers
//
// In USB Host mode, only EP0 control registers are used.  The other registers
// should be disabled.
//******************************************************************************

typedef union
{
    WORD UEP[16];
} _UEP;


/********************************************************************
 * Buffer Descriptor Status Register
 *******************************************************************/

/* Buffer Descriptor Status Register Initialization Parameters */

#define _BSTALL     0x04        //Buffer Stall enable
#define _DTSEN      0x08        //Data Toggle Synch enable
#define _DAT0       0x00        //DATA0 packet expected next
#define _DAT1       0x40        //DATA1 packet expected next
#define _DTSMASK    0x40        //DTS Mask
#define _USIE       0x80        //SIE owns buffer
#define _UCPU       0x00        //CPU owns buffer

// Buffer Descriptor Status Register layout.

typedef union _BD_STAT
{
    struct{
        unsigned            :2;
        unsigned    BSTALL  :1;     //Buffer Stall Enable
        unsigned    DTSEN   :1;     //Data Toggle Synch Enable
        unsigned            :2;     //Reserved - write as 00
        unsigned    DTS     :1;     //Data Toggle Synch Value
        unsigned    UOWN    :1;     //USB Ownership
        unsigned            :8;
        unsigned    count   :10;    //Byte count
    };
    struct{
        unsigned            :2;
        unsigned    PID0    :1;
        unsigned    PID1    :1;
        unsigned    PID2    :1;
        unsigned    PID3    :1;
        unsigned            :2;
        unsigned            :8;
        unsigned    BC      :10;
    };
    struct{
        unsigned            :2;
        unsigned    PID     :4;         //Packet Identifier
    };
    DWORD           Val;
} BD_STAT;


/********************************************************************
 * Buffer Descriptor Table Mapping
 *******************************************************************/

// BDT Entry Layout
typedef union __BDT
{
    struct
    {
        BD_STAT     STAT;
        BYTE*       ADR;                      //Buffer Address
    };
    struct
    {
        DWORD       res  :16;
        DWORD       count:10;
    };
    DWORD           v[2];
} BDT_ENTRY;



