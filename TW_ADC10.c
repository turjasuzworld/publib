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


            uint16_t                gAdcRawVal = 0;
volatile    int                     gComputedLux = 0;
volatile    uint_fast8_t*            adcIntrptSts = 0;

void (*adc10_callback)(uint16_t adc10RawVal, int* pComputedLuxVar) = 0;

//Function to register the callback:
void register_adc10_callback(void (*callback)(uint16_t,int*)) {
    adc10_callback = callback;
}


uint8_t InitADC10_new(struct adc10Params* gAdc10Params)
{

    switch (gAdc10Params->refVoltage) {
        case ADC10_REF_VOLTAGE_INT_1_5V:
            ADC10CTL0 = SREF_1 + REFON;
            break;
        case ADC10_REF_VOLTAGE_INT_2_5V:
            ADC10CTL0 = SREF_1 + REFON + REF2_5V;
            break;
        default:
            ADC10CTL0 = SREF_0;
            ADC10CTL0 &= ~(REFON + REF2_5V);
            break;
    }
    switch (gAdc10Params->sampleHoldTime) {
        case ADC10_SHT_4_CYCLES:
            ADC10CTL0 |= ADC10_SHT_4_CYCLES;
            break;
        case ADC10_SHT_8_CYCLES:
            ADC10CTL0 |= ADC10_SHT_8_CYCLES;
            break;
        case ADC10_SHT_16_CYCLES:
            ADC10CTL0 |= ADC10_SHT_16_CYCLES;
            break;
        case ADC10_SHT_64_CYCLES:
            ADC10CTL0 |= ADC10_SHT_64_CYCLES;
            break;
        default:
            ADC10CTL0 |= ADC10_SHT_64_CYCLES; // if nothing is mentioned or wrong value
            break;
    }
    switch (gAdc10Params->enableInterrupt) {
        case ADC10_ENABLE_INTRPT:
            ADC10CTL0 |= ADC10_ENABLE_INTRPT;
            break;
        default:
            ADC10CTL0 &= ~ADC10_ENABLE_INTRPT;
            break;
    }
    //Configure channel
    switch (gAdc10Params->inputChannel) {
        case ADC10_INPUT_CH_0:
            ADC10CTL1 = ADC10_INPUT_CH_0;                       // input A0
            ADC10AE0 |= BIT0;                         // PA.0 ADC option select
            break;
        case ADC10_INPUT_CH_1:
            ADC10CTL1 = ADC10_INPUT_CH_1;                       // input A1
            ADC10AE0 |= BIT1;                         // PA.1 ADC option select
            break;
        case ADC10_INPUT_CH_2:
            ADC10CTL1 = ADC10_INPUT_CH_2;                       // input A2
            ADC10AE0 |= BIT2;                         // PA.2 ADC option select
            break;
        case ADC10_INPUT_CH_3:
            ADC10CTL1 = ADC10_INPUT_CH_3;                       // input A3
            ADC10AE0 |= BIT3;                         // PA.3 ADC option select
            break;
        case ADC10_INPUT_CH_4:
            ADC10CTL1 = ADC10_INPUT_CH_4;                       // input A4
            ADC10AE0 |= BIT4;                         // PA.4 ADC option select
            break;
        case ADC10_INPUT_CH_5:
            ADC10CTL1 = ADC10_INPUT_CH_5;                       // input A5
            ADC10AE0 |= BIT5;                         // PA.5 ADC option select
            break;
        case ADC10_INPUT_CH_6:
            ADC10CTL1 = ADC10_INPUT_CH_6;                       // input A6
            ADC10AE0 |= BIT6;                         // PA.6 ADC option select
            break;
        case ADC10_INPUT_CH_7:
            ADC10CTL1 = ADC10_INPUT_CH_7;                       // input A7
            ADC10AE0 |= BIT7;                         // PA.5 ADC option select
            break;
        case ADC10_INPUT_CH_10:
            ADC10CTL1 = ADC10_INPUT_CH_10;                       // input A10
            break;
        case ADC10_INPUT_CH_11:
            ADC10CTL1 = ADC10_INPUT_CH_11;                       // input A11
            break;
        default:
            break;
    }
    // Configure Clock
    switch (gAdc10Params->clockSource) {
        case ADC10_CLK_SRC_ADC10CLK:
            ADC10CTL1 |= ADC10_CLK_SRC_ADC10CLK;
            break;
        case ADC10_CLK_SRC_ACLK:
            ADC10CTL1 |= ADC10_CLK_SRC_ACLK;
            break;
        case ADC10_CLK_SRC_MCLK:
            ADC10CTL1 |= ADC10_CLK_SRC_MCLK;
            break;
        case ADC10_CLK_SRC_SMCLK:
            ADC10CTL1 |= ADC10_CLK_SRC_SMCLK;
            break;
        default:
            ADC10CTL1 |= ADC10_CLK_SRC_SMCLK; //Defaults to SMCLK
            break;
    }

    // Configure clock pre-div
    switch (gAdc10Params->clockDivider) {
        case ADC10_CLK_PREDIV_BY_1:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_1;
            break;
        case ADC10_CLK_PREDIV_BY_2:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_2;
            break;
        case ADC10_CLK_PREDIV_BY_3:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_3;
            break;
        case ADC10_CLK_PREDIV_BY_4:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_4;
            break;
        case ADC10_CLK_PREDIV_BY_5:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_5;
            break;
        case ADC10_CLK_PREDIV_BY_6:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_6;
            break;
        case ADC10_CLK_PREDIV_BY_7:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_7;
            break;
        case ADC10_CLK_PREDIV_BY_8:
            ADC10CTL1 |= ADC10_CLK_PREDIV_BY_8;
            break;
        default:
            break;
    }
    ADC10CTL0 |= ADC10ON ;
    adcIntrptSts = &gAdc10Params->convertDone;
}


void                    controlConversion(uint8_t state)
{
    if(ADC10_START_CONVERSION == state) {
        *adcIntrptSts = 1; // ensures the gFlag indicating the ADC conversion is done is reset.1= conversion in progress, 2 = done
#ifdef USE_ADC10_LPM
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
        __bis_SR_register(LPM0_bits + GIE);        // LPM0, ADC10_ISR will force exi
#else
        while(ADC10CTL1 & ADC10BUSY); // Blocking until Conversion is done
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
//        while(ADC10CTL1 & ADC10BUSY); // Blocking until Conversion is done
//        gAdcRawVal = ADC10MEM;
        __bis_SR_register(GIE);
#endif

    }
}

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


//// ADC10 interrupt service routine
//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=ADC10_VECTOR
//__interrupt void ADC10_ISR (void)
//#elif defined(__GNUC__)
//void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
//#else
//#error Compiler not supported!
//#endif
//{
//  //__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
//#ifdef USE_ADC10_LPM
//    __bic_SR_register_on_exit(LPM0_bits);     // exit power save mode
//#endif
//    gAdcRawVal = ADC10MEM;
//}

// ADC10 interrupt service routine (modified with CB):
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
#ifdef USE_ADC10_LPM
    __bic_SR_register_on_exit(LPM0_bits);     // exit power save mode
#endif
    gAdcRawVal = ADC10MEM;
    if (adc10_callback) {
        //adc10_callback(gAdcRawVal,pComputedLuxVar); // call and ignore return value
        adc10_callback(gAdcRawVal, (int*)&gComputedLux);
        *adcIntrptSts = 2; // sets global flag as 2= done
        __no_operation();
    }
}

