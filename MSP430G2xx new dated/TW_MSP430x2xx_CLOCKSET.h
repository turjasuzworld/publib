/*
 * TW_MSP430x2xx_CLOCKSET.h
 *
 *  Created on: Jun 15, 2018
 *      Author: Turjasu
 */

#ifndef TW_MSP430X2XX_CLOCKSET_H_
#define TW_MSP430X2XX_CLOCKSET_H_

extern int SetHFMode(int);
extern void SetPresclrAMSMClk(unsigned char,unsigned char,unsigned char);
extern void    SetnStartVLOCLK(void);
extern void  SetDCOClk(unsigned char);
extern void SetSMCLKSrc(unsigned char); // 0 = DCO,1 = xt2(if present) or lfxt1/vloclk

#endif /* TW_MSP430X2XX_CLOCKSET_H_ */
