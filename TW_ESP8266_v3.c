/*
 * TW_ESP8266.c
 *
 *  Created on: Jan 18, 2020
 *      Author: Turjasu
 *
 *      Uses USCI A0
 */

#include "TW_ESP8266_v3.h"
#include "TW_GenericUART_UCA3.h"
#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// Project specific includes//
//#include "BranonSpecific/BranonSpecific_v4.h"
//#include "BranonSpecific/HomeScreen.h"

/*
 *  Variables
 */

                                  unsigned char           _MdmBuffer[1024],
                                                          _MdmStatus[80],        // Global MBuffer to store the modem replies. Define in your required C/CPP files/classes
                                                          _MdmHTTPBuff[1024],
                                                          _MdmIPAddr[16] = {NULL};
                                  unsigned char           _APpassword[30]="TM#4C1294NCPDTI3",
                                                          _APname[30]="TurjasuzWorldAP8";//INTIALIZED VALUE, CHECK FOR THIS
volatile      static              int                     _MdmBuffCnt = 0;
                                  char                    *temp_mdm = NULL;

              long long int                              _bytesDataTransacted=0;


/*
* SET & GET API DECLARATIONS : : CHANGES ACCORDING TO THE PROJECT !!!
*/
const  char              _MdmGET_API_url[] = "GET http://www.turjasuzworld.online/api/srv2.php?dvid=D1257 HTTP/1.1\r\nHost: Turjasu\r\nConnection: close\r\n\r\n";
const  char              _MdmGETurl[] = "GET http://www.turjasuzworld.in/demo/api/getdevicewifi.php?did=TD1002 HTTP/1.1\r\nHost: Turjasu\r\nConnection: close\r\n\r\n";
char              _MdmSETurl[] = "GET http://www.turjasuzworld.in/demo/api/setdevicetemp.php?did=TD1002-1&temp=999.99&day=19&mon=07&year=2020&hh=15&mm=52&ss=47 HTTP/1.1\r\nHost: Turjasu\r\nConnection: close\r\n\r\n";;
char              _MdmConstructedSetUrl[];
const  char              _MdmSET_DebugUrl[] = "";
const  char              _amp[]="&";
const  char              _MdmSETurl_URI[] = "GET http://www.turjasuzworld.online/api/srv.php?";
const  char              _MdmSETurl_DVID[] = "did=TD1004-1";//D1257&

const  char              _MdmSETurl_HTTP_CONSTRUCT[] = " HTTP/1.1\r\nHost: Turjasu\r\nConnection: close\r\n\r\n";
const  char              _serverURL[] = "turjasuzworld.in";



struct      _wifiParams     WifiParams;



/*
 * Wrapper to set the ssid and password
 * Max. no of characters = 30
 */
int                setWifiCred(char* ssid, char* password)
{
    static int idx = 0;
    if((strlen(ssid) > 30)||(strlen(password) > 30)) return -1;

    if((*ssid != NULL)&&(*password != NULL))
    {
        for (idx = 0; idx < 30; idx++)
        {
            WifiParams._wifiSSID[idx] = *ssid;
            ssid++;
        }

        for (idx = 0; idx < 30; idx++)
        {
            WifiParams._wifiPSWD[idx] = *password;
            password++;
        }


    }
    else    return -2;

    return 0;
}





/* Configure the ports ONLY FOR THE USCIA0
 * P3.4 = USCIA0TXD
 * P3.5 = USCIA0RXD
 * IF THE BOARD USES OTHER USCIAx MODULE, PLEASE MODIFY THE
 * INSTRUCTIONS ACCORDINGLY
 *
 * TW PRODUCTS WILL ALWAYS USE USCIA0 = P3.4 & 3.5 FOR MODEM
 * COMMUNICATIONS
 *
 */
esp8266StateMachines ESP_PinSetup(void)
{
    esp8266StateMachines esp8266StateMachines;
    _ESP_PORT_DIR |= _ESP_RST;
    _ESP_PORT_OUT &= ~_ESP_RST; // keep ESP in Reset state
    esp8266StateMachines = _E8266_PWR_UP;
    return esp8266StateMachines;
}

/*
 * Turn the modem on/off control
 * mode = 1 = turn on immediately
 * mode = 2 = turn on after 1 second delay
 * mode = 3 = turn off
 * mode = 4 = restart (requires 2 second delay)
 *
 * returns a 0 if everything is done properly
 */

esp8266StateMachines ESP_ON_OFF(uint8_t mode)
{
        esp8266StateMachines status = _UNKNOWN;
        if (_E8266_PWR_UP == ESP_PinSetup()) // Configures the switch on/off pins accordingly
        {
            switch (mode)
            {
                case 1:// TURN ON IMMEDIATELY
                    _ESP_RST_PORT_OUT |=  _ESP_RST;
                    status = _E8266_PWR_UP;
                    break;
                case 2:// TURN ON AFTER 1 SEC DELAY
                    _delay_cycles(SystemFreq);  // 1 sec delay
                    _ESP_RST_PORT_OUT |=  _ESP_RST;
                    status = _E8266_PWR_UP;
                    break;
                case 3:// TURN OFF IMMEDIATELY
                    _ESP_RST_PORT_OUT &= ~ _ESP_RST;
                    status = _E8266_PWR_DN;
                    break;
                case 4:// RESTART
                    _ESP_RST_PORT_OUT &= ~ _ESP_RST;
                    _delay_cycles(SystemFreq); // this will be timer based later
                    _ESP_RST_PORT_OUT |=   _ESP_RST;
                    _delay_cycles(SystemFreq*2); // this will be timer based later
                    status = _E8266_RST_SUCCESS;
                    break;
                default:
                    status = _FAIL;
                    break;
            }
        }
        else
        {
                // no handling for unknown state
        }



        return  status;
}

///*
// *  Functions to control ESP8266
// */
//esp8266StateMachines resetESP8266(void)
//{
//    P9OUT   |=  ESP_RST;
//    _delay_cycles(SystemFreq);
//    P9OUT   &= ~ESP_RST;
//    _delay_cycles(SystemFreq/2);
//    return _E8266_RESET_DONE;
//}

esp8266StateMachines moduleInitDiag(esp8266StateMachines currState)
{
    esp8266StateMachines state = currState;

    return state;

}

/*
 * baudrate will be 9600 to any suitable value.
 * Oversampling is off. please read datasheet before pushing values
 */
