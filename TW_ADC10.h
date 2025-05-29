/*
 * ProjMastrv1.h
 *
 *  Created on: Jun 18, 2018
 *      Author: Turjasu
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef _TW_ADC10H_
#define _TW_ADC10H_

#define         MSPG2553                0x01
#define         SMCLK                   0x03
#define         MCLK                    0x02
#define         ACLK                    0x01
#define         ADC10OSC                0x0

// Define enums/macros for settings
//#define USE_ADC10_LPM                   // comment this if needed in standard mode

#define ADC10_REF_VOLTAGE_VCC_VSS     0
#define ADC10_REF_VOLTAGE_INT_1_5V    1
#define ADC10_REF_VOLTAGE_INT_2_5V    2

#define ADC10_CLOCK_SRC_ADCOSC        ADC10SSEL_0
#define ADC10_CLOCK_SRC_ACLK          ADC10SSEL_1
#define ADC10_CLOCK_SRC_MCLK          ADC10SSEL_2
#define ADC10_CLOCK_SRC_SMCLK         ADC10SSEL_3

#define ADC10_SHT_4_CYCLES            ADC10SHT_0
#define ADC10_SHT_8_CYCLES            ADC10SHT_1
#define ADC10_SHT_16_CYCLES           ADC10SHT_2
#define ADC10_SHT_64_CYCLES           ADC10SHT_3

#define ADC10_INPUT_CH_0              INCH_0
#define ADC10_INPUT_CH_1              INCH_1
#define ADC10_INPUT_CH_2              INCH_2
#define ADC10_INPUT_CH_3              INCH_3
#define ADC10_INPUT_CH_4              INCH_4
#define ADC10_INPUT_CH_5              INCH_5
#define ADC10_INPUT_CH_6              INCH_6
#define ADC10_INPUT_CH_7              INCH_7
#define ADC10_INPUT_CH_10             INCH_10
#define ADC10_INPUT_CH_11             INCH_11

#define ADC10_ENABLE_INTRPT           ADC10IE

#define ADC10_CLK_SRC_ADC10CLK        ADC10SSEL_0
#define ADC10_CLK_SRC_ACLK            ADC10SSEL_1
#define ADC10_CLK_SRC_MCLK            ADC10SSEL_2
#define ADC10_CLK_SRC_SMCLK           ADC10SSEL_3

#define ADC10_CLK_PREDIV_BY_1         ADC10DIV_0
#define ADC10_CLK_PREDIV_BY_2         ADC10DIV_1
#define ADC10_CLK_PREDIV_BY_3         ADC10DIV_2
#define ADC10_CLK_PREDIV_BY_4         ADC10DIV_3
#define ADC10_CLK_PREDIV_BY_5         ADC10DIV_4
#define ADC10_CLK_PREDIV_BY_6         ADC10DIV_5
#define ADC10_CLK_PREDIV_BY_7         ADC10DIV_6
#define ADC10_CLK_PREDIV_BY_8         ADC10DIV_7

#define ADC10_START_CONVERSION        1


struct adc10Params{
    uint16_t inputChannel;      // ADC10 channel (0 to 15)
    uint8_t refVoltage;       // Reference voltage setting
    uint8_t clockSource;      // ADC clock source
    uint8_t clockDivider;     // Clock divider
    uint16_t sampleHoldTime;   // Sample and hold time
    uint8_t enableInterrupt; // Enable ADC interrupt
    bool    startAdcNow;        // Start the ADC10 as soon as config finishes
    uint_fast8_t    convertDone;
} ;

extern          uint16_t                gAdcRawVal;
extern          void                    InitADC10(unsigned char, unsigned char, unsigned char, unsigned char);                    // initializes the ADC10 module of MSP4302553 or eqv
extern          void                    ConvertADC(int, unsigned char, unsigned short*);
extern          void                    controlConversion(uint8_t);
extern          uint8_t                 InitADC10_new(struct adc10Params* );
//extern          void (*adc10_callback)(int  gParamToBeSetThru_CB);
extern          void register_adc10_callback(void (*callback)(uint16_t,int*));





#endif /* _TW_ADC10H_ */
