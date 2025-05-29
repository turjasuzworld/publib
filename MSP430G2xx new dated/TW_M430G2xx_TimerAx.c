/*
 * TW_M430G2xx_TimerAx.c
 *
 *  Created on: May 24, 2020
 *      Author: Turjasu
 *
 *
 *      !!!!!!!!!!!ENABLE GIE IN YOUR CODE!!!!!!!!!!!!!!
 */

#include "TW_M430G2xx_TimerAx.h"
#include "TW_CapaJoystick.h"

unsigned int TimerVal = 0, delayNeededInSec = _10Seconds;
void (*ptr)() = &AcquireValueJoystk;                         // Register Callback when timer expires

void B(void (*ptr)())
{
    (*ptr) (); // callback to A
}


/*
 * Configures Timer A0 CCR0
 * in interrupt  / polling mode
 * timerVal should be used in such a way that
 * 1 second intervals can be calculated
 */
void configTA0(unsigned int timerVal, unsigned char mode)
{
    switch (mode)
    {
        case _interruptMode:

            CCTL0 |= CCIE;                             // CCR0 interrupt enabled
            __bis_SR_register(GIE);
            break;
        case _pollingMode:

            CCTL0 &= ~CCIE;                             // CCR0 interrupt enabled
            break;
        default:
            break;
    }


    if(timerVal<65536)
        {
            CCR0 = timerVal;          // Validity check
            TimerVal = timerVal;
        }
    TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode

//    __bis_SR_register(GIE);         // Enter LPM0 w/ interrupt  __bis_SR_register(LPM0_bits + GIE)
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
    delayNeededInSec--;
    if(delayNeededInSec==0)
    {
        delayNeededInSec = _10Seconds;
        //B(ptr); //Either do an event based callback , else set a master global bit _deviceStatus
        _deviceStatus |= _acquireSensorValueFlag;
    }

  CCR0 += TimerVal;                            // Add Offset to CCR0
}