uint8_t ConfigureEspUART(float baudrate, uint8_t interrupt_polling)
{
    volatile float    val, val3;
    uint16_t  val2;
    volatile int val4 = 0;
     _ESP_PORT_SEL |= (_ESP_UART_RX + _ESP_UART_TX);                             // P3.4,5 = USCI_A0 TXD/RXD
     UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
     UCA0CTL1 |= UCSSEL_2;                     // SMCLK
     // Calculation of the baudrate values
     val = (SystemFreq/baudrate);
     val2 = (uint16_t)(val);
     if(val2 < 16)
     {

                 UCA0BR0 = val2 % 256;                              // (see User's Guide)
                 UCA0BR1 = val2 / 256;                              //
                 UCA0MCTL |= UCBRS_7;
                 val3 = (val - val2)*8;
                 val4 = (int)(val3);
                 switch (val4)
                 {
                    case 0:
                        UCA0MCTL |= UCBRS_0 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 1:
                        UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 2:
                        UCA0MCTL |= UCBRS_2 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 3:
                        UCA0MCTL |= UCBRS_3 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 4:
                        UCA0MCTL |= UCBRS_4 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 5:
                        UCA0MCTL |= UCBRS_5 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 6:
                        UCA0MCTL |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    case 7:
                        UCA0MCTL |= UCBRS_7 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
                        break;
                    default:
                        break;
                }
     }
     else
     {
         UCA0MCTL |= UCOS16;
         UCA0BR0 = ((uint16_t)(val/16))%256;
         UCA0BR1 = ((uint16_t)(val/16))/256;
         val3 = (val/16 - (uint16_t)(val/16))*16;
         val4 = (uint16_t)(val3);

         switch (val4)
         {
            case 1:
                UCA0MCTL |= UCBRF_1;
                break;
            case 2:
                UCA0MCTL |= UCBRF_2;
                break;
            case 3:
                UCA0MCTL |= UCBRF_3;
                break;
            case 4:
                UCA0MCTL |= UCBRF_4;
                break;
            case 5:
                UCA0MCTL |= UCBRF_5;
                break;
            case 6:
                UCA0MCTL |= UCBRF_6;
                break;
            case 7:
                UCA0MCTL |= UCBRF_7;
                break;
            case 8:
                UCA0MCTL |= UCBRF_8;
                break;
            case 9:
                UCA0MCTL |= UCBRF_9;
                break;
            case 10:
                UCA0MCTL |= UCBRF_10;
                break;
            case 11:
                UCA0MCTL |= UCBRF_11;
                break;
            case 12:
                UCA0MCTL |= UCBRF_12;
                break;
            case 13:
                UCA0MCTL |= UCBRF_13;
                break;
            case 14:
                UCA0MCTL |= UCBRF_14;
                break;
            case 15:
                UCA0MCTL |= UCBRF_15;
                break;
            default:
                break;
        }


     }


     UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     switch (interrupt_polling)
     {
        case 1:
            UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
            __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
            break;
        case 2:
             UCA0IE &= ~(UCRXIE + UCTXIE);                         // Enable USCI_A0 RX interrupt
             break;
        default:
            break;
    }

     return 0;
}


/*
 *  Reset the buffer counter
 */
void    ClrEspBuff(void)
{
    if(_MdmBuffCnt>0)
        {
            unsigned int var;
            for (var = 0; var < _MdmBuffCnt; ++var)
            {
                _MdmBuffer[var]='\0';
            }
            _MdmBuffCnt = 0;
        }
}


/*
 *  Send String of data to modem by calling this Fn.
 */
void     SendDataToESP(const char* data)
{
//    uint8_t indx=0;

        ClrEspBuff();
        while(*data)
        {
            while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
            UCA0TXBUF = *data;                  // TX -> RXed character
                SendCharToUCA3(*data);
            data++;
        }



}

/*
 *  Send single character data to modem
 */
void      SendCharToESP(unsigned char data)
{
        while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
        UCA0TXBUF = data;                  // TX -> RXed character
            SendCharToUCA3(data);
        _delay_cycles(SystemFreq/100);

}

/*
 *  Disable the Long Response and Echo,
 *  Condition is saved in _MdmInitSts
 */
void    DeEchoShrtRsp(void)
{

}


/*
 *  Initializes the Modem and checks the status of init. Also
 *  Checks for the diagnostics and saves required values
 *  to an Array through a return Pointer to an Array
 *  OutPut Array Format = {
 *  ICCID[20] //+CCID: “898600220909A0206023898600220909A0206023”
 *  COPS[] //+COPS: 0,0,"Vodafone - Delhi"
 *  QINISTAT[1] //+QINISTAT: 3
 *  QLTS[]      //+QLTS: "19/07/06,17:26:27+22,0" --> ONLY ONCE ALLOWED IMMEDIATELY AFTER POWER UP & NEEDS AT+CTZU=3
 *  CREG[]      //+CREG: 0,5
 *  CSQ[]       //+CSQ: 31,0
 *  CBC[]       //+CBC: 0,82,3955
 *
 *  _MdmStatus[0-19:ICCID][20-22:0,2/0,5 CREG][OPERATER]
 *
 *  ///////////////////////
 *  STATE MACHINES
 *  //////////////////////
 *   POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE-->EXIT
 *   /////////////////////
 *   example:
 *   _MdmStatus[70]=[iccid 0-19][;][netwrg status 20-22][;][operator name 23-?][;][RSSI ? +2][;][network time GMT + time zone ?+21][;][cbc mode ? +7][;]
 *   _MdmStatus[70]=[12231092012343127901][;][0,5][;][Vodafone - Delhi][;][31][;][19/07/13,08:09:53,+22][;][,03,3960]
 */

esp8266StateMachines    _ReadEspRSSI(char* apName, char* rssiHolder)
{
    esp8266StateMachines RetSts;
    volatile char* res = NULL;
    ///////////// Network Strength Fetch ////////////
    static int RetryTimeout = 2;
    do
    {
        SendDataToESP("AT+CWLAP\r\n");

        _delay_cycles(SystemFreq);
        res = NULL;

        res = mdmReplySearch((const char *)_MdmBuffer, "OK");
        if(res == "Timeout") return _E8266_RSSI_RCV_TIMEOUT;

        if(*res == 'O')
        {
            RetSts = _E8266_RSSI_RCV_SUCCESS;
        }

        RetryTimeout--;

    } while((RetSts != _E8266_RSSI_RCV_SUCCESS)&&(RetryTimeout>0));

    if(RetSts == _E8266_RSSI_RCV_SUCCESS)
    {
        temp_mdm = strstr((const char*)_MdmBuffer, (const char*)apName);
        temp_mdm+= strlen((const char*)apName);
        temp_mdm += 2;
        rssiHolder[0] = 0; // completely clear previous values to null
        rssiHolder[1] = 0;
        rssiHolder[2] = 0;
        int Charcount= 0; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
        while(*temp_mdm != ',')
        {

            rssiHolder[Charcount]= *temp_mdm;
            temp_mdm++;
            Charcount++;
        }

    }

    return RetSts;
}


