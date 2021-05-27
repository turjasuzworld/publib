/*
 * TW_ESP8266.h
 *
 *  Created on: Jan 18, 2020
 *      Author: Turjasu
 */

#ifndef ESP8266_TW_TW_ESP8266_H_
#define ESP8266_TW_TW_ESP8266_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Variables
 */
extern   long long int     _bytesDataTransacted;
extern   unsigned char     _MdmBuffer[1024], _MdmStatus[80],        // Global MBuffer to store the modem replies. Define in your required C/CPP files/classes
                           _MdmHTTPBuff[1024];    //HTTP Get Buffer



extern   struct      _wifiParams     WifiParams;
/*
 *  Definitions for Port and Pins
 */
#define                 SystemFreq                      16000000.00000 // This MUST BE DEFINED PROPERLY FOR COMMUNICATION

#define                 TurnOnImmediately               1
#define                 TurnOnAfter1Sec                 2
#define                 TurnOff                         3
#define                 Restart                         4

#define                 _ESP_PORT_DIR                    P3DIR
#define                 _ESP_EN_PORT_DIR                 P7DIR
#define                 _ESP_RST_PORT_DIR                P9DIR
#define                 _ESP_PORT_OUT                    P3OUT
#define                 _ESP_EN_PORT_OUT                 P7OUT
#define                 _ESP_RST_PORT_OUT                P9OUT

#define                 _ESP_RST                         BIT6
#define                 _ESP_EN                          BIT2

// USCIA0 comm. bits
#define                 _ESP_PORT_SEL                 P3SEL
#define                 _ESP_UART_RX                  BIT5
#define                 _ESP_UART_TX                  BIT4
#define                 _IntrptBased                     1
#define                 _PollingBased                    2


/*
 * Structures
 */

struct      _wifiParams{
            char _wifiSSID[30];
            char _wifiPSWD[30];
            char _wifiIPAddr[16];
            char _wifiInetPingMs[4];
            char _wifiRssi[3];
};

/*
 *  Enums
 */
//enum ReplyCodes{
//    SUCCESS,
//    FAIL,
//    _E8266_INIT_SUCCESS,
//    _E8266_HW_FLT,
//    _E8266_HW_RETRY_TIMEOUT,
//    UNKNOWN,
//};

typedef     enum    {
        _Esp_Full_Init,
        _Esp_Connect_WIFI,
        _Esp_disConnect_WIFI,
        _Esp_GET_Request,
        _Esp_UDP_GET_Request,
        _Esp_UDP_Close_Request,
        _Esp__repeated_send_Request,
        _Esp_SET_Request,
        _Esp_POST_Request,
}espOperCommand;


typedef enum    { //POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE
                          //*   --> CHK GPRS -->EXIT
        _E8266_NON_UDP_MODE = -3,
        _E8266_MINUS2,
        _E8266_MINUS1,
        _SUCCESS,
        _FAIL,
        _E8266_INIT_SUCCESS,
        _E8266_HW_FLT,
        _E8266_HW_RETRY_TIMEOUT,
        _UNKNOWN,
        _E8266_PWR_UP,
        _E8266_PWR_RESTART,
        _E8266_PWR_RESTART_DELAY,
        _E8266_PWR_UP_SUCCESS,              //07
        _E8266_PWR_UP_FAILURE,
        _E8266_PWR_DN,
        _E8266_PWR_OFF,
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
        _E8266_PING_SUCCESS,                //33        // THIS ALSO ENSURES INTERNET IS PRESENT = PINGING WWW.GOOGLE.COM
        _E8266_PING_FAIL,                               // THIS ALSO ENSURES THAT INTERNET IS NOT PRESENT
        _E8266_PING_TIMEOUT,
        _E8266_RSSI_RCV_SUCCESS,
        _E8266_RSSI_RCV_ERROR,
        _E8266_RSSI_RCV_TIMEOUT,
        _E8266_CIPSTART_OK,                 //39
        _E8266_CIPSTART_UDP_OK,
        _E8266_CIPCLOSE_UDP,
        _E8266_CIPCLOSE_UDP_SUCCESS,
        _E8266_CIPCLOSE_UDP_FAIL,
        _E8266_CIPSTART_ERROR,
        _E8266_CIPSTART_DNS_ERROR,
        _E8266_CIPSTART_TIMEOUT,
        _E8266_CIPSTART_ALREADY_CONNCTD,
        _E8266_CIPSEND_ARROW_SUCCESS,
        _E8266_CIPSEND_ARROW_FAIL,  // ALSO INCLUDES TIMEOUT
        _E8266_CIPSEND_ARROW_TIMEOUT,
        _E8266_CIPCLOSE_FAIL,
        _E8266_CIPCLOSED_SUCCESS,
        _E8266_SEND_OK_RECVD,               //48
        _E8266_SEND_FAIL,
        _E8266_SEND_TIMEOUT,
        _E8266_MODULE_EXIT,
        _E8266_SM_CMD_ERROR,    // State MAchine Command could not be processed as the present state is wrong or not allowed
        _E8266_TEST_ENUM,

} esp8266StateMachines;


/*
 *  Source functions required for the control and communications
 *  using ESP8266 (External Wifi connectivity functions using ESP-01)
 */

extern                      esp8266StateMachines resetESP8266(void);          // generates a reset to the esp device
extern                      esp8266StateMachines moduleInitDiag(esp8266StateMachines);
extern                      uint8_t             ConfigureEspUART(float baudrate, uint8_t interrupt_polling);
extern                      void                SendDataToESP(const char* data);
extern                      void                SendCharToESP(unsigned char);

extern                      esp8266StateMachines ESP_PinSetup(void);
extern                      esp8266StateMachines ESP_ON_OFF(uint8_t);

extern                      void                DeEchoShrtRsp(void);
extern                      esp8266StateMachines    ESPInitAndDiag(void);       // Implements StateMachine for the init and diag for M95
extern                      esp8266StateMachines    MdmMakeReady(espOperCommand, esp8266StateMachines, char* , char* ,char*,char*,int,int);
extern                      void                ParseCallback(void (*ptr)());
extern                      esp8266StateMachines _ReadEspRSSI(char*, char*);
extern                      int                 ReadIPAddr(void);
extern                      void                ClrEspBuff(void);
extern                      char*               mdmReplySearch(const char*, const char*);
extern                      int                 setWifiCred(char* , char* );
extern                      void                disESP8266(void);
extern                      void                enESP8266(void);
extern                      esp8266StateMachines    _provision_Connect_GETcall_ESP(esp8266StateMachines, bool, char* , char* );


#endif /* ESP8266_TW_TW_ESP8266_H_ */
