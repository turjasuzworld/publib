/*
 * TW_MSP430G2xx_ESP.h
 *
 *  Created on: May 25, 2020
 *      Author: Turjasu
 */

#ifndef TW_MSP430G2XX_ESP_H_
#define TW_MSP430G2XX_ESP_H_
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

extern      volatile            unsigned char                                   _MdmBuffer[_bufferMax],
                                                                                _MdmStatus[80],        // Global MBuffer to store the modem replies. Define in your required C/CPP files/classes
                                                                                _MdmHTTPBuff[_bufferMax],     //HTTP Get Buffer
                                                                                _MdmIPAddr[16];
extern                          unsigned char                                   _APpassword[30],
                                                                                _APname[30];
extern     volatile             unsigned int                                    _MdmBuffCnt;

extern                          uint8_t                                         _devIP_Address[16];

extern                          const   char                                    _AT_reply[];
extern                          const   char                                    _AT_CWMODE_CUR_reply[];
extern                          const   char                                    _AT_CWJAP_CUR_reply[];
extern                          const   char                                    _AT_CIFSR_reply[];
extern                          const   char                                    _AT_PING_reply[];
extern                          const   char                                    _AT_CIPSTART_reply[];
extern                          const   char                                    _AT_CIPSEND_reply[];
extern                          const   char                                    _AT_CIPCLOSE_reply[];

/*
 *  Enums
 */
typedef     enum    {
        _Esp_Full_Init,
        _Esp_Connect_WIFI,
        _Esp_disConnect_WIFI,
        _Esp_GET_Request,
        _Esp_SET_Request,
        _Esp_POST_Request,
}espOperCommand;


typedef enum    { //POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE
                          //*   --> CHK GPRS -->EXIT
        _SUCCESS,
        _FAIL,
        _E8266_INIT_SUCCESS,
        _E8266_HW_FLT,
        _E8266_HW_RETRY_TIMEOUT,
        _UNKNOWN,
        _E8266_PWR_UP,
        _E8266_PWR_UP_SUCCESS,
        _E8266_PWR_DN,
        _E8266_UNKNOWN_FAILURE,
        _E8266_RST_SUCCESS,
        _E8266_RST_FAIL,
        _E8266_AT_RESPNSE_FAIL,
        _E8266_AT_RESPNSE_SUCCESS,
        _E8266_DeECHO_FAIL,
        _E8266_DeECHO_SUCCESS,
        _E8266_STN_MODE_FAIL,
        _E8266_STN_MODE_SUCCESS,
        _E8266_SoFTAP_MODE_FAIL,
        _E8266_SoFTAP_MODE_SUCCESS,
        _E8266_MIX_MODE_FAIL,
        _E8266_MIX_MODE_SUCCESS,
//        _E8266_SoFTAP_MODE_FAIL,
//        _E8266_SoFTAP_MODE_SUCCESS,
        _E8266_WIFI_CONCTD_SUCCESS,
        _E8266_WIFI_RCV_IP_SUCCESS,
        _E8266_SERVR_CONNECT_SUCCESS,
        _E8266_SERVR_CONNECT_TIMEOUT,
        _E8266_SERVR_DISCONNCT,
        _E8266_CWJAP_UNKNWN_FAIL,
        _E8266_CWJAP_CON_TIMOUT_1,
        _E8266_CWJAP_WRNG_PSWRD_2,
        _E8266_CWJAP_AP_NT_FND_3,
        _E8266_CWJAP_CON_GENERAL_FAIL_4,
        _E8266_CIFSR_FAIL,
        _E8266_PING_SUCCESS,    // THIS ALSO ENSURES INTERNET IS PRESENT = PINGING WWW.GOOGLE.COM
        _E8266_PING_FAIL,       // THIS ALSO ENSURES THAT INTERNET IS NOT PRESENT
        _E8266_PING_TIMEOUT,
        _E8266_RSSI_RCV_SUCCESS,
        _E8266_RSSI_RCV_ERROR,
        _E8266_RSSI_RCV_TIMEOUT,
        _E8266_CIPSTART_OK,
        _E8266_CIPSTART_ERROR,
        _E8266_CIPSTART_TIMEOUT,
        _E8266_CIPSTART_ALREADY_CONNCTD,
        _E8266_CIPSEND_ARROW_SUCCESS,
        _E8266_CIPSEND_ARROW_FAIL,  // ALSO INCLUDES TIMEOUT
        _E8266_CIPSEND_ARROW_TIMEOUT,
        _E8266_CIPCLOSE_FAIL,
        _E8266_CIPCLOSED_SUCCESS,
        _E8266_SEND_OK_RECVD,
        _E8266_SEND_FAIL,
        _E8266_SEND_TIMEOUT,
        _E8266_MODULE_EXIT,
        _E8266_SM_CMD_ERROR,    // State MAchine Command could not be processed as the present state is wrong or not allowed
        _E8266_TEST_ENUM,

} esp8266StateMachines;


/*
 *  Source functions required for the control and communications
 *  using ESP8266
 */

extern                      esp8266StateMachines resetESP8266(void);          // generates a reset to the esp device
extern                      esp8266StateMachines moduleInitDiag(esp8266StateMachines);
extern                      uint8_t             ConfigureEspUART(long unsigned int baudrate, uint8_t interrupt_polling);
extern                      void                SendDataToESP(const uint8_t* data);
extern                      void                SendCharToESP(unsigned char);

extern                      esp8266StateMachines ESP_PinSetup(void);
extern                      esp8266StateMachines ESP_ON_OFF(uint8_t);

extern                      void                DeEchoShrtRsp(void);
extern                      esp8266StateMachines    ESPInitAndDiag(void);       // Implements StateMachine for the init and diag for M95
extern                      esp8266StateMachines    MdmMakeReady(espOperCommand , esp8266StateMachines, char*, char*);
extern                      void                ParseCallback(void (*ptr)());
extern                      esp8266StateMachines ReadEspRSSI(unsigned char*);
extern                      int                 ReadIPAddr(void);
extern                      void                ClrEspBuff(void);
extern                      esp8266StateMachines espConnectWiFi(void);
extern                      esp8266StateMachines espReadFromServer(esp8266StateMachines);


#endif /* TW_MSP430G2XX_ESP_H_ */
