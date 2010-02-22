/** 
 * \file adc.c
 * \brief Routines for interacting with the ADC
 */
 
#include "adc.h"

void ADC_init(void) {
    /** 
     * Initialize the ADC.
     *
     * This should only need to be called one time on boot up
     *
     *
     * ADC timing is set using this math:
     *
     * sample and hold time = (200ns * ADC_SAMPLE_TIME)
     *
     * time per sample needs to be 4.62962963 microseconds
     *
     * Sample time: 10*200ns = 2microseconds
     *
     * Conversion time: 12*200ns = 2.4microseconds
     *
     * Total time: 4.4 microseconds
     */
    unsigned int pins;
    
    // ensure the ADC is off 
    CloseADC10();

    // use AN2,3,4 as the analog inputs
    pins = ENABLE_AN2_ANA | ENABLE_AN3_ANA | ENABLE_AN4_ANA;

    // write the configurations

    //         Module on       Integer Data      Automatic sampling
    AD1CON1 = (ADC_MODULE_ON | ADC_FORMAT_INTG | ADC_CLK_AUTO | 
            // Automatic Sampling
               ADC_AUTO_SAMPLING_ON);

    //         Use external references     No calibration
    AD1CON2 = (ADC_VREF_EXT_EXT |        ADC_OFFSET_CAL_DISABLE | 
            // Scan Mode     3 samples per interrupt
               ADC_SCAN_ON | ADC_SAMPLES_PER_INT_3 |  
            // Use double buffers     Don't alternate inputs
               ADC_ALT_BUF_ON |       ADC_ALT_INPUT_OFF);

    //         PB (40mHz) clock  ADC sample time (1.5 microseconds) //2*(3+1) clock divider
    AD1CON3 = (ADC_CONV_CLK_PB | ADC_SAMPLE_TIME_10 |               3);
    
    // set the pins to be analog inputs
    mPORTBSetPinsAnalogIn(pins);
    
    // set the pins to be part of the ADC scan
    AD1CSSL = (pins);

    // enable the ADC interrups
    ConfigIntADC10(ADC_INT_ON | ADC_INT_PRI_7);
    IFS1bits.AD1IF = 0;

    // set the port direction for the test pin (RB9)
    TRISB = TRISB & ~0x0100;

    // turn the ADC on
    EnableADC10();                             

    ADC_led_pin = 0x0100;
    
    // wait for the first conversion to complete 
    while ( ! mAD1GetIntFlag() );
}

void ADC_storeMostRecent() {
    /** 
     * Store the most recent ADC result.
     * 
     * This function gets called from the ADC ISR each time the 3 pin
     * scan completes. The data points are then copied to the active 
     * data buffer
     */
    
    short int x1, x2, x3;
    // memory address of the inactive ADC buffer
    unsigned int offset;
    // stores the data read from the ADC buffer
    unsigned int data;

    // determine which buffer is idle and create an offset 
    offset = 8 * (((~ReadActiveBufferADC10()) & 0x01));

    // read conversion results
    x1 = ReadADC10(offset);
    x2 = ReadADC10(offset + 1);
    x3 = ReadADC10(offset + 2);

    // combine the three 10 bit results into one 32 bit dword
    data = (x1 << 2) | (x2 << 12) | (x3 << 22);

    // write data to the buffer
    *(unsigned int*)&(SMP_BUFFER[SMP_PACKET_OFFSET]) = data;
    SMP_PACKET_OFFSET += 4;

    if ( SMP_PACKET_OFFSET >= (SMP_SAMPLE_BUFFER_NUM+1)*SMP_BUFFER_SIZE) {
        // A 1k block has been filled
        
        // mark it as ready to send
        SMP_BUFFER_STATE[SMP_SAMPLE_BUFFER_NUM] |= SMP_BUF_RTS;
        
        // go on to the next block
        SMP_SAMPLE_BUFFER_NUM = (SMP_SAMPLE_BUFFER_NUM + 1) % SMP_NUM_BUFFERS;
        SMP_PACKET_OFFSET = (SMP_SAMPLE_BUFFER_NUM * SMP_BUFFER_SIZE);

        // mark this buffer with a sample packet id
        // this number increments so that the PC knows 
        // if it is missing packets
        SMP_PACKET_ID++;
        *(unsigned int*)&(SMP_BUFFER[SMP_PACKET_OFFSET]) = SMP_PACKET_ID;
        SMP_PACKET_OFFSET += 4;
    }
}

/* ADC ISR */
void __ISR(_ADC_VECTOR, ipl7) ADCHandler(void) {
    /** 
     * Handle the ADC interrupt
     *
     * This is painfully simple. It clears the interrupt flag then calls
     * ADC_storeMostRecent to handle the new data
     */
     
    // clear the interrupt flag                         
    IFS1bits.AD1IF = 0;

    // pull up RB8 for testing
    LATB = LATB | ADC_led_pin;

    if ( SMP_MODE == SAMPLING ) {
        ADC_storeMostRecent();
    }
    
    // pull RB8 back down
    LATB = LATB & ~ADC_led_pin;

}

