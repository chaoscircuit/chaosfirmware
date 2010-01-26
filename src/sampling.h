/**
 * \file sampling.h
 * \brief Header file for sampling.c
 */
 
#ifndef SAMPLING_H
#define SAMPLING_H

#include <plib.h>
#include "mdac.h"
#include "USB\usb.h"
#include "globals.h"

#define SMP_NUM_BUFFERS 20
#define SMP_BUFFER_SIZE 1024

#define SMP_BUF_RTS 0x01

extern BYTE SMP_BUFFER_STATE[SMP_NUM_BUFFERS];
extern BYTE SMP_BUFFER[SMP_BUFFER_SIZE * SMP_NUM_BUFFERS];
extern int SMP_SAMPLE_BUFFER_NUM;
extern int SMP_SEND_BUFFER_NUM;
extern int SMP_MODE;
extern int SMP_PACKET_OFFSET;
extern int SMP_PACKET_ID;
extern int SMP_LAST_TRANSMISSION;

void SMP_init(void);
void SMP_start(word mdac_value);
void SMP_sendData(void);
byte* SMP_getNextSendBuffer(void);
void SMP_end(void);
void SMP_gotoDemonstrationMode(void);


#endif
