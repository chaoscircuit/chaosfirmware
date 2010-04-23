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
    CHAOS_PORT_DIR = CHAOS_PORT_DIR & ~CHAOS_PIN;

    // makes sure it is off
    CHAOS_PORT = CHAOS_PORT & ~CHAOS_PIN;
    
    // turn on the chaos circuit
    CHAOS_turnOn();
}

void CHAOS_turnOn() {
    /**
     * Turn on the chaos circuit
     */
    CHAOS_PORT = CHAOS_PORT | CHAOS_PIN;
}

void CHAOS_turnOff() {
    /**
     * Turn off the chaos circuit
     */
    CHAOS_PORT = CHAOS_PORT & ~CHAOS_PIN;
}
