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

    #define mLED_1              LATEbits.LATE0
    #define mLED_2              LATEbits.LATE1
    #define mLED_3              LATEbits.LATE2
    
    #define LED_LAT                LATE
    
    #define mLED_Init()  {TRISE = TRISE & ~0x07; LATE = LATE & ~0x07;}

#endif

    #define mLED_1_On()         mLED_1  = 1;
    #define mLED_2_On()         mLED_2  = 1;
    #define mLED_3_On()         mLED_3  = 1;
    
    #define mLED_1_Off()        mLED_1  = 0;
    #define mLED_2_Off()        mLED_2  = 0;
    #define mLED_3_Off()        mLED_3  = 0;
    
    #define mLED_1_Toggle()     mLED_1  = !mLED_1;
    #define mLED_2_Toggle()     mLED_2  = !mLED_2;
    #define mLED_3_Toggle()     mLED_3  = !mLED_3;

    #define mChaos_LED_On()     mLED_3  = 1;
    #define mChaos_LED_Off()    mLED_3  = 0;
    #define mChaosLED_Toggle()    mLED_3 = !mLED_3;
    #define mDemonstration_LED_On()     mLED_1  = 1;
    #define mDemonstration_LED_Off()    mLED_1  = 0;
    #define mDemonstration_LED_Toggle()    mLED_1  = !mLED_1;
    #define mHeartbeat_LED_On()     mLED_2  = 1;
    #define mHeartbeat_LED_Off()    mLED_2  = 0;
    #define mHeartbeat_LED_Toggle()    mLED_2  = !mLED_2;


#endif
