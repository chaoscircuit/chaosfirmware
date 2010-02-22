/**
 * \file encoder.h
 * \brief Header file for encoder.c
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <plib.h>

extern int ENC_elapsed;

void ENC_init(void);
void ENC_intEnable(void);
void ENC_intDisable(void);
#endif
