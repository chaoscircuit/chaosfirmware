/**
 * \file tone.h
 * \brief Header file for tone.c
 */

#ifndef TONE_H
#define TONE_H

#include <plib.h>
#include "globals.h"

#define TONE_PORT   IOPORT_B
#define TONE_PIN    BIT_9
#define TMR1_TOGGLES_PER_SEC 100000
#define TEMPO_MULTIPLER 1

extern int TONE_tone;
extern char* TONE_notes;
extern int* TONE_beats;
extern int TONE_play;
extern int TONE_count;
void TONE_playNote(char note);
void TONE_playSong(int song);
#endif
