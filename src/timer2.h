/**
 * \file timer2.h
 * \brief Header file for timer2.c
 */

#ifndef TIMER2_H
#define TIMER2_H

#include <plib.h>
#include <p32xxxx.h>
#include "globals.h"

#define TMR2_TOGGLES_PER_SEC 1000

void TMR2_init(void);
int TMR2_ticks;

#endif
