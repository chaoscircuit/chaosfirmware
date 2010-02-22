/**
 * \file tone.c
 * \brief Routines for controlling the internal speaker
 */
 
#include "tone.h"
unsigned long TMR1_ticks;
int TONE_tone;
char song0notes[] = "E E E C E G g ";
int song0beats[] = {100, 75, 
               100, 150, 
               100, 150, 
               100, 50, 
               100, 150, 
               100, 275, 
               100, 100};

char song1notes[] = "e e e c g e c g e B B B C g p c g e ";
int song1beats[] = {300, 100, 
               300, 100, 
               300, 100, 
               300, 100, 
               160, 60, 
               300, 100, 
               300, 100, 
               160, 60, 
               500, 100,
               300, 100, 
               300, 100, 
               300, 100, 
               300, 100, 
               160, 60, 
               300, 100, 
               300, 100, 
               160, 60, 
               500, 100};
int TONE_count;
char *TONE_notes;
int *TONE_beats;
int TONE_play;

void TONE_init() {
    /**
     * Initialize the tone library and the timer for the buzzer
     */
    #define PB_DIV         1
    #define PRESCALE       8
    #define T1_TICK       (SYS_CLOCK/PB_DIV/PRESCALE/TMR1_TOGGLES_PER_SEC)
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_8, T1_TICK);
    DisableIntT1;
    INTEnableSystemMultiVectoredInt();
    TMR1_ticks = 0;
    TONE_tone = 0;
    TONE_play = FALSE;
    PORTSetPinsDigitalOut(TONE_PORT, TONE_PIN);
    PORTClearBits(TONE_PORT, TONE_PIN);
    TONE_tone = 0;
    TONE_notes = &song0notes[0];
    TONE_beats = &song0beats[0];
    TONE_count = 14;
 }
 
 void TONE_playNote(char note) {
     /** 
     *  Sets the frequency required to play a note in a 2 octave range.  
     *  Lowercase notes are in the bottom octave, uppercase notes in the top.
     */
    char names[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'p', ' '};
    int tones[] = {227, 202, 192, 170, 152, 143, 128, 114, 101, 96, 85, 76, 72, 64, 160, 0};
    int i;    
    for(i = 0; i < 16; i++) {
        if(names[i] == note) {
            TONE_tone = tones[i];
            return;
        }
    }
 }
 
 void TONE_playSong(int song) {
    if(song == 0) {
        TONE_count = 14;
        TONE_notes = &song0notes[0];
        TONE_beats = &song0beats[0];
    } else {
        TONE_count = 36;
        TONE_notes = &song1notes[0];
        TONE_beats = &song1beats[0];
    }
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_7);
    TONE_play = TRUE;
 }
 
 
/* Timer 1 ISR */
void __ISR(_TIMER_1_VECTOR, ipl7) Timer1Handler(void) {
    /**
     * Handle interrupts for timer1
     * High priority interrupt, only ever has to toggle a pin
     */
    mT1ClearIntFlag();
    TMR1_ticks++;
    
    if(TONE_tone == 0) {
        PORTClearBits(TONE_PORT, TONE_PIN);
    } else {
        if((TMR1_ticks % TONE_tone) == 0) {
            PORTToggleBits(TONE_PORT, TONE_PIN);
        }
    }
}

