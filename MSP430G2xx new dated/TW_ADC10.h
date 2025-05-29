/*
 * ProjMastrv1.h
 *
 *  Created on: Jun 18, 2018
 *      Author: Turjasu
 */

#include <msp430.h>

#ifndef _TW_ADC10H_
#define _TW_ADC10H_

#define         MSPG2553                0x01
#define         SMCLK                   0x03
#define         MCLK                    0x02
#define         ACLK                    0x01
#define         ADC10OSC                0x0


extern          void                    InitADC10(unsigned char, unsigned char, unsigned char, unsigned char);                    // initializes the ADC10 module of MSP4302553 or eqv
extern          void                    ConvertADC(int, unsigned char, unsigned short*);





#endif /* _TW_ADC10H_ */