/*
 *  Get the IP Address of the instance
 */

int            ReadIPAddr(void)
{
    // useless Fxn
    return 0;
}

/*
 *  Disable the ESP8266 Module
 */
void            disESP8266(void)
{
    _ESP_EN_PORT_OUT &= ~_ESP_EN;
}

/*
 *  Enable the ESP8266 Module
 */
void            enESP8266(void)
{
    _ESP_EN_PORT_OUT |= _ESP_EN;
}



esp8266StateMachines    MdmMakeReady(espOperCommand OperCmd, esp8266StateMachines currState, char* _getUrlPtr, char* _servUrlPtr, char* _udpServUrlPtr,char* _setUrlPtr,int _udpServPort, int _udpDataLen) // currState will be verified everytime
{
    /*                  === GET EXAMPLE =====
     *

     */
    volatile                     esp8266StateMachines RetSts =  currState; //_E8266_PWR_UP;
    static volatile int          RetryTimeout1=0, RetryTimeout2=0,RetryTimeout3=0,RetryTimeout4=0,RetryTimeout5=0;
    volatile int Charcount=0, res_type = 0;
    static int apiLength=0, apnValidation=0, temp_count=0, waitCount = 0;
    volatile char* res = NULL;
    static  unsigned int   ctr=0, udpDataCtr = 0;
///////////// Initialization Switch Cases////////////////////
    switch (OperCmd)
    {
            case _Esp_Full_Init:
                SendDataToUCA3("__case _Esp_Full_Init in MdmMakeReady__\r");
                _ESP_EN_PORT_DIR |= _ESP_EN;
                _ESP_RST_PORT_DIR |= _ESP_RST;

                if((RetSts == _E8266_PWR_UP)||(RetSts == _E8266_PWR_RESTART))
                {
                    _ESP_RST_PORT_OUT &= ~_ESP_RST;
                    _delay_cycles(SystemFreq);
                    _ESP_RST_PORT_OUT |= _ESP_RST;
                    _delay_cycles(SystemFreq*5);

                    SendDataToESP("AT\r\n");
                    if (mdmReplySearch((const char*)_MdmBuffer, "OK")=="Timeout") RetSts = _E8266_PWR_UP_FAILURE;
                    else RetSts = _E8266_PWR_UP_SUCCESS;
                }
                else if(RetSts == _E8266_PWR_DN)
                {
                    _ESP_RST_PORT_OUT &= ~_ESP_RST;
                    SendDataToESP("AT\r\n");
                    if (mdmReplySearch((const char*)_MdmBuffer, "OK")=="Timeout") RetSts = _E8266_PWR_OFF;
                    else RetSts = _E8266_UNKNOWN_FAILURE;
                }
                else if(RetSts == _E8266_PWR_RESTART_DELAY)
                {
                    _ESP_RST_PORT_OUT &= ~_ESP_RST;
                    _delay_cycles(SystemFreq*5);
                    _ESP_RST_PORT_OUT |= _ESP_RST;
                    _delay_cycles(SystemFreq*5);

                    SendDataToESP("AT\r\n");
                    if (mdmReplySearch((const char*)_MdmBuffer, "OK")=="Timeout") RetSts = _E8266_PWR_UP_FAILURE;
                    else RetSts = _E8266_PWR_UP_SUCCESS;
                }
                else
                {

                }

                break;


        case _Esp_Connect_WIFI:
             RetSts = currState;
             if((RetSts == _E8266_PWR_UP_SUCCESS)||(RetSts == _E8266_RST_SUCCESS)||(RetSts == _E8266_DeECHO_SUCCESS))
             {
                 ///////////// send CWMODE = 1  _E8266_STN_MODE_FAIL /  _E8266_STN_MODE_SUCCESS, ////////////
                 RetryTimeout1 = 5;
                 do
                 {
                     SendDataToESP("AT+CWMODE_CUR=1\r\n"); // station mode set
                     //_delay_cycles(SystemFreq*3);
                     res = NULL;
                     do
                     {
                         res = strstr((const char *)_MdmBuffer,"AT+CWMODE_CUR=1"); //===> error detected here. cannot compare like this as the
                                                                                     //buffer is filled with other value that remoes the condition of res==NULL
                         _delay_cycles(SystemFreq/5);
                         //res = strstr((const char *)_MdmBuffer,"FAIL");
                     } while (res == NULL); // Blocking Call
                     res += 20;
                     if(*res=='O')
                     {
                         res_type = 0; // 0 = success
                         RetSts = _E8266_STN_MODE_SUCCESS;
                     }
                     else if(*res=='F')
                     {
                         res_type = -1; // -1 = fail
                         RetSts = _E8266_STN_MODE_FAIL;
                     }

                     RetryTimeout1--;

                 } while((RetSts != _E8266_STN_MODE_SUCCESS)&&(RetryTimeout1>0));

                 if (RetryTimeout1 == 0)
                  {
                     RetSts = _E8266_STN_MODE_FAIL;
                  }

                 /////////// Next connect to WIFI Network (this should be fetched from LCD///////////
                 if(RetSts == _E8266_STN_MODE_SUCCESS)
                              {

                                  RetryTimeout1 = 3;
                                  do
                                  {
                                      SendDataToESP("AT+CWJAP_CUR=\"");
                                      SendDataToESP((const uint8_t*)WifiParams._wifiSSID);
                                      SendDataToESP("\",\"");
                                      SendDataToESP((const uint8_t*)WifiParams._wifiPSWD);
                                      SendDataToESP("\"\r\n"); // later ssid and psswd will be fetched from LCD
                                      _delay_cycles(SystemFreq*3);
                                      RetryTimeout5 = 10;
                                      do
                                      {
                                          //res = strstr((const char *)_MdmBuffer,"OK");
                                          res = NULL;
                                          res = strstr((const char *)_MdmBuffer,"OK");
                                          _delay_cycles(SystemFreq/2);
                                          if(*res == 'O')
                                          {
                                              break;
                                          }
                                          res = NULL;
                                          res = strstr((const char *)_MdmBuffer,"FAIL");
                                          _delay_cycles(SystemFreq/2);
                                          if(*res == 'F')
                                          {
                                              break;
                                          }
                                          RetryTimeout5 --;
                                      } while ((res == NULL)&&(RetryTimeout5>0)); // no more Blocking Call
                                      if(res!=NULL)
                                      {
                                          if((*res)=='F')
                                          {
                                              /* Standard replies when Failure occurs
                                               * AT+CWJAP_CUR="?",""<\r>
                                                 <\r>
                                                    <\n>WIFI DISCONNECT<\r>
                                                    <\n>+CWJAP:3<\r>
                                                    <\n><\r>
                                                    <\n>FAIL<\r>
                                                    <\n>
                                               */
                                              res = NULL;
                                              res = strstr((const char *)_MdmBuffer,"CWJAP:");
                                              res = res + 6;// point to <error code> in +CWJAP_CUR:<error code>
                                              if(*res == '1')
                                                  {
                                                      RetSts = _E8266_CWJAP_CON_TIMOUT_1;
                                                  }
                                              else if(*res == '2')
                                                  {
                                                      RetSts = _E8266_CWJAP_WRNG_PSWRD_2;
                                                  }
                                              else if(*res == '3')
                                                  {
                                                      RetSts = _E8266_CWJAP_AP_NT_FND_3;
                                                  }
                                              else if(*res == '4')
                                                  {
                                                      RetSts = _E8266_CWJAP_CON_GENERAL_FAIL_4;
                                                  }
                                              else
                                                  {
                                                      RetSts = _E8266_CWJAP_UNKNWN_FAIL;
                                                  }

                                          }
                                          else if((*res)=='O')
                                          {
                                              RetSts = _E8266_WIFI_CONCTD_SUCCESS;
                                              res = NULL;
                                              do
                                              {
                                                  res = strstr((const char *)_MdmBuffer,"WIFI GOT IP");
                                              } while (res == NULL); // Blocking Call

                                              if(res!=NULL)
                                              {
                                                  RetSts = _E8266_WIFI_RCV_IP_SUCCESS;
                                              }
                                          }

                                      }
                                      else
                                      {
                                          // no handling reqd.
                                      }



                                      RetryTimeout1--;

                                  } while((RetSts != _E8266_WIFI_RCV_IP_SUCCESS)&&(RetryTimeout1>0));

                                  if (RetryTimeout1 == 0)
                                   {
                                      //return the RetSts received

                                   }
                              }

                 ////////////Ping and check Internet availability ////////////////////////////
                 if(RetSts == _E8266_WIFI_RCV_IP_SUCCESS)
                 {
                     RetryTimeout1 = 2;
                     do
                     {
                         SendDataToESP("AT+CIFSR\r\n");

                         res = NULL;
                         res = mdmReplySearch((const char *)_MdmBuffer, "\""); //+CIFSR:STAIP,"192.168.1.7"<\r><\n>

                         res += 1; // points to the first number of the IP
                         temp_count = 0;
                         while(*res != '"')
                         {

                             WifiParams._wifiIPAddr[temp_count] = *res;
                             //_MdmIPAddr[temp_count] = *res;
                             temp_count++;
                             res++;
                         }
                         _delay_cycles(SystemFreq/5);

                         waitCount = 5;
                         SendDataToESP("AT+PING=\"www.turjasuzworld.online\"\r\n");
                         _delay_cycles(SystemFreq);

                         do
                         {
                             res = NULL;
                             res = strstr((const char *)_MdmBuffer,"OK");
                             _delay_cycles(SystemFreq/2);
                             if(*res == 'O')
                             {
                                 break;
                             }
                             res = NULL;
                             res = strstr((const char *)_MdmBuffer,"ERROR");
                             _delay_cycles(SystemFreq/2);
                             if(*res == 'E')
                             {
                                 break;
                             }
                             waitCount --;
                         } while ((res == NULL)&&(waitCount>0)); // Blocking Call but implemented timeout
                         if(*res == 'O')
                         {
                                 ctr = 0;
                             while(*res != '+') res--;      // search for the previous + sign
                             res++;
                             while((*res != '\r')&&(ctr<sizeof(WifiParams._wifiInetPingMs)))
                             {
                                 WifiParams._wifiInetPingMs[ctr]=*res;
                                 ctr++;
                                 res++;
                             }
                             RetSts = _E8266_PING_SUCCESS;
                         }
                         else if(*res == 'E')
                         {
                             WifiParams._wifiInetPingMs[0]='n';
                             WifiParams._wifiInetPingMs[1]='o';
                             WifiParams._wifiInetPingMs[2]='I';
                             WifiParams._wifiInetPingMs[3]='N';
                             RetSts = _E8266_PING_FAIL;
                         }
                         RetryTimeout1--;
                     } while((RetSts != _E8266_PING_SUCCESS)&&(RetSts != _E8266_PING_FAIL)&&(RetryTimeout1>0));

                     if (RetryTimeout1 == 0)
                     {
                         RetSts = _E8266_PING_TIMEOUT;
                     }
                 }

             }
             else
             {
                 // No handling reqd. the last status will be returned
             }


            break;



        case _Esp_disConnect_WIFI:
            // use case not needed
            break;




        case _Esp__repeated_send_Request:
            SendDataToUCA3("__case _Esp__repeated_send_Request in MdmMakeReady__\r");
//            runHomeDisplay(_UpdateTxDataTransaction);
            RetSts = currState;
            if((RetSts == _E8266_CIPSTART_ALREADY_CONNCTD)||(RetSts == _E8266_CIPSTART_UDP_OK)||(RetSts == _E8266_CIPSTART_OK)||(RetSts == _E8266_SEND_OK_RECVD)||(RetSts == _E8266_SEND_TIMEOUT))
            {
                 RetryTimeout1 = 5;
                 do
                 {
                     SendDataToESP("AT+CIPSEND="); // station mode set
                     /*
                      * Check if UDP or TCP needed
                      * Send length accordingly
                      */
                     if(_udpDataLen > 0)
                     {
                         if((_udpDataLen >=1000)&&(_udpDataLen <2048))
                         {
                             SendCharToESP(_udpDataLen/1000 + 48);
                             SendCharToESP((_udpDataLen%1000)/100 + 48);
                             SendCharToESP((_udpDataLen%100)/10 + 48);
                             SendCharToESP(_udpDataLen%10 + 48);
                         }
                         else if(_udpDataLen >=100)
                         {
                             SendCharToESP(_udpDataLen/100 + 48);
                             SendCharToESP((_udpDataLen%100)/10 + 48);
                             SendCharToESP(_udpDataLen%10 + 48);
                         }
                         else if(_udpDataLen >=10)
                         {
                             SendCharToESP(_udpDataLen/10 + 48);
                             SendCharToESP(_udpDataLen%10 + 48);
                         }
                         else // < 10
                         {
                             SendCharToESP(_udpDataLen%10 + 48);
                         }

                     }
                     else //  for TCP
                     {
                         if(strlen((const char*)_getUrlPtr) >=100)
                         {
                             SendCharToESP(strlen((const char*)_getUrlPtr)/100 + 48);
                             SendCharToESP((strlen((const char*)_getUrlPtr)%100)/10 + 48);
                             SendCharToESP(strlen((const char*)_getUrlPtr)%10 + 48);
                         }
                         else if(strlen((const char*)_getUrlPtr) >=10)
                         {
                             SendCharToESP(strlen((const char*)_getUrlPtr)/10 + 48);
                             SendCharToESP(strlen((const char*)_getUrlPtr)%10 + 48);
                         }
                         else
                         {
                             SendCharToESP(strlen((const char*)_getUrlPtr)%10 + 48);
                         }
                     }
                     SendDataToESP("\r\n");
                     res = NULL;
                     res = mdmReplySearch((const char *)_MdmBuffer, ">");
                     if(*res=='>')
                     {
                         RetSts = _E8266_CIPSEND_ARROW_SUCCESS;
                     }
                     else
                     {
                         // HANDLING TO BE DONE HERE
                     }


                     RetryTimeout1--;

                 } while((RetSts != _E8266_CIPSEND_ARROW_SUCCESS)&&(RetryTimeout1>0));

                 if (RetryTimeout1 == 0)
                 {
                     RetSts = _E8266_CIPSEND_ARROW_TIMEOUT;// => CONTINUE FROM HERE
                 }

                 //////////////// Next send the GET Request in HTTP1.1 format without Secured channel/////
                 if(RetSts == _E8266_CIPSEND_ARROW_SUCCESS)
                 {
                     RetryTimeout1 = 1; // Keep retry of sending the string to only once as under one connection attempt, sending
                                         // The api request or data twice is of no use
                         do
                         {
                             if(_udpDataLen > 0)
                             {
                                 for (udpDataCtr = 0; udpDataCtr < _udpDataLen; ++udpDataCtr)
                                 {
                                     SendCharToESP(_getUrlPtr[udpDataCtr]);
                                 }

                             }
                             else
                             {
                                  SendDataToESP((const char*)_getUrlPtr); // GET request sent to Server
                             }
                                     RetryTimeout5 = 0;
                                     do
                                     {
                                         RetryTimeout5++;
                                         _delay_cycles(SystemFreq*2);//Adjust this as per network latency

                                     } while((_MdmBuffCnt == 0)&&(RetryTimeout5 < 3));



                                     RetryTimeout2 = 20;
                                     do
                                     {
                                         _delay_cycles(SystemFreq);
                                         if(_udpDataLen > 0)
                                         {
                                          res = NULL;
                                          res = strstr((const char *)_MdmBuffer,"SEND OK");//OK was changed to CLOSED on 2 May 2020
                                          _delay_cycles(SystemFreq/5);
                                          if(*res == 'S') //O was changed to CLOSED on 2 May 2020
                                          {

                                              break;
                                          }
                                         }

                                         res = NULL;
                                         res = strstr((const char *)_MdmBuffer,"CLOSED");//OK was changed to CLOSED on 2 May 2020
                                         _delay_cycles(SystemFreq/5);
                                         if(*res == 'C') //O was changed to CLOSED on 2 May 2020
                                         {

                                             break;
                                         }
                                         res = NULL;
                                         res = strstr((const char *)_MdmBuffer,"FAIL");
                                         _delay_cycles(SystemFreq/5);
                                         if(*res == 'F')
                                         {

                                             break;
                                         }

                                         RetryTimeout2--;
                                     } while ((res == NULL)&&(RetryTimeout2>0)); // Blocking Call

                                     if(res != NULL)
                                     {

                                             if((*res=='C')||(*res=='S')) //O was changed to CLOSED on 2 May 2020
                                             {
                                                 RetSts = _E8266_SEND_OK_RECVD;
                                             }
                                             else if(*res=='F')
                                             {
                                                 // HANDLING TO BE DONE HERE
                                                 RetSts = _E8266_SEND_FAIL;
                                             }
                                     }
                                     else if(RetryTimeout2 == 0)
                                     {
                                         RetSts = _E8266_SEND_TIMEOUT;// => CONTINUE FROM HERE
                                     }
//                                     else
//                                     {
//                                         RetSts = _E8266_SM_CMD_ERROR;
//                                     }

                                     RetryTimeout1--;
                         } while((RetSts != _E8266_SEND_OK_RECVD)&&(RetryTimeout1>0));

                 }
                 else
                 {
                     //RetSts != _E8266_CIPSEND_ARROW_SUCCESS
                 }
                 ////////// GET handling completed //////////////////////////////////////

                             if((RetSts == _E8266_SEND_OK_RECVD)&&(_udpServPort == _E8266_NON_UDP_MODE))
                             {

//                                         runHomeDisplay(_UpdateRxDataTransaction); // Indicate on HMI that data has been received from server

                                         /*
                                          * Fetch the GET DATA into RAM
                                          */

                                             temp_mdm = strchr((const char*)_MdmBuffer, '{'); //char *strchr(const char *str, int c) searches for the first
                                                                                         //occurrence of the character c (an unsigned char) in the
                                                                                         //string pointed to by the argument str //
                                             temp_mdm+= 2;                        // To make the pointer parse the Battery % val and also the mV val.

                                             while(*temp_mdm != '}')
                                             {

                                                 _MdmHTTPBuff[Charcount]= *temp_mdm;
                                                 temp_mdm++;
                                                 Charcount++;
                                             }

                                             //void (*ptr)() = &ParseMdmBuffer; // Callback to ParseMdmBuffer is disabled due to suspected stack crash.

                                             //ParseCallback(ptr);
                                             //ParseMdmBuffer();
                             }
                             else
                             {
                                //if  RetSts != _E8266_SEND_OK_RECVD
                             }

            }
            else
            {
                //if RetSts != _E8266_CIPSTART_OK
            }

//            runHomeDisplay(_UpdateIoTRxTxDataTransaction);

            break;

        case _Esp_GET_Request:
            SendDataToUCA3("__case _Esp_GET_Request in MdmMakeReady__\r");

            ///// Fetch the IP using IPFSR//////////
            RetSts = currState;
            if((RetSts == _E8266_WIFI_RCV_IP_SUCCESS)||(RetSts == _E8266_PING_SUCCESS)||(RetSts == _E8266_SEND_OK_RECVD))
            {


                /////////// Next Send the GET request to the server = CONNECT TO SERVER ///////////

                    RetryTimeout1 = 2;
                    do
                    {
                        SendDataToESP("AT+CIPSTART=\"TCP\",\""); // station mode set
                        SendDataToESP(_servUrlPtr);
                        SendDataToESP("\",80\r\n");
                        _delay_cycles(SystemFreq*3);
                        // Wait till modem output is silent !/////////
                        RetryTimeout5 = 0;
                        do
                        {
                            RetryTimeout5++;
                            _delay_cycles(SystemFreq*2);//Adjust this as per network latency

                        } while((_MdmBuffCnt == 0)&&(RetryTimeout5 < 3));
                        if(RetryTimeout5 == 3)
                        {
                            return _E8266_CIPSTART_TIMEOUT;
                        }
                        //////////////////////////////////////////////
                        RetryTimeout2 = 10;
                        do
                        {
                            // Check for CONNECT
                            res = NULL;
                            res = strstr((const char *)_MdmBuffer,"DNS Fail");
                            _delay_cycles(SystemFreq/5);
                            if(*res == 'D')
                            {
                                break;
                            }
                            // Check for CONNECT
                            res = NULL;
                            res = strstr((const char *)_MdmBuffer,"CONNECT");
                            _delay_cycles(SystemFreq/5);
                            if(*res == 'C')
                            {
                                break;
                            }
                            //Check for ALREADY CONNECTED
                            res = NULL;
                            res = strstr((const char *)_MdmBuffer,"ALREADY");
                            _delay_cycles(SystemFreq/5);
                            if(*res == 'A')
                            {
                                break;
                            }
                            // Second Check for ERROR
                            res = NULL;
                            res = strstr((const char *)_MdmBuffer,"FAIL");
                            _delay_cycles(SystemFreq/5);
                            if(*res == 'F')
                            {
                                break;
                            }
                            // Second Check for ERROR
                            res = NULL;
                            res = strstr((const char *)_MdmBuffer,"CLOSED");
                            _delay_cycles(SystemFreq/5);
                            if(*res == 'C')
                            {
                                break;
                            }

                            RetryTimeout2--;
                        } while ((res == NULL)&&(RetryTimeout2>0));
                        //res += 1;   // Since CLOSED and CLOSE starts with C, so we check with O and L and A for FAIL
                        if((*res=='D')&&(*(res+1)=='N'))
                        {
                            RetSts = _E8266_CIPSTART_DNS_ERROR;//
                        }
                        else if((*res=='C')&&(*(res+1)=='O'))
                        {
                            RetSts = _E8266_CIPSTART_OK;//CONNECT CONDITION
                        }
                        else if((*res=='F')&&(*(res+1)=='A'))
                        {
                            RetSts = _E8266_CIPSTART_ERROR;// For FAIL
                        }
                        else if((*res=='A')&&(*(res+1)=='L'))
                        {
                            RetSts = _E8266_CIPSTART_ALREADY_CONNCTD;//For ALREADY CONNECTED
                        }
                        else if((*res=='C')&&(*(res+1)=='L'))
                        {
                            // NO HANDLING DONE FOR CLOSED RetSts = _E8266_CIPSTART_ALREADY_CONNCTD;//For ALREADY CONNECTED
                        }
                        else
                        {
                            // no handling for OTHER USE CASES
                        }

                        RetryTimeout1--;

                    } while((RetSts != _E8266_CIPSTART_OK)&&(RetryTimeout1>0));

                    if (RetryTimeout1 == 0)
                    {
                        RetSts = _E8266_CIPSTART_TIMEOUT;
                    }

            }
            else
            {
                //RetSts = _E8266_SM_CMD_ERROR;
            }

            break;
////////////////////////////////////////////////////

        case _Esp_UDP_GET_Request:
            SendDataToUCA3("__case _Esp_UDP_GET_Request in MdmMakeReady__\r");
            ///// Fetch the IP using IPFSR//////////
            RetSts = currState;
            if((RetSts == _E8266_WIFI_RCV_IP_SUCCESS)||(RetSts == _E8266_PING_SUCCESS)||(RetSts == _E8266_SEND_OK_RECVD))
            {
                /////////// Next Send the GET request to the server = CONNECT TO SERVER ///////////

                RetryTimeout1 = 2;
                do
                {
                    /*
                     * AT+CIPSTART=<type>,<remoteIP>,       <remote port>[,(<UDPlocalport>),(<UDPmode>)]
                     * AT+CIPSTART="UDP","192.168.101.110",1000,                1002,           2
                     */
                    SendDataToESP("AT+CIPSTART=\"UDP\",\""); // station mode set
                    SendDataToESP(_udpServUrlPtr);
                    SendDataToESP("\",");
                    if(_udpServPort >=1000)
                    {
                        SendCharToESP(_udpServPort/1000 + 48);
                        SendCharToESP((_udpServPort%1000)/100 + 48);
                        SendCharToESP((_udpServPort%100)/10 + 48);
                        SendCharToESP(_udpServPort%10 + 48);
                    }
                    else if(_udpServPort >=100)
                    {
                        SendCharToESP(_udpServPort/100 + 48);
                        SendCharToESP((_udpServPort%100)/10 + 48);
                        SendCharToESP(_udpServPort%10 + 48);
                    }
                    else if(strlen((const char*)_udpServPort) >=10)
                    {
                        SendCharToESP(_udpServPort/10 + 48);
                        SendCharToESP(_udpServPort%10 + 48);
                    }
                    else
                    {
                        SendCharToESP(_udpServPort%10 + 48);
                    }
                    SendDataToESP("\r\n");
//                    SendDataToESP(",");
//                    SendDataToESP(_udpServPort); // Enable if local port != remote port
//                    SendDataToESP("\r\n");
                    _delay_cycles(SystemFreq*3);
                    //                        res = NULL;
                    //                        do
                    //                        {
                    //                            res = strstr((const char *)_MdmBuffer,"CONNECT");
                    //                        } while (res == NULL); // Blocking Call



                    RetryTimeout2 = 5;
                    do
                    {
                        // Check for CONNECT
                        res = NULL;
                        res = strstr((const char *)_MdmBuffer,"CONNECT");
                        _delay_cycles(SystemFreq/5);
                        if(*res == 'C')
                        {
                            break;
                        }
                        //Check for ALREADY CONNECTED
                        res = NULL;
                        res = strstr((const char *)_MdmBuffer,"ALREADY");
                        _delay_cycles(SystemFreq/5);
                        if(*res == 'A')
                        {
                            break;
                        }
                        // Second Check for ERROR
                        res = NULL;
                        res = strstr((const char *)_MdmBuffer,"FAIL");
                        _delay_cycles(SystemFreq/5);
                        if(*res == 'F')
                        {
                            break;
                        }
                        // Second Check for ERROR
                        res = NULL;
                        res = strstr((const char *)_MdmBuffer,"CLOSED");
                        _delay_cycles(SystemFreq/5);
                        if(*res == 'C')
                        {
                            break;
                        }

                        RetryTimeout2--;
                    } while ((res == NULL)&&(RetryTimeout2>0));
                    res += 1;   // Since CLOSED and CLOSE starts with C, so we check with O and L and A for FAIL
                    if(*res=='O')
                    {
                        RetSts = _E8266_CIPSTART_UDP_OK;
                    }
                    else if(*res=='A')
                    {
                        RetSts = _E8266_CIPSTART_ERROR;// For FAIL
                    }
                    else if(*res=='L')
                    {
                        RetSts = _E8266_CIPSTART_ALREADY_CONNCTD;//For ALREADY CONNECTED
                    }
                    else
                    {
                        // no handling for CLOSED, may add later
                    }

                    RetryTimeout1--;

                } while((RetSts != _E8266_CIPSTART_UDP_OK)&&(RetryTimeout1>0));

                if (RetryTimeout1 == 0)
                {
                    RetSts = _E8266_CIPSTART_TIMEOUT;
                }

            }
            else
            {
                //RetSts = _E8266_SM_CMD_ERROR;
            }

            break;






////////////////////////////////////////////////////
        case _Esp_SET_Request:


            SendDataToUCA3("__case _Esp_SET_Request in MdmMakeReady__\r");
            RetSts = currState;
            if((RetSts == _E8266_CIPSTART_OK)||(RetSts == _E8266_SEND_OK_RECVD)||(RetSts == _E8266_SEND_TIMEOUT))
            {
                 RetryTimeout1 = 5;
                 do
                 {
                     SendDataToESP("AT+CIPSEND="); // station mode set
                     if(strlen((const char*)_setUrlPtr) >=100)
                     {
                         SendCharToESP(strlen((const char*)_setUrlPtr)/100 + 48);
                         SendCharToESP((strlen((const char*)_setUrlPtr)%100)/10 + 48);
                         SendCharToESP(strlen((const char*)_setUrlPtr)%10 + 48);
                     }
                     else if(strlen((const char*)_setUrlPtr) >=10)
                     {
                         SendCharToESP(strlen((const char*)_setUrlPtr)/10 + 48);
                         SendCharToESP(strlen((const char*)_setUrlPtr)%10 + 48);
                     }
                     else
                     {
                         SendCharToESP(strlen((const char*)_setUrlPtr)%10 + 48);
                     }
                     SendDataToESP("\r\n");
                     res = NULL;
                     res = mdmReplySearch((const char *)_MdmBuffer, ">");
                     if(*res=='>')
                     {
                         RetSts = _E8266_CIPSEND_ARROW_SUCCESS;
                     }
                     else
                     {
                         // HANDLING TO BE DONE HERE
                     }


                     RetryTimeout1--;

                 } while((RetSts != _E8266_CIPSEND_ARROW_SUCCESS)&&(RetryTimeout1>0));

                 if (RetryTimeout1 == 0)
                 {
                     RetSts = _E8266_CIPSEND_ARROW_TIMEOUT;// => CONTINUE FROM HERE
                 }

                 //////////////// Next send the GET Request in HTTP1.1 format without Secured channel/////
                 if(RetSts == _E8266_CIPSEND_ARROW_SUCCESS)
                 {
                     RetryTimeout1 = 2;
                         do
                         {
                                     SendDataToESP(_setUrlPtr);
                                     //SendDataToESP(_MdmConstructedSetUrl); // GET request sent to Server
                                     while(_MdmBuffCnt == 0);
                                     _delay_cycles(SystemFreq);//SystemFreq/2
//                                     SendDataToESP("AT+CIPCLOSE\r\n"); // Close the Connection and wait for SEND OK
//                                     _delay_cycles(SystemFreq);


                                     RetryTimeout2 = 20;
                                     do
                                     {
                                         res = NULL;
                                         res = strstr((const char *)_MdmBuffer,"CLOSED");//OK was changed to CLOSED on 2 May 2020
                                         _delay_cycles(SystemFreq/5);
                                         if(*res == 'C') //O was changed to CLOSED on 2 May 2020
                                         {

                                             break;
                                         }
                                         res = NULL;
                                         res = strstr((const char *)_MdmBuffer,"FAIL");
                                         _delay_cycles(SystemFreq/5);
                                         if(*res == 'F')
                                         {

                                             break;
                                         }

                                         RetryTimeout2--;
                                     } while ((res == NULL)&&(RetryTimeout2>0)); // Blocking Call

                                     if(res != NULL)
                                     {

                                             if(*res=='C') //O was changed to CLOSED on 2 May 2020
                                             {
                                                 RetSts = _E8266_SEND_OK_RECVD;
                                             }
                                             else if(*res=='F')
                                             {
                                                 // HANDLING TO BE DONE HERE
                                                 RetSts = _E8266_SEND_FAIL;
                                             }
                                     }
                                     else if(RetryTimeout2 == 0)
                                     {
                                         RetSts = _E8266_SEND_TIMEOUT;// => CONTINUE FROM HERE
                                     }
                                     else
                                     {
                                         RetSts = _E8266_SM_CMD_ERROR;
                                     }

                                     RetryTimeout1--;
                         } while((RetSts != _E8266_SEND_OK_RECVD)&&(RetryTimeout1>0));

                 }
                 else
                 {
                     //RetSts != _E8266_CIPSEND_ARROW_SUCCESS
                 }
                 ////////// GET handling completed //////////////////////////////////////

                             if(RetSts == _E8266_SEND_OK_RECVD)
                             {


                             }
                             else
                             {
                                //if  RetSts != _E8266_SEND_OK_RECVD
                             }

            }
            else
            {
                //if RetSts != _E8266_CIPSTART_OK
            }

//            runHomeDisplay(_UpdateIoTRxTxDataTransaction);

            break;
//////////////////////
        case _Esp_UDP_Close_Request:
            RetSts = currState;
            if((RetSts == _E8266_SEND_OK_RECVD)||(RetSts == _E8266_CIPCLOSE_UDP)||(RetSts == _E8266_CIPSTART_DNS_ERROR))
            {
                RetryTimeout1 = 5;
                do
                {
                    SendDataToESP("AT+CIPCLOSE\r\n"); // station mode set
                    res = NULL;
                    res = mdmReplySearch((const char *)_MdmBuffer, "CLOSED");
                    if(*res=='C')
                    {
                        RetSts = _E8266_CIPCLOSE_UDP_SUCCESS;
                    }
                    else
                    {
                        //TODO: HANDLING TO BE DONE HERE IF NEEDED
                    }


                    RetryTimeout1--;

                } while((RetSts != _E8266_CIPCLOSE_UDP_SUCCESS)&&(RetryTimeout1>0));
                if (RetryTimeout1 == 0)
                {
                    RetSts = _E8266_CIPCLOSE_UDP_FAIL;
                }

            }
            break;
//////////////////////
        default:
            break;
    }



    SendDataToUCA3("State machine ->> __RetSts: ");
    SendCharToUCA3(RetSts/10 + 48);
    SendCharToUCA3(RetSts%10 + 48);
    SendDataToUCA3("\r\n");
    return RetSts;
}

