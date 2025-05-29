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
#include "TW_TimerA.h"

void ConfigureTimerA0(unsigned char ClkSrc, unsigned char ClkDiv, unsigned char Mode, unsigned short CCR0Val, bool TAClr, bool TACCROIntrpt )
{
    switch (ClkSrc)
    {
        case SMCLK:
                TA0CTL &= 0x00ff;
                TA0CTL |= TASSEL_2;             // Set SMCLK as the clock for Timer A0
            break;

        case ACLK:
                TA0CTL &= 0x00ff;
                TA0CTL |= TASSEL_1;             // Set ACLK as the clock for Timer A0

            break;

        case INCLK:
                TA0CTL &= 0x00ff;
                TA0CTL |= TASSEL_3;             // Set INCLK as the clock for Timer A0, check if your MSP supports

            break;

        case TACLK:
                TA0CTL &= 0x00ff;               // Set TACLK as the clock for Timer A0

            break;

        default:
            TA0CTL &= 0x00ff;
            TA0CTL |= TASSEL_2;                 // Set SMCLK as the clock for Timer A0 as default

            break;
    }


        switch (ClkDiv)

        {
            case 1:
                TA0CTL &= 0x033f;               // Doesnt disturb the other configurations, but clears previous DIVx. TACLR also does this
                TA0CTL |= ID_0;                 // No divisioon of clock
                break;

            case 2:
                TA0CTL &= 0x033f;               // Doesnt disturb the other configurations, but clears previous DIVx. TACLR also does this
                TA0CTL |= ID_1;                 // No divisioon of clock

                 break;

            case 4:
                TA0CTL &= 0x033f;               // Doesnt disturb the other configurations, but clears previous DIVx. TACLR also does this
                TA0CTL |= ID_2;                 // No divisioon of clock

                 break;

            case 8:
                TA0CTL &= 0x033f;               // Doesnt disturb the other configurations, but clears previous DIVx. TACLR also does this
                TA0CTL |= ID_3;                 // No divisioon of clock
                 break;
            default:
                break;
        }

        switch (Mode)
        {
            case STOP:
                TA0CTL &= 0x03cf;               // Doesnt disturb the other configurations, but clears previous Mode
                TA0CTL |= MC_0;                  // Stop the Timer and freeze TAR
                break;

            case UP:
                TA0CTL &= 0x03cf;               // Doesnt disturb the other configurations, but clears previous Mode
                TA0CTL |= MC_1;                  // Stop the Timer and freeze TAR
                break;

            case CONT:
                TA0CTL &= 0x03cf;               // Doesnt disturb the other configurations, but clears previous Mode
                TA0CTL |= MC_2 ;                 // Stop the Timer and freeze TAR
                break;

            case UP_DOWN:
                TA0CTL &= 0x03cf;               // Doesnt disturb the other configurations, but clears previous Mode
                TA0CTL |= MC_3;                  // Stop the Timer and freeze TAR
                break;
            default:
                break;
        }

        if(TAClr)
        {
            TA0CTL |= TACLR;                    // Setting this will clear the IDx, TAR and the MCx. So run this SR with suitable values again
        }
        else
        {
            TA0CTL &= ~TACLR;
        }

        CCR0 = CCR0Val;

        if(TACCROIntrpt)
        {

            CCTL0 |= CCIE;                             // CCR0 interrupt enabled
        }
        else
        {
            CCTL0  &= ~CCIE;                             // CCR0 interrupt enabled
            CCTL0  &= ~CCIFG;
        }



}
