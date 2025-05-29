/*
 * TW_MSP430G2xx_ESP.c
 *
 *  Created on: May 25, 2020
 *      Author: Turjasu
 */
#include "TW_MSP430G2xx_ESP.h"


/*
 *  Variables
 */

volatile                          unsigned char           _MdmBuffer[_bufferMax],
                                                          _MdmStatus[80],        // Global MBuffer to store the modem replies. Define in your required C/CPP files/classes
                                                          _MdmHTTPBuff[_bufferMax],
                                                          _MdmIPAddr[16] = {NULL};
                                  unsigned char           _APpassword[30]="TM#4C1294NCPDTI3",
                                                          _APname[30]="TurjasuzWorldAP8";//INTIALIZED VALUE, CHECK FOR THIS
volatile                          unsigned int            _MdmBuffCnt = 0;
                                  char                    *temp_mdm = NULL;
volatile  unsigned char                                   _MdmAPN[] = "internet******";
uint8_t                                                   _devIP_Address[16]={NULL};

const   char                                    _AT_reply[] = "OK";
const   char                                    _AT_CWMODE_CUR_reply[]="OK";
const   char                                    _AT_CWJAP_CUR_reply[]="WIFI GOT IP";
const   char                                    _AT_CIFSR_reply[]="+CIFSR:STAIP";
const   char                                    _AT_PING_reply[]="OK";
const   char                                    _AT_CIPSTART_reply[]="CONNECT";
const   char                                    _AT_CIPSEND_reply[]="OK";
const   char                                    _AT_CIPCLOSE_reply[]="SEND OK";




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
uint8_t ConfigureEspUART(long unsigned int baudrate, uint8_t interrupt_polling)
{
//      UCA0CTL1 |= UCSWRST;                     // **Reset USCI state machine**
     _ESP_PORT_SEL |= (_ESP_UART_RX + _ESP_UART_TX);                             // P3.4,5 = USCI_A0 TXD/RXD
     _ESP_PORT_SEL2 |= (_ESP_UART_RX + _ESP_UART_TX);
     UCA0CTL1 |= UCSSEL_2;                     // SMCLK
     // Configure the baudrate values
     switch (baudrate)                             // Default BR is 9600, if not mentioned
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

     UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

     switch (interrupt_polling)
     {
        case 1:
            IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt

            break;
        case 2:
            IE2 &= ~(UCA0TXIE + UCA0RXIE);                         // Enable USCI_A0 RX interrupt
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
void     SendDataToESP(const uint8_t* data)
{
//    uint8_t indx=0;

        ClrEspBuff();
        while(*data)
        {
            while (!(IFG2&UCA0TXIFG));             // USCI_A0 TX buffer ready?
            UCA0TXBUF = *data;                  // TX -> RXed character
            data++;
        }



}

/*
 *  Send single character data to modem
 */
void      SendCharToESP(unsigned char data)
{
        while (!(IFG2&UCA0TXIFG));             // USCI_A0 TX buffer ready?
        UCA0TXBUF = data;                  // TX -> RXed character
        _delay_cycles(SystemFreq/100);

}

/*
 *  Disable the Long Response and Echo,
 *  Condition is saved in _MdmInitSts
 */
void    DeEchoShrtRsp(void)
{}


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

esp8266StateMachines    ReadEspRSSI(unsigned char* apName)
{
    esp8266StateMachines RetSts;
    static char* res = NULL;
    ///////////// Network Strength Fetch ////////////
    static int RetryTimeout = 3;
    do
    {
        SendDataToESP("AT+CWLAP\r\n");
        res = NULL;
        do
        {
            res = strstr((const char *)_MdmBuffer,"OK");
        } while (res == NULL); // Blocking Call

        if(*res == 'O')
        {
            RetSts = _E8266_RSSI_RCV_SUCCESS;
        }

        RetryTimeout--;

    } while((RetSts != _E8266_RSSI_RCV_SUCCESS)&&(RetryTimeout>0));

    if (RetryTimeout == 0)
    {
        RetSts = _E8266_RSSI_RCV_TIMEOUT;
    }

    else if(RetSts == _E8266_RSSI_RCV_SUCCESS)
    {
        temp_mdm = strstr((const char*)_MdmBuffer, apName); // replace TurjasuzworldAP9 with value from LCD
        temp_mdm+= strlen(apName);
        temp_mdm += 3;
        int Charcount= 42; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
        while(*temp_mdm != ',')
        {

            _MdmStatus[Charcount]= *temp_mdm;
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
//    ///////////// Network Strength Fetch ////////////
//    uint8_t RetryTimeout = 5;
//    do
//    {
//        SendDataToMdm("AT+QISHOWRA=1\r");
// //       while(_MdmBuffCnt < sizeof(QILOCIPRply)-1);
//        _delay_cycles(SystemFreq*7);
//        StrResult = strncmp(_MdmBuffer, CSQRply, 5);
//        RetryTimeout--;
//
//    } while((StrResult != 0)&&(RetryTimeout>1));
//
//    if (RetryTimeout == 1)
//            {
//                _MdmStatus[42] = 0xFA;
//                _MdmStatus[43] = 0xFA;
//                return -1;
//            }
//    else
//    {
//        temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
//                                                    //occurrence of the character c (an unsigned char) in the
//                                                    //string pointed to by the argument str //
//        temp_mdm+= 2;
//        int Charcount= 42; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
//        while(*temp_mdm != ',')
//        {
//
//            _MdmStatus[Charcount]= *temp_mdm;
//            temp_mdm++;
//            Charcount++;
//        }
//
//    }
//
//    return 0;
}



esp8266StateMachines    MdmMakeReady(espOperCommand OperCmd, esp8266StateMachines currState, char* getString, char* setString) // currState will be verified everytime
{
    /*                  === GET EXAMPLE =====
     *

     */
    esp8266StateMachines RetSts =  _E8266_PWR_UP;
    static int          RetryTimeout1=0, RetryTimeout2=0;//RetryTimeout3=0,RetryTimeout4=0,RetryTimeout5=0;
//    unsigned char    temp[4];
    volatile int Charcount=0, res_type = 0;
    static int apiLength=0, apnValidation=0, temp_count=0, waitCount = 0;
    volatile char* res = NULL;
//    static  char    _getString[] = getString, _setString[]= setString;
///////////// Initialization Switch Cases////////////////////
    switch (OperCmd)
    {
        case _Esp_Full_Init:
            ////SendDataToUCA3("__case _Esp_Full_Init in MdmMakeReady__\r");
            RetryTimeout1 = 5;
            while((RetryTimeout1>0)&& (RetSts == _E8266_PWR_UP||
                                        RetSts == _E8266_AT_RESPNSE_FAIL||
                                        RetSts == _E8266_DeECHO_FAIL||
                                        RetSts == _E8266_CIFSR_FAIL||
                                        RetSts == _E8266_CWJAP_CON_TIMOUT_1))
            {
                if(RetSts == _E8266_PWR_UP)
                {
                    RetSts = ESP_ON_OFF(TurnOnImmediately);
                }
                else
                {
                    //Modem_ON_OFF(Restart);
                    RetSts = ESP_ON_OFF(Restart);// States = unknown / Fail / E8266 rst success
                }

                switch (RetSts)
                {
                    case _UNKNOWN:

                        ESP_ON_OFF(TurnOff); // alternatively we can send module to deep sleep
                        RetSts = _UNKNOWN;

                        break;

                    case _FAIL:

                        ESP_ON_OFF(TurnOff); // alternatively we can send module to deep sleep
                        RetSts = _FAIL;

                         break;

                    case _E8266_RST_SUCCESS:
                        // no handling, Retsts will be returned
                         break;

                    case _E8266_PWR_UP:
                        //AT and ATE0 handling to be done here later

                        RetSts = _E8266_PWR_UP_SUCCESS;
                         break;
                    default:
                        break;
                }

                RetryTimeout1--;
//                RetSts = MdmInitAndDiag();
                RetSts = moduleInitDiag(RetSts); // Esp8266 initialization and update the state machine state here
                _delay_cycles(SystemFreq);
            }

            break;



        case _Esp_Connect_WIFI:
             RetSts = currState;
             if((RetSts == _E8266_PWR_UP)||(RetSts == _E8266_RST_SUCCESS)||(RetSts == _E8266_DeECHO_SUCCESS))
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
                                      SendDataToESP((const uint8_t*)_APname);
                                      SendDataToESP("\",\"");
                                      SendDataToESP((const uint8_t*)_APpassword);
                                      SendDataToESP("\"\r\n"); // later ssid and psswd will be fetched from LCD
                                      //_delay_cycles(SystemFreq*3);

                                      do
                                      {
                                          //res = strstr((const char *)_MdmBuffer,"OK");
                                          res = NULL;
                                          res = strstr((const char *)_MdmBuffer,"OK");
                                          _delay_cycles(SystemFreq/5);
                                          if(*res == 'O')
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

                                      } while (res == NULL); // Blocking Call
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
                         _delay_cycles(SystemFreq/2);

                         do
                         {
                             res = NULL;
                             res = strstr((const char *)_MdmBuffer,"\"");//+CIFSR:STAIP,"192.168.1.7"<\r><\n>
                                                                                     //+CIFSR:STAMACA,"ec:fa:Tbc:5b:57:d9"<\r><\n>
                             _delay_cycles(SystemFreq/5);
                         } while (res == NULL); // Blocking Call

                         res += 1; // points to the first number of the IP
                         temp_count = 0;
                         while(*res != '\r')
                         {
                             _devIP_Address[temp_count] = *res;
                             //_MdmIPAddr[temp_count] = *res;
                             temp_count++;
                             res++;
                         }
                         _delay_cycles(SystemFreq/5);

                         waitCount = 5;
                         SendDataToESP("AT+PING=\"www.google.com\"\r\n");
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
                             RetSts = _E8266_PING_SUCCESS;
                         }
                         else if(*res == 'E')
                         {
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
//            RetSts = _M95_STOP_CGATT_SUCCESS;
//            ///////////// Check CGATT STATUS ////////////
//            RetryTimeout = 5;
//            do
//            {
//                SendDataToMdm("AT+CGATT?\r"); // CHECK IF GPRS IS ATTACHED ?
//                _delay_cycles(SystemFreq*3);
//                StrResult = strncmp(_MdmBuffer, CGATT_OFF_Rply, 11);
//                if(StrResult != 0)
//                {
//                    SendDataToMdm("AT+CGATT=0\r"); // if GPRS bearer detached, attach it
//                    _delay_cycles(SystemFreq*3);
//                }
//                RetryTimeout--;
//
//            } while((StrResult != 0)&&(RetryTimeout>0));
//            if (RetryTimeout == 0)
//             {
//                RetSts = _M95_STOP_CGATT_FAIL;
//             }
            break;
        case _Esp_GET_Request:

            ///// Concatenate the GET URL//////////
//            _getString = strcat(_getString, _serviceURI);
//            _getString = strcat(_getString, _xVal);
//            // next send the x value
//            _getString = strcat(_getString, _yVal);
//            //next send the y value

            RetSts = currState;
            if((RetSts == _E8266_WIFI_RCV_IP_SUCCESS)||(RetSts == _E8266_PING_SUCCESS)||(RetSts == _E8266_SEND_OK_RECVD))
            {


                /////////// Next Send the GET request to the server = CONNECT TO SERVER ///////////

                    RetryTimeout1 = 2;
                    do
                    {

                        SendDataToESP("AT+CIPSTART=\"TCP\",\"turjasuzworld.in\",80\r\n"); // station mode set
                        //_delay_cycles(SystemFreq*3);
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
                            RetSts = _E8266_CIPSTART_OK;
                        }
                        else if(*res=='A')
                        {
                            RetSts = _E8266_CIPSTART_ERROR;
                        }
                        else
                        {
                            // no handling for CLOSED, may add later
                        }

                        RetryTimeout1--;

                    } while((RetSts != _E8266_CIPSTART_OK)&&(RetryTimeout1>0));

                    if (RetryTimeout1 == 0)
                    {
                        RetSts = _E8266_CIPSTART_TIMEOUT;
                    }
                    /////////// Next Send the GET request to the server = CIPSEND ///////////

               if(RetSts == _E8266_CIPSTART_OK)
               {
                    RetryTimeout1 = 5;
                    do
                    {
                        SendDataToESP("AT+CIPSEND=117\r\n"); // station mode set
                        _delay_cycles(SystemFreq/50);
                        res = NULL;
                        do
                        {
                            res = strstr((const char *)_MdmBuffer,">");
                            _delay_cycles(SystemFreq/5);
                        } while (res == NULL); // Blocking Call
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
                                SendDataToESP((const uint8_t *)getString); // GET request sent to Server
                                _delay_cycles(SystemFreq/2);
                                SendDataToESP("AT+CIPCLOSE\r\n"); // Close the Connection and wait for SEND OK
                                _delay_cycles(SystemFreq);
                                /*
                                res = NULL;
                                do
                                {
                                    res = strstr((const char *)_MdmBuffer,"SEND "); // SEND FAIL handling also has to be done!!
                                } while (res == NULL); // Blocking Call
                                res += 5;               // Point to the OK / FAIL
                                */

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
                            } while((RetSts == _E8266_SM_CMD_ERROR)&&(RetryTimeout1>0));

                    }
                    else
                    {
                        //RetSts != _E8266_CIPSEND_ARROW_SUCCESS
                    }
                    ////////// GET handling completed //////////////////////////////////////

                                if(RetSts == _E8266_SEND_OK_RECVD)
                                {

                                            ////runHomeDisplay(_UpdateRxDataTransaction); // Indicate on HMI that data has been received from server

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

#if     _espRxDataParsingReqd
                                                ParseMdmBuffer();
#endif
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



            }
            else
            {
                //RetSts = _E8266_SM_CMD_ERROR;
            }


            ////runHomeDisplay(_UpdateIoTRxTxDataTransaction);

            break;
        case _Esp_SET_Request:
            ///// Concatenate the SET API ///////////////////
//            _setString = strcat(_setString, _turjasuzworldURI);
//            _setString = strcat(_setString, _serviceURI);
//            _setString = strcat(_setString, _xVal);
//            // next send the x value
//            _setString = strcat(_setString, _yVal);
//            //next send the y value


            RetSts = currState;
            if((RetSts == _E8266_WIFI_RCV_IP_SUCCESS)||(RetSts == _E8266_PING_SUCCESS)||(RetSts == _E8266_TEST_ENUM)||(RetSts == _E8266_SEND_OK_RECVD))
            {


                /////////// Next Send the GET request to the server = CONNECT TO SERVER ///////////

                    RetryTimeout1 = 5;
                    do
                    {
                        SendDataToESP("AT+CIPSTART=\"TCP\",\"turjasuzworld.in\",80\r\n"); // station mode set
                        //_delay_cycles(SystemFreq*3);
                        res = NULL;
                        do
                        {
                            res = strstr((const char *)_MdmBuffer,"CONNECT");
                        } while (res == NULL); // Blocking Call
                        if(*res=='C')
                        {
                            RetSts = _E8266_CIPSTART_OK;
                        }
                        else
                        {
                            // NON CONNECT HANDLING TO BE DONE HERE
                        }

                        RetryTimeout1--;

                    } while((RetSts != _E8266_CIPSTART_OK)&&(RetryTimeout1>0));

                    if (RetryTimeout1 == 0)
                    {
                        RetSts = _E8266_CIPSTART_ERROR;
                    }
                    /////////// Next Send the GET request to the server = CIPSEND ///////////
                    RetryTimeout1 = 5;
                    do
                    {
                        apiLength = (int)strlen((const char*)setString);
                        apiLength += 8;
                        SendDataToESP("AT+CIPSEND=");
                        SendCharToESP((apiLength/100) + 48); // hundreds
                        SendCharToESP(((apiLength%100)/10)+ 48); // tens
                        SendCharToESP((apiLength%10)+ 48);// units
                        SendDataToESP("\r\n");// Length of the SET API
                        //_delay_cycles(SystemFreq*3);
                        res = NULL;
                        do
                        {
                            res = strstr((const char *)_MdmBuffer,">");
                        } while (res == NULL); // Blocking Call
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
                            RetryTimeout1 = 5;
                            do
                            {
                                SendDataToESP((const uint8_t*)setString); // GET request sent to Server
                                _delay_cycles(SystemFreq);
                                SendDataToESP("AT+CIPCLOSE\r\n"); // Close the Connection and wait for SEND OK
                                _delay_cycles(SystemFreq*5);
                                res = NULL;
                                do
                                {
                                    res = strstr((const char *)_MdmBuffer,"CLOSED"); // SEND OK changed to  CLOSED ; SEND FAIL handling also has to be done!!
                                } while (res == NULL); // Blocking Call

                                if(*res=='C')
                                {
                                    RetSts = _E8266_SEND_OK_RECVD;
                                }
                                else
                                {
                                    // HANDLING TO BE DONE HERE
                                    //RetSts = _E8266_SEND_FAIL;
                                }

                                RetryTimeout1--;

                            } while((RetSts != _E8266_SEND_OK_RECVD)&&(RetryTimeout1>0));

                            if (RetryTimeout1 == 0)
                            {
                                RetSts = _E8266_SEND_TIMEOUT;// => CONTINUE FROM HERE
                            }
                    }
                    ////////// SET handling completed //////////////////////////////////////

                            }
                            else
                            {
                                //RetSts = _E8266_SM_CMD_ERROR;
                            }


            ////runHomeDisplay(_UpdateIoTRxTxDataTransaction);

            break;


        default:
            break;
    }



    //SendDataToUCA3("__RetSts: ");
    //SendCharToUCA3(RetSts);
    //SendDataToUCA3("\r\n");
    return RetSts;
}

esp8266StateMachines espConnectWiFi(void)
{
    esp8266StateMachines RetSts;
    int var;
    RetSts = MdmMakeReady(_Esp_Connect_WIFI, _E8266_PWR_UP,NULL,NULL);
//    switch (RetSts)
//    {
//        case _E8266_PING_SUCCESS:
//
//            break;
//        case _E8266_CWJAP_CON_TIMOUT_1:
//
//            break;
//        case _E8266_CWJAP_WRNG_PSWRD_2:
//
//            break;
//        case _E8266_CWJAP_AP_NT_FND_3:
//
//            break;
//        case _E8266_CWJAP_CON_GENERAL_FAIL_4:
//
//            break;
//        default:
//
//            break;
//    }


    return RetSts;
}

esp8266StateMachines espReadFromServer(esp8266StateMachines _state)                              // Specific Callback handler for Branon only to read IOT status (60Sec)
{
      esp8266StateMachines RetSts;
      RetSts = MdmMakeReady(_Esp_GET_Request,_state,NULL,NULL);
      return RetSts;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *    Interrupt Vectors for the data acquired from modem
 */

/*
 *  UCA0 INTERRUPT
 */
 #if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
 #pragma vector=USCIAB0RX_VECTOR
 __interrupt void USCI0RX_ISR(void)
 #elif defined(__GNUC__)
 void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
 #else
 #error Compiler not supported!
 #endif
 {
//   while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
//   UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character

         _MdmBuffer[_MdmBuffCnt] = UCA0RXBUF;                  // TX -> RXed character
         _MdmBuffCnt++;
         if(_MdmBuffCnt==_bufferMax) _MdmBuffCnt=0; // Added on 2 May 2020
 }

