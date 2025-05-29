/*
 * TW_UART.c
 *
 *  Created on: Jun 22, 2018
 *      Author: Turjasu
 */
#include <msp430.h>
#include "TW_UART.h"
#include"TW_WATCHDOG.h"
#include <stdint.h>
#include <stdbool.h>


//================ Initilization of USCIA0 into UART ===============================
void SetUSCIA0ForUART(uint32_t BaudRate)
/*This function initializes the USCI module Ax as the UART. Pass Baudrate as argument
 * Baudrate will be taken as UCOS16 = 0
 */
{
//    ClearWDT();
      UCA0CTL1 |= UCSWRST;                     // **Reset USCI state machine**
      P1SEL |= UCA0RXD + UCA0TXD ;               // P1.1 = RXD, P1.2=TXD
      P1SEL2 |= UCA0RXD + UCA0TXD ;                    // P1.1 = RXD, P1.2=TXD
      UCA0CTL1 |= UCSSEL_2;                     // SMCLK
      switch (BaudRate)                             // Default BR is 9600, if not mentioned
      {
        case 9600:
            UCA0BR0 = 104;                            // 1MHz 9600 = 104, 115200 = 8
            UCA0BR1 = 0;                              // 1MHz 9600
            UCA0MCTL |= UCBRS_1;              // cpu @ 1mhz, BAUD @ 115200
            break;
        case 115200:
            UCA0BR0 = 8;                            // 1MHz 9600 = 104, 115200 = 8
            UCA0BR1 = 0;                              // 1MHz 9600
            UCA0MCTL |= UCBRS_6;              // cpu @ 1mhz, BAUD @ 115200
            break;

        default:
            UCA0BR0 = 104;                            // 1MHz 9600 = 104, 115200 = 8
            UCA0BR1 = 0;                              // 1MHz 9600
            UCA0MCTL |= UCBRS_1;              // cpu @ 1mhz, BAUD @ 115200
            break;
    }
//      UCA0STAT |= UCLISTEN;                     // ONLY FOR TESTING NOT FOR PRODUCTION / DEV
      UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
      IE2 |= UCA0RXIE; //+ UCA0TXIE;               // Enable USCI_A0 RX & TX interrupt, but untill GIE is Set, it wont work
                                                // Note that the ISR is to be written in Main if Interrpt is used. Also bucket for
                                                // receiving the data
}

//========================Send Multiple data from a Normal Array like uchar/char/short etc thru UART===============
void    SendData(unsigned char   Data)
{
//        ClearWDT();
        while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
        UCA0TXBUF = Data;
        _delay_cycles(100000);                      // delay of 100 mS

}
//========================String comparison function================================================================
unsigned char       CompareUARTReply(unsigned char *data, unsigned char *buffer, unsigned int length)
{
    unsigned int count=0;
    unsigned char reply=0;

 //   _delay_cycles(100000);
    while((buffer[count] == data[count])&&(count <length))
    {
        //ConfigWDGT(DivClkby32768, ACLKSel, WDTMode, RSTfn, RiseEdge);//===============================================
        count++;
        reply = 0x01;
    }
    if((buffer[count] != data[count])&&(count <length))
    {
        reply = 0;
    }

    return reply;
}
//==============================Send string data ===================================
void    StringData(const unsigned char   *Data)
{
    while(*Data)
    {
        //ConfigWDGT(DivClkby32768, ACLKSel, WDTMode, RSTfn, RiseEdge);//===============================================
        while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
        UCA0TXBUF = *Data;
        _delay_cycles(2000);                      // delay of 2 mS
        Data++;
       while (UCA0STAT&UCBUSY);                // USCI_A0 BUSY?
    }
    _delay_cycles(100000);                      // delay of 100 mS
 //   ClearWDT();
}

//============================Enable Receiver Mode for any RS485 based IC===================
/*
 * Tested with SN65HVD1780...
 */
 void     RS485_Rx_Mode(void)
 {
     P2OUT  &=      ~ENRS485_RX;        // Makes the RE' Line Low for enabling the Receiving mode
     P2OUT  &=      ~ENRS485_TX;        // Makes the DE line low to disable the driver
 }

 void     RS485_Tx_Mode(void)
  {
      P2OUT  |=      ENRS485_RX;        // Makes the RE' Line high for disabling the Receiving mode
      P2OUT  |=      ENRS485_TX;        // Makes the DE line high to enable the driver
  }

 void     RS485_OFF_Mode(void)
  {
      P2OUT  |=      ENRS485_RX;        // Makes the RE' Line high for disabling the Receiving mode
      P2OUT  &=      ~ENRS485_TX;        // Makes the DE line high to enable the driver
  }
