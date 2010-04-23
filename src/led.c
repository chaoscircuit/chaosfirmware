/**
 * \file led.c
 * \brief Routines for controlling the LEDs
 */

#include "led.h"

void LED_init() {
    /**
     * Initialize the LEDs
     */
    mLED_Init();
}

void LED_test() {
    /**
     * Test the LEDs
     */
    // just flash the LEDs a few times

    int i,j;
    int delay = 100000;
    char old_lat = LED_LAT;
   
    LED_LAT = LED_LAT | 0x01;
    for(i = 0; i < delay; i++);
    LED_LAT = LED_LAT & ~0x01;
    for(i = 0; i < delay; i++);
    
    LED_LAT = old_lat;
}
