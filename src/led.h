/**
 * \file led.h
 * \brief Header file for led.c
 */

#ifndef LED_H
#define LED_H

#include <plib.h>
#include "globals.h"

void LED_init(void);
void LED_test(void);

/* LED Macros ************************************************************/

#ifdef STARTERKIT

    #define mLED_1              LATDbits.LATD0
    #define mLED_2              LATDbits.LATD1
    #define mLED_3              LATDbits.LATD2

    #define LED_LAT                LATD
    
    #define mLED_Init()  {TRISD = TRISD & ~0x07; LATD = LATD & ~0x07;}

#endif

#ifndef STARTERKIT

    #define mLED_1              LATFbits.LATF0
    
    #define LED_LAT                LATF
    
    #define mLED_Init()  {TRISF = TRISF & ~0x01; LATF = LATF & ~0x01;}

#endif

    #define mLED_1_On()         mLED_1  = 1;

    #define mLED_1_Off()        mLED_1  = 0;
    
    #define mLED_1_Toggle()     mLED_1  = !mLED_1;

    #define mDemonstration_LED_On()     mLED_1  = 1;
    #define mDemonstration_LED_Off()    mLED_1  = 0;
    #define mDemonstration_LED_Toggle()    mLED_1  = !mLED_1;

#endif
