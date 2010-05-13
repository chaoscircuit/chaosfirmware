/**
 * \file chaos.c
 * \brief Routines for controlling the Chaos circuit
 */

#include "chaos.h"

void CHAOS_init() {
    /**
     * Initialize the chaos circuit
     */
	// sets the port direction
	TRISE = TRISE & ~0x10;

	// makes sure it is off
    LATE = LATE & ~0x10;
    
    // turn on the chaos circuit
    CHAOS_turnOn();
}

void CHAOS_turnOn() {
    /**
     * Turn on the chaos circuit
     */
    LATE = LATE | 0x10;
    mChaos_LED_Off();
}

void CHAOS_turnOff() {
    /**
     * Turn off the chaos circuit
     */
    LATE = LATE & ~0x10;
    mChaos_LED_On();
}
