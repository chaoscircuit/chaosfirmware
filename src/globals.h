/**
 * \file globals.h
 * \brief Define several global definitions used in many places
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#define VERSION 1002

#include <GenericTypeDefs.h>
#include <peripheral/int.h>
#include <peripheral/ports.h>
#include <USB/usb.h>
#include <USB/usb_device_generic.h>
#include <plib.h>
#include "led.h"

#define DEBUG
//#define STARTERKIT

#define TRUE 1 == 1
#define FALSE !TRUE

extern BYTE BUF[1024];

#define DEMONSTRATION 0
#define SAMPLING 1

/* Clock Macros **********************************************************/
#ifndef SYS_CLOCK
    //#error "Define SYS_CLOCK (ex. -DSYS_CLOCK=80000000) on compiler command line"
#endif

#define GetSystemClock()            SYS_CLOCK
#define GetPeripheralClock()        SYS_CLOCK
#define GetInstructionClock()

void GLB_init();
void GLB_reset();

#endif
