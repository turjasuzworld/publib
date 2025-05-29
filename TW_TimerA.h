/*
 * TW_TimerA.h
 *
 *  Created on: Jun 27, 2018
 *      Author: Turjasu
 */

#ifndef TW_TIMERA_H_
#define TW_TIMERA_H_

#define         SMCLK                   0x03
#define         MCLK                    0x02
#define         ACLK                    0x01
#define         TACLK                   0x04
#define         INCLK                   0x05
#define         STOP                    0
#define         UP                      0X01
#define         CONT                    0X02
#define         UP_DOWN                 0X03
#define         CLR_TIMER               true
#define         TA_CCR0_IE              true

extern void ConfigureTimerA0(unsigned char, unsigned char, unsigned char,unsigned short, bool,bool );

#endif /* TW_TIMERA_H_ */
