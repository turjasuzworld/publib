/*
 * TW_TimerA.h
 *
 *  Created on: Jun 27, 2018
 *      Author: Turjasu
 */

#ifndef TW_TIMERA_V2_H_
#define TW_TIMERA_V2_H_

#define         MCLK                    0x01
#define         SMCLK                   0x02
#define         ACLK                    0x03
#define         TACLK                   0x04
#define         INCLK                   0x05
#define         STOP                    0
#define         UP                      1
#define         CONT                    2
#define         UP_DOWN                 3
#define         CLR_TIMER               true
#define         TA_CCR0_IE              true

extern void _ElapseTimeCCR0_A0(unsigned char , unsigned char , unsigned char , uint16_t,  uint16_t );

#endif /* TW_TIMERA_V2_H_ */
