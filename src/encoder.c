/**
 * \file encoder.c
 * \brief Routines for handling the encoder
 */

#include "encoder.h"
#include "local_typedefs.h"
#include "mdac.h"
#include "tone.h"
// Be sure to change the read bits in the ISR if changing these values
#define ENCA BIT_4
#define ENCB BIT_5
#define ENCSWITCH BIT_15
#define CN_ENABLE (CN12_ENABLE | CN17_ENABLE | CN18_ENABLE)

/* Uncomment one of the following STEP_CALCULATIONS to determine how the encoder responds */
#define DISCRETE_STEP_CALCULATIONS
//#define LINEAR_STEP_CALCULATIONS
//#define INVERSE_STEP_CALCULATIONS
//#define SQUARED_STEP_CALCULATIONS

#define UP 1
#define DOWN 0

#define MED_STEP_SIZE_THRESHOLD  180
#define LARGE_STEP_SIZE_THRESHOLD  80

int ENC_elapsed;

static byte lastValue;
static byte currentValue;
static byte lastSwitchState;

static const byte cw[4] = {1, 3, 0, 2};
static const byte ccw[4] = {2, 0, 3, 1};

void ENC_init() {
    /**
     * Initialize the encoder
     */
    //Set up encoder counter
    ENC_elapsed = 0;
    lastSwitchState = UP;
    // set up the on change interupt

    PORTSetPinsDigitalIn(IOPORT_B, ENCSWITCH);
    PORTSetPinsDigitalIn(IOPORT_F, ENCA | ENCB);
    mCNOpen(CN_ON | CN_IDLE_CON, CN_ENABLE, CN12_PULLUP_ENABLE);    
    mPORTBRead();
    mPORTFRead();
    ConfigIntCN(CHANGE_INT_ON | CHANGE_INT_PRI_5);
    INTEnableSystemMultiVectoredInt();

}

void ENC_intEnable() {
    /**
     * Enable the encoder interrupts
     */
    EnableCN12;
    EnableCN17;
    EnableCN18;
}

void ENC_intDisable() {
    /**
     * Disable the encoder interrupts
     */
    DisableCN12;
    DisableCN17;
    DisableCN18;
}

// ISRs
void __ISR(_CHANGE_NOTICE_VECTOR, ipl5) ChangeNoticeHandler(void) {
    /**
     * Handle on change interrupts related to the encoder.
     * Due to the how the grey code in the encoder works, this interrupt fires
     * approximately 4 times for every "click" of the encoder.  By keeping track of
     * the elapsed time between every 4th interrupt, we can adjust the step size based
     * on the duration of the interval.  There are several different implementations shown
     * below that can be toggled on and off with the flags at the top of the function.
     */
    static cwsteps = 0;
    static ccwsteps = 0;
    
    // Clear the ISR
    mCNClearIntFlag();
    
    // Read PORTF and PORTB to clear CN mismatch condition
    mPORTFRead();
    mPORTBRead();
    
    // Check if encoder was pressed in
    byte switchState = PORTReadBits(IOPORT_B, BIT_15) >> 15;
    if(switchState != lastSwitchState) {
        if(switchState == UP) {
            if(MDAC_value == 1985) {
                TONE_playSong(0);
            } else if(MDAC_value == 1977) {
                TONE_playSong(1);
            }
            MDAC_resetValue();
            
        }
        lastSwitchState = switchState;
    }
        
    lastValue = currentValue;
    
    // Since we are reading bits 4 and 5 we shift to get the grey code
    currentValue = PORTReadBits(IOPORT_F, ENCA | ENCB);
    currentValue = currentValue >> 4;
    
    
    // Calculate what size step to take.
    #ifdef DISCRETE_STEP_CALCULATIONS
        // Discrete
        enum MDAC_StepSize step;
        if(ENC_elapsed < LARGE_STEP_SIZE_THRESHOLD) {
            step = MDAC_LARGE_STEP;
        } else if(ENC_elapsed < MED_STEP_SIZE_THRESHOLD) {
            step = MDAC_MEDIUM_STEP;
        } else {
            step = MDAC_SMALL_STEP;
        }
    #endif
    
    #ifdef LINEAR_STEP_CALCULATIONS
        // Linear
        int step = 150 - ((40*ENC_elapsed) >> 7);
    #endif

    #ifdef INVERSE_STEP_CALCULATIONS
        // inverse
        int step;
        if(ENC_elapsed != 0) {
            step = 600/(ENC_elapsed);
        }
    #endif
    
    #ifdef SQUARED_STEP_CALCULATIONS
        // t-squared
        int step = 150 - ((ENC_elapsed*ENC_elapsed) >> 3);
    #endif
    
    // Cap the step size
    if(step < 1) {
        step = 1;
    } else if (step > 150) {
        step = 150;
    }
    
    // Figure out which direction we are going and if we get 4 interrupts in the 
    // same direction, increment the MDAC
    if (cw[lastValue] == currentValue) {
        cwsteps++;
        ccwsteps = 0;
        if(cwsteps % 4 == 0) {
            MDAC_decrement(step);
            
            char* str[50];
            ENC_elapsed = 0;
        }
    } else if (ccw[lastValue] == currentValue) {
        ccwsteps++;
        cwsteps = 0;
        if(ccwsteps % 4 == 0) {
            MDAC_increment(step);
            char* str[50];
            ENC_elapsed = 0;            
        }
    }
    
    
}

