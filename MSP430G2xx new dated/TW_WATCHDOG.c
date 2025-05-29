/*
 * TW_WATCHDOG.c
 *
 * Congures the watchdogtimer in the Watch Dog mode for MSP430G2553 (tested).
 * Might work for other families, please check your self.
 *
 *  Created on: Jul 24, 2018
 *      Author: Turjasu
 */
#include <msp430.h>
#include "TW_WATCHDOG.h"



void ConfigWDGT(unsigned char ClkDiv, unsigned char ClkSel, unsigned char Mode, unsigned char RES_NMIfn, unsigned char NMISenseType)
{
    unsigned short WDTConstruct = 0;
        switch (ClkDiv)
        {
            case DivClkby32768:
                WDTConstruct &= ~DivClkby32768;
                break;
            case DivClkby8192: ////////////////////////////approx 1.46 Hz
                WDTConstruct &= ~DivClkby8192;
                break;
            case DivClkby512:
                WDTConstruct &= ~DivClkby512;
                break;
            case DivClkby64:
                WDTConstruct &= ~DivClkby64;
                break;

            default:
                WDTConstruct &= ~DivClkby8192;
                break;
        }

        (ClkSel == 1) ?  (WDTConstruct |= BIT2):(WDTConstruct &= ~ BIT2);

        (Mode == 1) ?  (WDTConstruct |= BIT4):(WDTConstruct &= ~ BIT4);

       (RES_NMIfn == 1) ?  (WDTConstruct |= BIT5):(WDTConstruct &= ~ BIT5);

        (NMISenseType == 1) ?  (WDTConstruct |= BIT6):(WDTConstruct &= ~ BIT6);

        WDTCTL  =  WDTPW + WDTCNTCL + WDTConstruct;

}

   void        ClearWDT(void)
{
       WDTCTL  |= WDTPW+WDTCNTCL;
}

  unsigned char        CheckWDTFlg(void)
   {
        if(IFG1&WDTIFG)
            {
            IFG1 &= ~WDTIFG; // cler the flag
                return 1;
            }
        else return 0;
   }

  void                         RestartWDT(void)
  {
      WDTCTL &= WDTPW + 0x7F;   //restart watchdog timer
  }

