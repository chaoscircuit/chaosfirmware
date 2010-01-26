/** 
 * \file adc.h
 * \brief Header file for adc.c
 */

#ifndef ADC_H
#define ADC_H

#include <plib.h>
#include "sampling.h"
#include "globals.h"

void ADC_init(void);
void ADC_read(void);
void ADC_storeMostRecent(void);

#endif
