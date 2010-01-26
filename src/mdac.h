/**
 * \file mdac.h
 * \brief Header file for mdac.c
 */

#ifndef MDAC_H
#define MDAC_H

#include <plib.h>
#include "local_typedefs.h"

extern int MDAC_value;

enum MDAC_StepSize {
    MDAC_SMALL_STEP = 1,
    MDAC_MEDIUM_STEP = 10,
    MDAC_LARGE_STEP = 50
};

void MDAC_init(void);
void MDAC_send(word data);
void MDAC_setValue(word value);
void MDAC_increment(enum MDAC_StepSize size);
void MDAC_decrement(enum MDAC_StepSize size);

#endif
