/*
 * TW_MSP430_NTP.c
 * This is suited for Branon application board
 *  Created on: May 20, 2021
 *      Author: Turjasu
 */

#include "TW_MSP430_NTP.h"


/*
 * NTP RELATED DECLARATIONS
 * & DEFINES
 */
#define NTP_PACKET_SIZE  48; // NTP time stamp is in the first 48 bytes of the message
char bufferNTP[48] = {
                      // Initialize values needed to form NTP request
                      // see https://www.meinbergglobal.com/english/info/ntp-packet.htm
                      0b11100011, // LI, Version, Mode
                      0,     // Stratum, or type of clock
                      6,     // Polling Interval
                      0xEC,  // Peer Clock Precision
                      0,0,0,0,0,0,0,0, // 8 bytes of zero for Root Delay & Root Dispersion
                      49,
                      52,   // idx = 15
                      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// Send NTP packet to a time server
//memset(bufferNTP, 0, NTP_PACKET_SIZE);


/*
 * NTP fxn to access the ntp server and then sync it from google time server
 * This fxn also needs TW_ESP8266_v2 files for accessing the udp using the ESP01
 *
 */
ntpOperCommand      _syncNtpWithGoogleServer(const char* _ntpGoogleServerIP, const unsigned int _ntpUdpPort, const unsigned int _ntpPacketSizeLength)
{
    /* MdmMakeReady creates a udp connection to "time4.google.com" at port 123 and sends out a standard
     * NTP packet of 48 bytes. Inreply it gets time from server. That is then parsed and time at local
     * time is calculated. The time zone taken here is that of IST by default.
     */
    static esp8266StateMachines justToClearSetApiCall = _UNKNOWN;
    static ntpOperCommand ntpStateMachine = _NTP_INITIALIZED;
    char* findNtpData = NULL;
    bool  ntpTimeFetchSuccess = false;
    unsigned int NtpBuffCtr = 0;
    volatile struct tm  ts;
    volatile struct tm* ts_ptr = NULL;
    volatile time_t _deviceTime;
    ntpStateMachine = _NTP_START;
     justToClearSetApiCall = MdmMakeReady(_Esp_UDP_GET_Request, _E8266_PING_SUCCESS, NULL, NULL, (char*)_ntpGoogleServerIP, NULL, _ntpUdpPort,0);
     justToClearSetApiCall = MdmMakeReady(_Esp__repeated_send_Request, justToClearSetApiCall, bufferNTP, NULL, "time4.google.com", NULL, _ntpUdpPort,_ntpPacketSizeLength);
     __delay_cycles(SystemFreq);
     __delay_cycles(SystemFreq);
     __delay_cycles(SystemFreq);
     __delay_cycles(SystemFreq);
     //findNtpData = mdmReplySearch((const char *)_MdmBuffer, "+IPD");
     //findNtpData = strstr(_MdmBuffer, "+IPD");
     findNtpData = memchr(_MdmBuffer,'+',sizeof(_MdmBuffer));
     if(findNtpData)
     {
      ntpStateMachine = _NTP_PARSING_SRVR_DATA;
      findNtpData += 8; // skip the +IPD,48: part to get the data
      for (NtpBuffCtr = 0; NtpBuffCtr < 48; ++NtpBuffCtr)
       {
        bufferNTP[NtpBuffCtr] = *findNtpData;
        findNtpData++;
       }
      ntpTimeFetchSuccess = true;
      ntpStateMachine = _NTP_SUCCESS;
     }
     else
     {
         ntpStateMachine = _NTP_ERROR;
     }
     justToClearSetApiCall = MdmMakeReady(_Esp_UDP_Close_Request, justToClearSetApiCall, NULL, NULL, (char*)_ntpGoogleServerIP, NULL, _ntpUdpPort,0);
     // Timestamp starts at byte 40 of the received packet and is 4 bytes long.
     _deviceTime   = bufferNTP[NTP_PACKET_TIMESTAMP_STRT_LOC];
     _deviceTime <<= 8;
     _deviceTime  |= bufferNTP[NTP_PACKET_TIMESTAMP_STRT_LOC + 1];
     _deviceTime <<= 8;
     _deviceTime  |= bufferNTP[NTP_PACKET_TIMESTAMP_STRT_LOC + 2];
     _deviceTime <<= 8;
     _deviceTime  |= bufferNTP[NTP_PACKET_TIMESTAMP_STRT_LOC + 3];
     _deviceTime  += 19800;


     // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
//         time(&_deviceTime);
     ts_ptr = localtime((const time_t*)&_deviceTime);
     ts = *ts_ptr;
#ifdef  CONFIGURE_FOR_IST   // conditional compilation if we need Indian Standard time adjustment or else (set in ntp header file)
     ts.tm_isdst = 1;
     ts.tm_year += 1900;
     ts.tm_mon += 1;
//     ts.tm_hour += 5;
//     ts.tm_min += 30;
//         ts.tm_mday += 1;
#elif

#endif


#ifdef  CONFIGURE_BQ32000_AFTER_SYNC
     if (ntpTimeFetchSuccess)
     {
         /*
          * Structure of BQ32000_Setting[]: ss mm hh dw DD MM YY r r r
          */
             ntpStateMachine = _NTP_SETTING_RTC_BQ32000;
             BQ32000_Setting[0] = 0;
             BQ32000_Setting[0] |= (ts.tm_sec/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[0] <<= 4;
             BQ32000_Setting[0] |= ((ts.tm_sec%10)& 0x0F);

             BQ32000_Setting[1] = 0;
             BQ32000_Setting[1] |= (ts.tm_min/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[1] <<= 4;
             BQ32000_Setting[1] |= ((ts.tm_min%10)& 0x0F);

             BQ32000_Setting[2] = 0;
             BQ32000_Setting[2] |= (ts.tm_hour/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[2] <<= 4;
             BQ32000_Setting[2] |= ((ts.tm_hour%10)& 0x0F);

             BQ32000_Setting[3] = 0;
             BQ32000_Setting[3] |= (ts.tm_wday/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[3] <<= 4;
             BQ32000_Setting[3] |= ((ts.tm_wday%10)& 0x0F);

             BQ32000_Setting[4] = 0;
             BQ32000_Setting[4] |= (ts.tm_mday/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[4] <<= 4;
             BQ32000_Setting[4] |= ((ts.tm_mday%10)& 0x0F);

//             ts.tm_mon ++;
             BQ32000_Setting[5] = 0;
             BQ32000_Setting[5] |= (ts.tm_mon/10);//((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[5] <<= 4;
             BQ32000_Setting[5] |= ((ts.tm_mon%10)& 0x0F);

//             ts.tm_year += 1900 ; // fetch back the current year
             BQ32000_Setting[6] = 0;
             BQ32000_Setting[6] = ((ts.tm_year%100)/10);//Take the last 2 of 2020 ((*timeParsePtr) << 4)|((*(timeParsePtr +1))& 0x0F);
             BQ32000_Setting[6] <<= 4;
             BQ32000_Setting[6] |= ((ts.tm_year%10)& 0x0F);

#if       RTC_TYPE_BQ32000
             WriteTo24CxxEEPROMMultiBytes_InterruptMode(0xD0, 2, 0, 10, BQ32000_Setting);
             __delay_cycles(SystemFreq);
             ReadFrom24CxxEEPROMMultiBytes_InterruptMode(0xD0, 2, 0, 7, TimeRead);
             if((TimeRead[1]==BQ32000_Setting[1])
                 &&(TimeRead[2]==BQ32000_Setting[2])
                 &&(TimeRead[3]==BQ32000_Setting[3])
                 &&(TimeRead[4]==BQ32000_Setting[4])
                 &&(TimeRead[5]==BQ32000_Setting[5])
                 &&(TimeRead[6]==BQ32000_Setting[6]))
             {
                 ntpStateMachine = _NTP_BQ32000_SET_SUCCESS;
             }
             else
             {
                 ntpStateMachine = _NTP_BQ32000_SET_FAILURE;
             }
#elif        RTC_TYPE_DS1307

#elif        RTC_TYPE_MSP430F5419A_RTC_A

             /*
              *  Set the RTC inside MSP430
              */
             // Initialize LFXT1
             P7SEL |= 0x03;                            // Select XT1
             UCSCTL6 &= ~(XT1OFF);                     // XT1 On
             UCSCTL6 |= XCAP_3;                        // Internal load cap
             // Configure RTC_A
             //RTCCTL01 |= RTCTEVIE;
             RTCCTL01 |= RTCRDYIE + RTCBCD + RTCHOLD + RTCMODE;
                                                       // RTC enable, BCD mode, RTC hold
                                                       // enable RTC read ready interrupt
                                                       // enable RTC time event interrupt

             RTCYEAR = 0x2000;                         // Year = 0x2000 + difference from GOOG
             RTCYEAR |= BQ32000_Setting[6];
             RTCMON = BQ32000_Setting[5];              // Month
             RTCDAY = BQ32000_Setting[4];            // Day
             RTCDOW = BQ32000_Setting[3];            // Day of week = 0x01 = Monday
             RTCHOUR = BQ32000_Setting[2];                           // Hour = 0x10
             RTCMIN = BQ32000_Setting[1];                            // Minute = 0x32
             RTCSEC = BQ32000_Setting[0];                            // Seconds = 0x45

//             RTCADOWDAY = 0x2;                         // RTC Day of week alarm = 0x2
//             RTCADAY = 0x20;                           // RTC Day Alarm = 0x20
//             RTCAHOUR = 0x10;                          // RTC Hour Alarm
//             RTCAMIN = 0x23;                           // RTC Minute Alarm

             RTCCTL01 &= ~(RTCHOLD);                   // Start RTC calendar mode


             // Loop until XT1,XT2 & DCO fault flag is cleared
             do
             {
               UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
                                                       // Clear XT2,XT1,DCO fault flags
               SFRIFG1 &= ~OFIFG;                      // Clear fault flags
             }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
             ntpStateMachine = _NTP_BQ32000_SET_SUCCESS;
#endif

         }
#endif

     return ntpStateMachine;
}


//////////////////////////////////////////
// Utilities
//void convertEpoch2Structure(time_t timeEpoch, struct tm* timeStructure) // implementation not checked
//{
//    timeStructure = localtime((const time_t*)&timeEpoch);
//}
//
//void convertStructure2Epoch(struct tm* timeStructure, time_t* timeEpoch)// implementation not checked
//{
//    *timeEpoch = mktime(timeStructure);
//}

/*String convertDateTime2String(tm timeStructure)
{
    // asctime ends with \n, remove it!
    String result = (String)asctime(&timeStructure);
    result.trim();
    return result;
}

String convertDateTime2String(time_t timeEpoch)
{
    // ctime ends with \n, remove it!
    String result = (String)ctime(&timeEpoch);
    result.trim();
    return result;
}

String formatDateTime2String(const char * format, tm timeStructure)
{
    char buffer[128];
    memset(buffer, 0x00, strlen(buffer));
    strftime(buffer, 128, format, &timeStructure);
    strcat(buffer, 0x00);

//#if defined(ENERGIA_ARCH_CC13X2)
//    String result = buffer;
//    return result.substring(0, result.length() - 2);
//#else
    return (String)buffer;
//#endif
}

String formatDateTime2String(const char * format, time_t timeEpoch)
{
    tm _timeStructure;
    convertEpoch2Structure(timeEpoch, _timeStructure);
    return formatDateTime2String(format, _timeStructure);
}*/

//uint8_t convertString2DateTime(char* stringDateTime, char* stringFormat, time_t timeEpoch) // implementation not checked
//{
//    struct tm _timeStructure;
//
//    if (convertString2DateTime(stringDateTime, stringFormat, _timeStructure))
//    {
//        convertStructure2Epoch(_timeStructure, timeEpoch);
//        return _NTP_UTILITIES_OPER_SUCCESS;
//    }
//    else
//    {
//        return CONVERT_OTHER_ERROR;                                             // error
//    }
//}

//uint8_t convertString2DateTime(char* stringDateTime, char* stringFormat, struct tm timeStructure)
//{
//    struct tm _timeStructure;
//    char charDateTime[32];
//    char charFormat[32];
//    stringDateTime.toCharArray((char*)charDateTime, 32);
//    stringFormat.toCharArray((char*)charFormat, 32);
//
//    if (strptime(charDateTime, charFormat, &_timeStructure) == NULL)
//    {
//        return _NTP_UTILITIES;                                             // error
//    }
//    else
//    {
//        timeStructure = _timeStructure;
//        return CONVERT_SUCCESS;
//    }
//}
#ifdef      RTC_TYPE_MSP430F5419A_RTC_A
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(RTCIV,16))
  {
    case RTC_NONE:                          // No interrupts
      break;
    case RTC_RTCRDYIFG:                     // RTCRDYIFG
        __no_operation();
      break;
    case RTC_RTCTEVIFG:                     // RTCEVIFG
      __no_operation();                     // Interrupts every minute
      break;
    case RTC_RTCAIFG:                       // RTCAIFG
      break;
    case RTC_RT0PSIFG:                      // RT0PSIFG
      break;
    case RTC_RT1PSIFG:                      // RT1PSIFG
      break;
    case 12: break;                         // Reserved
    case 14: break;                         // Reserved
    case 16: break;                         // Reserved
    default: break;
  }
}
#endif

