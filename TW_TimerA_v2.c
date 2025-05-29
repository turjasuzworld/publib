/*
 * TW_TimerA.c
 *
 * Configures TimeA0, TimerA1 etc for operations and specific capture/compare routines will be added also.
 * If you enable interrupt here, Do include your ISR in the main program with #PRAGMA...!
 *  Created on: Jun 27, 2018
 *      Author: Turjasu
 */

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <TW_TimerA_v2.h>

/*
 * Generate a Delay with entering LPM to schedule cyclic
 * tasks. The CPU enters LPM0 and wakes after timer reaches
 * upto the value in CCR0Val. Give CCR0Val depending on
 * Clock Freq.
 */
volatile uint16_t RefillValue = 0, NoOfOverflowsCCR0 = 0, countDown = 0;
void _ElapseTimeCCR0_A0(unsigned char ClkSrc, unsigned char ClkDiv, unsigned char Mode, uint16_t CCR0Val, uint16_t ovrflows)
{
    RefillValue = CCR0Val;
    NoOfOverflowsCCR0 = ovrflows;
    countDown = ovrflows;
    TA0CTL &= 0x00ff;
    switch (ClkSrc)
    {
        case SMCLK:
                TA0CTL |= TASSEL_2;             // Set SMCLK as the clock for Timer A0
            break;
        case ACLK:
                TA0CTL |= TASSEL_1;             // Set ACLK as the clock for Timer A0
            break;
        case INCLK:
                TA0CTL |= TASSEL_3;             // Set INCLK as the clock for Timer A0, check if your MSP supports
            break;
        case TACLK:
         default:
            break;
    }

    TA0CTL &= 0xFF3F;               // Doesnt disturb the other configurations, but clears previous DIVx. TACLR also does this
        switch (ClkDiv)
        {
            case 1:
                TA0CTL |= ID_0;                 // No divisioon of clock
                break;
            case 2:
                TA0CTL |= ID_1;                 // No divisioon of clock
                 break;
            case 4:
                TA0CTL |= ID_2;                 // No divisioon of clock
                 break;

            case 8:
                TA0CTL |= ID_3;                 // No divisioon of clock
                 break;
            default:
                TA0CTL |= ID_0;
                break;
        }

        TA0CTL &= 0xFFC0;
        switch (Mode)
        {
            case STOP:
                TA0CTL |= MC_0;                  // Stop the Timer and freeze TAR
                break;

            case UP:
                TA0CTL |= MC_1;                  // Stop the Timer and freeze TAR
                break;

            case CONT:
                TA0CTL |= MC_2 ;                 // Stop the Timer and freeze TAR
                break;

            case UP_DOWN:
                TA0CTL |= MC_3;                  // Stop the Timer and freeze TAR
                break;
            default:
                break;
        }

        CCTL0 |= CCIE;                             // CCR0 interrupt enabled
        if(CCR0Val < 65535) CCR0 = CCR0Val;

        __bis_SR_register(LPM0_bits + GIE);
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
    countDown--;
    if(countDown == 0)
    {
        countDown = NoOfOverflowsCCR0;
        if(TA0CTL & 0x20)//Need to check if MCx = 2/3 for which BIT5 of TA0CTL will be = 1. For Modes 0 & 1 BIT5 == 0 Always
        {
            CCR0 += RefillValue;
        }
        __bic_SR_register_on_exit(LPM0_bits);
    }

}
