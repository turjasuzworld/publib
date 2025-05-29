/*
 * TW_UART.h
 *
 *  Created on: Jun 22, 2018
 *  Modified: Oct21, 2018: Added baudrate as argument
 *      Author: Turjasu
 */

#ifndef TW_UART_H_
#define TW_UART_H_

#include <stdint.h>

#define     UCA0RXD             BIT1
#define     UCA0TXD             BIT2
#define     ENRS485_TX          BIT1
#define     ENRS485_RX          BIT2


extern      void SetUSCIA0ForUART(uint32_t BaudRate);                              // Assuming 1 MHz clocking by DCO and SMCLK clocks USCI
extern      void    StringData(const unsigned char   *);
extern      void    SendData(unsigned char);
extern      void     RS485_Rx_Mode(void);
extern      void     RS485_Tx_Mode(void);
extern      void     RS485_OFF_Mode(void);
extern      unsigned char       CompareUARTReply(unsigned char *, unsigned char *, unsigned int );

#endif /* TW_UART_H_ */
