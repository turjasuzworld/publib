/*
 * TW_MSP430x2xx_CLOCKSET.c
 *
 *  Created on: Jun 15, 2018
 *      Author: Turjasu
 */

#include <msp430.h>
#include "TW_MSP430x2xx_CLOCKSET.h"
//=======================================Sets HF Mode,else runs CPU @ 1 MHZ=================================//
int SetHFMode(int XtalFreq)
{
    // Doesnot determine the reason for OFIFG setting, only sets the LFXT1 to HF Mode//
    int condition = 0, retry=0;
    // First we ensure the DCO is running at 1 MHz Speed, so that 50uS delay for HF Mode can be calculated
    if (CALBC1_1MHZ==0xFF)                 // If calibration constant erased
     {
       while(1);                               // do not load, trap CPU!!
     }
     DCOCTL = 0;                               // Select lowest DCOx and MODx settings
     BCSCTL1 = CALBC1_1MHZ;                    // Set range
     DCOCTL = CALDCO_1MHZ;                     // Set DCO step + modulation */

    _bic_SR_register(OSCOFF);                   // Clears  OSCOFF Flag
    BCSCTL1 |= XTS;                             // sets the XTS = 1 for HF Mode, else in LF Mode
    if((BCSCTL1 & XTS) == 0)                    // Some devices don't support HF, then this returns -2 and keeps CPU in DCO 1 Mhz
        {
            return -2;
        }
    if((XtalFreq >= 3)&&(XtalFreq <=16))
        {
            BCSCTL3 |= LFXT1S_2 ;
        }
    else if ((XtalFreq >= 1)&&(XtalFreq <3))
        {
            BCSCTL3 |= LFXT1S_1 ;
        }
    else if ((XtalFreq > 0)&&(XtalFreq <1))
        {
            BCSCTL3 |= LFXT1S_0 ;
        }
    else return -1;

    BCSCTL3 &= ~XCAP_0;

    IFG1 &= ~OFIFG;

    while(((IFG1 & OFIFG) != 0)&&(retry < 10))
    {
        _delay_cycles(100);
        IFG1 &= ~OFIFG;
        retry++;
        if(retry == 9) condition = 404;
    }

    if((BCSCTL3&LFXT1OF)!=0) condition++; // if LFXT1OF flag goes high, condition returned is 1, if it ofifg=1 and lfxt1of = 405

    BCSCTL2 |= SELM_3;

    return condition;                      // Healthy return is 0

}
//====================== Sets Prescalar ============================================//
// Put 1 for /1, 2 for /2, 4 for /4 and 8 for /8. This format is for all clocks
void SetPresclrAMSMClk(unsigned char divax,unsigned char divmx,unsigned char divsx)
{
    switch (divax)
    {
        case 1:
            BCSCTL1 |= DIVA_0;
            break;
        case 2:
            BCSCTL1 |= DIVA_1;
            break;
        case 4:
            BCSCTL1 |= DIVA_2;
            break;
        case 8:
            BCSCTL1 |= DIVA_3;
            break;

        default:
            BCSCTL1 |= DIVA_0;
            break;
    }

    switch (divmx)
       {
           case 1:
               BCSCTL2 |= DIVM_0;
               break;
           case 2:
               BCSCTL2 |= DIVM_1;
               break;
           case 4:
               BCSCTL2 |= DIVM_2;
               break;
           case 8:
               BCSCTL2 |= DIVM_3;
               break;

           default:
               BCSCTL2 |= DIVM_0;
               break;
       }

    switch (divsx)
           {
               case 1:
                   BCSCTL2 |= DIVS_0;
                   break;
               case 2:
                   BCSCTL2 |= DIVS_1;
                   break;
               case 4:
                   BCSCTL2 |= DIVS_2;
                   break;
               case 8:
                   BCSCTL2 |= DIVS_3;
                   break;

               default:
                   BCSCTL2 |= DIVS_0;
                   break;
           }

}
//======================Start Using and set VLOCLK for ACLK if you plan to use LPM2 or >2=====
void    SetnStartVLOCLK(void)
{
    BCSCTL1     &=  ~XTS;
    BCSCTL3     &=  ~LFXT1S0;
    BCSCTL3     &=  ~LFXT1S1;
    BCSCTL3     |=   LFXT1S_2;
}
//=====================SetClock Freq=====================================
//
void  SetDCOClk(unsigned char SetDCOFreq)
{

    switch (SetDCOFreq)
    {
        case 1:
            //1Mhz
                 if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
                 {
                   while(1);                               // do not load, trap CPU!!
                 }
                 DCOCTL = 0;                               // Select lowest DCOx and MODx settings
                 BCSCTL1 = CALBC1_1MHZ;                    // Set range
                 DCOCTL = CALDCO_1MHZ;                     // Set DCO step + modulation */
            break;

        case 8:
            //8Mhz
                 if (CALBC1_8MHZ==0xFF)                    // If calibration constant erased
                 {
                   while(1);                               // do not load, trap CPU!!
                 }
                 DCOCTL = 0;                               // Select lowest DCOx and MODx settings
                 BCSCTL1 = CALBC1_8MHZ;                    // Set range
                 DCOCTL = CALDCO_8MHZ;                     // Set DCO step + modulation */
            break;

        case 12:
            //12Mhz
                 if (CALBC1_12MHZ==0xFF)                   // If calibration constant erased
                 {
                   while(1);                               // do not load, trap CPU!!
                 }
                 DCOCTL = 0;                               // Select lowest DCOx and MODx settings
                 BCSCTL1 = CALBC1_12MHZ;                   // Set range
                 DCOCTL = CALDCO_12MHZ;                    // Set DCO step + modulation*/
            break;

        case 16:
            //16Mhz
                 if (CALBC1_16MHZ==0xFF)                   // If calibration constant erased
                 {
                   while(1);                               // do not load, trap CPU!!
                 }
                 DCOCTL = 0;                               // Select lowest DCOx and MODx settings
                 BCSCTL1 = CALBC1_16MHZ;                   // Set range
                 DCOCTL = CALDCO_16MHZ;                    // Set DCO step + modulation*/
            break;

        default:
            //1Mhz
            if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
            {
                while(1);                               // do not load, trap CPU!!
            }
            DCOCTL = 0;                               // Select lowest DCOx and MODx settings
            BCSCTL1 = CALBC1_1MHZ;                    // Set range
            DCOCTL = CALDCO_1MHZ;                     // Set DCO step + modulation */
            break;
    }

}
//============Set Clock Mode for SMCLK=============================
void SetSMCLKSrc(unsigned char Src)
{
    switch (Src)
    {
        case 0:
            BCSCTL2 |= SELS;           // XT2 if present in chip, else lfxt or vloclk
            break;
        case 1:
            BCSCTL2 &= ~SELS;           // DCO
             break;
        default:
            BCSCTL2 &= ~SELS;           // DCO
            break;
    }
}
