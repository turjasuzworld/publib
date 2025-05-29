/*
 * TW_M430G2xx_TimerAx.h
 *
 *  Created on: May 24, 2020
 *      Author: Turjasu
 */

#ifndef TW_M430G2XX_TIMERAX_H_
#define TW_M430G2XX_TIMERAX_H_
#include <msp430.h>
#include <stdbool.h>



#define                     _clockPulseTimeWidth_uS                                     1

#define                     _TimerValReqd                                               50000 // <== Change here, value should be calculated from nearest to 1 sec wrt system clock

#define                     _delayForTimerA0_Overflow_mS                                (_TimerValReqd/1000)

#define                     _1Sec                                                       (1000/_delayForTimerA0_Overflow_mS)       // 1000mS = 1 sec, 50 mS is TA0 overflow timing

#define                     _10Seconds                                                  (2*_1Sec) // 1 Mhz -> 1uS pulse -> 50000 Timer counts will take 50 mS to over flow. So for 1 sec -> 20 overflows

#define                     _interruptMode                                              1
#define                     _pollingMode                                                0

extern unsigned int TimerVal, delayNeededInSec;
extern  unsigned char  _deviceStatus;          // define in the main routine

extern void configTA0(unsigned int, unsigned char);
extern void B(void (*ptr)());



#endif /* TW_M430G2XX_TIMERAX_H_ */
