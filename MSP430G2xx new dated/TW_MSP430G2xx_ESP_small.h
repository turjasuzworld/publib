/*
 * TW_MSP430G2xx_ESP_small.h
 *
 *  Created on: May 27, 2020
 *      Author: Turjasu
 */

#ifndef TW_MSP430G2XX_ESP_SMALL_H_
#define TW_MSP430G2XX_ESP_SMALL_H_

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "TW_CapaJoystick.h"



//extern   volatile       unsigned char                                           _MdmAPN[];           // APN name
/*
 *  Definitions for Port and Pins
 */
//#define                 SystemFreq                      1000000.00000 // This MUST BE DEFINED PROPERLY FOR COMMUNICATION

#define                 TurnOnImmediately               1
#define                 TurnOnAfter1Sec                 2
#define                 TurnOff                         3
#define                 Restart                         4

#define                 _ESP_PORT_DIR                    P1DIR
#define                 _ESP_RST_PORT_DIR                P2DIR
#define                 _ESP_PORT_OUT                    P1OUT
#define                 _ESP_RST_PORT_OUT                P2OUT
#define                 _ESP_RST                         BIT4
#define                 WIFI_CONNECTED_LED               BIT6   // P2.6
// USCIA0 comm. bits
#define                 _ESP_PORT_SEL                 P1SEL
#define                 _ESP_PORT_SEL2                 P1SEL2
#define                 _ESP_UART_RX                  BIT1      // RX - MCU side
#define                 _ESP_UART_TX                  BIT2
#define                 _IntrptBased                     1
#define                 _PollingBased                    2

#define                 _bufferMax                       130

#define                 _espRxDataParsingReqd            false

/*
 * Variables
 */

extern      volatile            unsigned char                                   _MdmBuffer[_bufferMax];
                                                                                //_MdmIPAddr[16];
extern                          unsigned char                                   _APpassword[30],
                                                                                _APname[30];
extern     volatile             unsigned int                                    _MdmBuffCnt;


extern                          const   char                                    _AT_reply[];
extern                          const   char                                    _AT_CWMODE_CUR_reply[];
extern                          const   char                                    _AT_CWJAP_CUR_reply[];
extern                          const   char                                    _AT_CIFSR_reply[];
extern                          const   char                                    _AT_PING_reply[];
extern                          const   char                                    _AT_CIPSTART_reply[];
extern                          const   char                                    _AT_CIPSEND_reply[];
extern                          const   char                                    _AT_CIPCLOSE_reply[];




/*
 *  Source functions required for the control and communications
 *  using ESP8266
 */

extern                      uint8_t             ConfigureEspUART(long unsigned int baudrate, uint8_t interrupt_polling);
extern                      void                SendDataToESP(const uint8_t* data);
extern                      void                SendCharToESP(unsigned char);
extern                      char*             checkReply(const char*, const char*);


#endif /* TW_MSP430G2XX_ESP_SMALL_H_ */
