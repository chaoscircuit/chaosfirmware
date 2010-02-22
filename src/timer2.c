/**
 * \file timer2.c
 * \brief Routines to use timer2
 */

#include "timer2.h"
#include "encoder.h"
#include "sampling.h"
#include "tone.h"

int note_count;
int note_stop;

void TMR2_init() {
    /**
     * Initialize timer2
     */
    #define PB_DIV         1
    #define PRESCALE       64
    #define T2_TICK       (SYS_CLOCK/PB_DIV/PRESCALE/TMR2_TOGGLES_PER_SEC)
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_64, T2_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_6);
    INTEnableSystemMultiVectoredInt();
    TMR2_ticks = 0;
    note_count = 0;
    note_stop = -1;
}

/* Timer 2 ISR */
void __ISR(_TIMER_2_VECTOR, ipl6) Timer2Handler(void) {
    /**
     * Handle interrupts for timer2
     * General purpose 1 ms timer controls a lot of the IO on the device
     */
     
    // clear the interrupt flag                         
    mT2ClearIntFlag();
    
    TMR2_ticks++;
    ENC_elapsed++;
    
    SMP_LAST_TRANSMISSION++;
    
    if(SMP_LAST_TRANSMISSION > 100) {
        SMP_gotoDemonstrationMode();
    }
    
    if(TMR2_ticks % 500 == 0) {
        mHeartbeat_LED_Toggle();
    }
    
    if(TONE_play == TRUE) {
        if(note_stop == -1) {
            note_stop = TMR2_ticks + TONE_beats[note_count]*TEMPO_MULTIPLER;
        }
        
        if(TMR2_ticks == note_stop) {
            note_stop = TMR2_ticks + TONE_beats[note_count]*TEMPO_MULTIPLER;
            TONE_playNote(TONE_notes[note_count]);
            note_count++;
            if(note_count >= TONE_count) {
                TONE_play = FALSE;
                note_count = 0;
                note_stop = -1;
                TONE_playNote(' ');
                DisableIntT1;
            }
        }
    }
}

