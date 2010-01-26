/**
 * \file local_typedefs.h
 * \brief Microchip local type definitions
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H


#if defined(__18CXX)
    // To Do: Define rom
#else
#define rom
#endif

typedef unsigned char   byte;           // 8-bit
typedef unsigned int    word;           // 16-bit
typedef unsigned long   dword;          // 32-bit

typedef union _BYTE
{
    byte _byte;
    struct
    {
        unsigned b0:1;
        unsigned b1:1;
        unsigned b2:1;
        unsigned b3:1;
        unsigned b4:1;
        unsigned b5:1;
        unsigned b6:1;
        unsigned b7:1;
    };
} BYTE_UNION;

typedef union _WORD
{
    word _word;
    struct
    {
        byte byte0;
        byte byte1;
    };
    struct
    {
        BYTE_UNION Byte0;
        BYTE_UNION Byte1;
    };
    struct
    {
        BYTE_UNION LowB;
        BYTE_UNION HighB;
    };
    struct
    {
        byte v[2];
    };
} WORD_UNION;
#define LSB(a)      ((a).v[0])
#define MSB(a)      ((a).v[1])

typedef union _DWORD
{
    dword _dword;
    struct
    {
        byte byte0;
        byte byte1;
        byte byte2;
        byte byte3;
    };
    struct
    {
        word word0;
        word word1;
    };
    struct
    {
        BYTE_UNION Byte0;
        BYTE_UNION Byte1;
        BYTE_UNION Byte2;
        BYTE_UNION Byte3;
    };
    struct
    {
        WORD_UNION Word0;
        WORD_UNION Word1;
    };
    struct
    {
        byte v[4];
    };
} DWORD_UNION;
#define LOWER_LSB(a)    ((a).v[0])
#define LOWER_MSB(a)    ((a).v[1])
#define UPPER_LSB(a)    ((a).v[2])
#define UPPER_MSB(a)    ((a).v[3])

typedef void(*pFunc)(void);

typedef union _POINTER
{
    struct
    {
        byte bLow;
        byte bHigh;
    };
    word _word;                         // bLow & bHigh
    

    byte* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    word* wRam;                         // Ram word poitner: 2 bytes poitner pointing
                                        // to 2 bytes of data

    rom byte* bRom;                     // Size depends on compiler setting
    rom word* wRom;
} __POINTER;


#define OK      TRUE
#define FAIL    FALSE

#endif //TYPEDEFS_H
