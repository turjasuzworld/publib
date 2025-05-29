/*
 * TW_WATCHDOG.h
 *
 *  Created on: Jul 24, 2018
 *      Author: Turjasu
 */

#ifndef TW_WATCHDOG_H_
#define TW_WATCHDOG_H_
///////////// Clock division value set /////////////////
#define             DivClkby32768       0x00
#define             DivClkby8192         0x01
#define             DivClkby512            0x10
#define             DivClkby64              0x11
////////////Select Clock////////////////////////////////
#define             SMCLKSel                0
#define             ACLKSel                   1
////////////// WDT Mode ////////////////////////////
#define             WDTMode               0
#define             IntervalMode          1
///////////////RST NMI function /////////////////////
#define             RSTfn                         0
#define             NMIfn                       1
//////////////NMI Inttrpt sense type /////////////////
#define             RiseEdge                   0x0
#define             FallEdge                   1

extern          void                         ConfigWDGT(unsigned char , unsigned char , unsigned char , unsigned char , unsigned char ); //
extern          void                         ClearWDT(void);
extern          unsigned char        CheckWDTFlg(void); // checks if the previous reset was due to WDT
extern          void                         RestartWDT(void);

#endif /* TW_WATCHDOG_H_ */
