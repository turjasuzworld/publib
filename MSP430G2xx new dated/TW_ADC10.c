/* Initializes the USCIB0 as I2C Master
 * 7 bit addressing and many functions are there
 * like single and page write, read
 * Author: Turjasu
 * DATED: 22.06.2018
 * VERSION 1.01
 * CHECKED WORKING OF ALL SOURCE CODES WITH 24C64. USE I2CSLV_Save_Burst FOR ANY GENERAL I2C SLAVE LIKE RTC OR PORT EXPANDERS
 * ADDED SPECIAL READ BURST SOURCE CODE FOR DS1307
 * Tested in: MSP430G2553
 */


#include <msp430.h>
#include "TW_ADC10.h"



void    InitADC10(unsigned char ActiveChannels, unsigned char ClkSrc, unsigned char ConvSeq, unsigned char ClkDiv)
{
    /* This routine is to be used in run once mode, else some modifications in some
     * registers will not be effected or corrupted.
     * ClkDiv = 000 to 111 for /1 to /8
     *
     */
  ADC10CTL0 &= ~ENC;                                                                // Disable ENC to modify changes
//  ADC10CTL0 |= ADC10SHT_3 + REFBURST + ADC10ON + REF2_5V + REFON;                  // ADC10SC
  ADC10CTL0 |= ADC10SHT_3 + REFBURST + ADC10ON ;

      switch (ClkSrc)
      {
        case 0x0:
            ADC10CTL1 &= ~ADC10SSEL0;                   // ADC10OSC
            ADC10CTL1 &= ~ADC10SSEL1;
            break;
        case 0x01:
            ADC10CTL1 |=  ADC10SSEL0;                   // ACLK
            ADC10CTL1 &= ~ADC10SSEL1;

            break;
        case 0x02:
            ADC10CTL1 &= ~ADC10SSEL0;                   // MCLK
            ADC10CTL1 |=  ADC10SSEL1;

            break;
        case 0x03:
            ADC10CTL1 |=  ADC10SSEL0;                   // SMCLK
            ADC10CTL1 |=  ADC10SSEL1;

            break;


        default:
            ADC10CTL1 &= ~ADC10SSEL0;                   // default : ADC10OSC
            ADC10CTL1 &= ~ADC10SSEL1;

            break;
      }

  switch (ConvSeq)
  {
    case 0:
        ADC10CTL1 &=  ~CONSEQ0;                   // Single-channel-single-conversion
        ADC10CTL1 &=  ~CONSEQ1;

        break;
    case 1:
        ADC10CTL1 |=   CONSEQ0;                   // Sequence-of-channels
        ADC10CTL1 &=  ~CONSEQ1;

        break;
    case 2:
        ADC10CTL1 &=  ~CONSEQ0;                   // Repeat-single-channel
        ADC10CTL1 |=   CONSEQ1;

        break;
    case 3:
        ADC10CTL1 |=   CONSEQ0;                   // Repeat-sequence-of-channels
        ADC10CTL1 |=   CONSEQ1;

        break;

    default:
        ADC10CTL1 &=  ~CONSEQ0;                   // Single-channel-single-conversion
        ADC10CTL1 &=  ~CONSEQ1;
        break;
  }
  ADC10CTL1 &= ~ 0xff1f;                          // Clears previous ClkDiv to 000
  unsigned short temp= 0;
  temp |= ClkDiv;
  temp <<= 5;

  ADC10CTL1 |= temp;                            // Divider forselected clock
  ADC10AE0   = ActiveChannels;                    // PA.1 ADC option select
}
//=================================Conversion Subroutine==============================
void      ConvertADC(int channel, unsigned char ICType, unsigned short* ADCVal)
{

        switch (channel)
        {
            case 0:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                break;
            case 1:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x1000;                             // INCH set to channel 1
                break;
            case 2:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x2000;                             // INCH set to channel 2
                break;
            case 3:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x3000;                             // INCH set to channel 3

                break;
            case 4:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x4000;                             // INCH set to channel 4

                break;
            case 5:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x5000;                             // INCH set to channel 5

                break;
            case 6:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x6000;                             // INCH set to channel 6

                break;
            case 7:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x7000;                             // INCH set to channel 7

                break;
            case 8:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x8000;                             // INCH set to channel 8

                break;
            case 9:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0x9000;                             // INCH set to channel 9

                break;

            default:
                break;
         }

    if(!(ICType == MSPG2553))
    {

         switch (channel)
         {
            case 13:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0xd000;                             // INCH set to channel 13

                break;

            case 14:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0xe000;                             // INCH set to channel 14

                 break;

            case 15:
                ADC10CTL1 &= ~0xf000;                           // Clears the previous channel setting, also set to INCH = 0
                ADC10CTL1 |= 0xf000;                             // INCH set to channel 1

                 break;

            default:
                break;
        }
    }
    ADC10CTL0 &=   ~ADC10IFG;                                   // Clears the conversion completion flag
    ADC10CTL0 |=  ENC;                                          // Enable ENC
    ADC10CTL0 |=    ADC10SC;                                     // Starts a conversion
    while(!(ADC10CTL0 & ADC10IFG));                             // wait until Conversion is complete
    ADCVal[channel] = (ADC10MEM & 0x03ff);
    ADC10CTL0 &=    ~ADC10SC;                                     // Stops a conversion
    ADC10CTL0 &=    ~ENC;                                         // Disable ENC
    ADC10CTL0 &=   ~ADC10IFG;                                   // Clears the conversion completion flag
}