char*               mdmReplySearch(const char* _buff, const char* compStr)
{
    char* temp_res=NULL;
    int   timeout = 10;
    while((_MdmBuffCnt == 0)&&(timeout>0))
    {
        _delay_cycles(SystemFreq);
        timeout--;
    }
    if(timeout==0) return "Timeout";
    _delay_cycles(SystemFreq);
    temp_res = strstr(_buff, compStr);//strstr(, )
    return temp_res;

}

/* NOT FOR UDP MODE !!!!
 * Complete tested routine to manage
 * connection, reconnection and server GET/SET access flow
 */
esp8266StateMachines    _provision_Connect_GETcall_ESP(esp8266StateMachines _state, bool getChkNeeded, char* _GET_URL, char* _SERVER_URL)
{
    volatile  esp8266StateMachines k = _state;
    static int _espPwrOnRetryTimeout=0;
//    k=_E8266_PWR_UP;

    if((k == _E8266_PWR_UP)||
            (k == _E8266_SEND_OK_RECVD)||
            (k == _E8266_PING_SUCCESS)||
            (k == _E8266_CIPSTART_OK)||
            (k == _E8266_CIPSEND_ARROW_TIMEOUT)||
            (k == _E8266_CIPSTART_TIMEOUT)||
            (k == _E8266_SEND_FAIL)||
            (k == _E8266_SEND_TIMEOUT)||
            (k == _E8266_SM_CMD_ERROR))
    {
                if((k == _E8266_PWR_UP)||
                        (k == _E8266_CIPSEND_ARROW_TIMEOUT)||
                        (k == _E8266_CIPSTART_TIMEOUT)||
                        (k == _E8266_SEND_TIMEOUT)||
                        (k == _E8266_SEND_FAIL))
                {
                        _espPwrOnRetryTimeout=0;
                        while((k != _E8266_PWR_UP_SUCCESS)&&(_espPwrOnRetryTimeout<5))
                        {
                            if((k == _E8266_PWR_UP)||
                                    (k == _E8266_PWR_UP_FAILURE)||
                                    (k == _E8266_CIPSEND_ARROW_TIMEOUT)||
                                    (k == _E8266_CIPSTART_TIMEOUT)||
                                    (k == _E8266_SEND_TIMEOUT)||
                                    (k == _E8266_SEND_FAIL))
                                {
                                    disESP8266();
                                    _delay_cycles(SystemFreq);
                                    enESP8266();
                                }
                            if(k == _E8266_PWR_UP)
                                {
                                    k = MdmMakeReady(_Esp_Full_Init, k,NULL,NULL,NULL,NULL,_E8266_NON_UDP_MODE,0);
                                }
                            else
                                {
                                    k = MdmMakeReady(_Esp_Full_Init, _E8266_PWR_UP,NULL,NULL,NULL,NULL,_E8266_NON_UDP_MODE,0);// IGNORING REST USE CASES OF k AS OF NOW
                                }
                            _espPwrOnRetryTimeout++;
                        }

                        if(_espPwrOnRetryTimeout == 5)
                        {
                              if(k == _E8266_PWR_UP_FAILURE)  k = MdmMakeReady(_Esp_Full_Init, _E8266_PWR_DN,NULL,NULL,NULL,NULL,_E8266_NON_UDP_MODE,0);
                              return k; // any unwanted states will be returned and server conn will be aborted!
                        }




                        //// ESP can't reach below location unless
                        //// _E8266_PWR_UP_SUCCESS is received
                        k = MdmMakeReady(_Esp_Connect_WIFI, k,NULL,NULL,NULL,NULL,_E8266_NON_UDP_MODE,0);
                        if((k == _E8266_PING_FAIL)||(k == _E8266_CWJAP_AP_NT_FND_3)||(k == _E8266_CWJAP_WRNG_PSWRD_2))
                        {
                            return k; // any unwanted states will be returned and server conn will be aborted!
                        }
                }
        /*
         *  This part is only required if
         *  there is GET response needed
         */
        if(getChkNeeded)
        {
            k = MdmMakeReady(_Esp_GET_Request, k,_GET_URL,(char*)_SERVER_URL,NULL,NULL,_E8266_NON_UDP_MODE,0);
            if(k != _E8266_CIPSTART_DNS_ERROR)
            {
                k = MdmMakeReady(_Esp__repeated_send_Request, k,_GET_URL,(char*)_SERVER_URL,NULL,NULL,_E8266_NON_UDP_MODE,0);
            }
            else
            {
                k=MdmMakeReady(_Esp_UDP_Close_Request, k, NULL, NULL, NULL, NULL, _E8266_NON_UDP_MODE, 0);
            }
            _delay_cycles(SystemFreq*5);
        }
        else
        {

        }

    }
    return k; // should return _E8266_PING_SUCCESS (if getChkNeeded == false) or _E8266_SEND_OK_RECVD (if getChkNeeded == true)
}





/*
 * callback function to parse data received in UCAx Buffer
 */
void ParseCallback(void (*ptr)())
{
    (*ptr) (); // callback to A
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *    Interrupt Vectors for the data acquired from modem
 */

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:

    break;                             // Vector 0 - no interrupt log_data

  case 2:                                   // Vector 2 - RXIFG
      _MdmBuffer[_MdmBuffCnt] = UCA0RXBUF;                  // TX -> RXed character
      SendCharToUCA3(_MdmBuffer[_MdmBuffCnt]);              // comment if usb terminal logging is not needed
      _MdmBuffCnt++;
      _bytesDataTransacted++;
      if(_MdmBuffCnt==1024) _MdmBuffCnt=0; // Added on 2 May 2020
    break;

  case 4:

    break;                             // Vector 4 - TXIFG

  default:

    break;
  }

  __bic_SR_register_on_exit(LPM0_bits);     // exit power save mode if super loop is required in main
}
