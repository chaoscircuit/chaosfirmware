/**
 * \file debug_uart.h
 * \brief Header file for debug_uart.c
 */

#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <plib.h>
#include <GenericTypeDefs.h>
#include "globals.h"

void DBG_SendData(char *data);

inline void
DBG_WriteString(char *data) {
    #ifdef DEBUG
        putsUART1(data);
    #endif
}

inline void
DBG_WriteInt(int data) {
    #ifdef DEBUG
        char str[32];
        sprintf(str, "%d\r\n", data);
        putsUART1(str);
    #endif
}
#endif
