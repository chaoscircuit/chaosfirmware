/**
 * \file chaos.h
 * \brief Header file for chaos.c
 */

#ifndef CHAOS_H
#define CHAOS_H

#include <plib.h>
#include "globals.h"

#define CHAOS_PORT LATF
#define CHAOS_PORT_DIR TRISF
#define CHAOS_PIN 0x02

void CHAOS_init(void);
void CHAOS_turnOn(void);
void CHAOS_turnOff(void);

#endif
