/*
 * TW_MSP430_NTP.h
 *
 *  Created on: May 20, 2021
 *      Author: Turjasu
 */

#ifndef TW_MSP430_NTP_H_
#define TW_MSP430_NTP_H_

#define     NTP_PACKET_TIMESTAMP_STRT_LOC               40
#define     CONFIGURE_BQ32000_AFTER_SYNC                true
#define     CONFIGURE_FOR_IST               true
#define     RTC_TYPE_BQ32000                false
#define     RTC_TYPE_DS1307                 false
#define     RTC_TYPE_MSP430F5419A_RTC_A     true

#include "ESPv4/TW_ESP8266_v4.h"
#include <Time.h>
//Conditional Compilation - set in ntp header
#ifdef  CONFIGURE_BQ32000_AFTER_SYNC
#include "BranonSpecific/BranonSpecific_v4.h"
#include "TW_USCI_Bx.h"
#endif
//
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



typedef     enum    {
        _NTP_INITIALIZED,
        _NTP_START,
        _NTP_PARSING_SRVR_DATA,
        _NTP_SETTING_RTC_BQ32000,
        _NTP_BQ32000_SET_SUCCESS,
        _NTP_BQ32000_SET_FAILURE,
        _NTP_SUCCESS,
        _NTP_ERROR,
        _NTP_UTILITIES_OPER_SUCCESS,
        _NTP_UTILITIES_OPER_ERROR

}ntpOperCommand;

extern  char bufferNTP[48];

extern      ntpOperCommand      _syncNtpWithGoogleServer(const char* , const unsigned int , const unsigned int );
///
/// @brief  Utilities
/// @{

/// @n  There are 2 representation for data and time.
/// * @b Epoch
/// @n POSIX time = number of seconds since 00:00 Jan 1st, 1979
/// * @b Structure
/// @n The MSP432 has its own not compatible structure!
/// MSP432 specific structure | Standard C structure
/// ---------------- | ----------------
/// struct _RTC_C_Calendar | struct tm
/// { | {
///   uint_fast8_t seconds; | int tm_sec;  // seconds after the minute [0-60]
///   uint_fast8_t minutes; | int tm_min;  // minutes after the hour [0-59]
///   uint_fast8_t hours; | int tm_hour;  // hours since midnight [0-23]
///   uint_fast8_t dayOfWeek; | (*)
///   uint_fast8_t dayOfmonth; | int tm_mday;  // day of the month [1-31]
///   uint_fast8_t month; | int tm_mon;  // months since January [0-11]
///   uint_fast16_t year; | int tm_year;  // years since 1900
///   (*) | int tm_wday;  // days since Sunday [0-6]
///   . | int tm_yday;  // days since January 1 [0-365]
///   . | int tm_isdst;  // Daylight Savings Time flag
///   . | long tm_gmtoff;  // offset from CUT in seconds
///   . | char *tm_zone;  // timezone abbreviation
/// }; | };
///

///
/// @brief      Convert epoch into structure
/// @param[in]  timeEpoch time as epoch, input
/// @param[out] timeStructure time as structure, output
///
extern  void convertEpoch2Structure(time_t , struct tm );


///
/// @brief      Convert structure into epoch
/// @param[in]  timeStructure time as time structure, input
/// @param[out] timeEpoch time as epoch, output
///
extern  void convertStructure2Epoch(struct tm timeStructure, time_t  timeEpoch);

///
/// @brief  Conversion error codes
///
/// @{
#define CONVERT_SUCCESS 0           ///< success
#define CONVERT_OTHER_ERROR 9       ///< other error
/// @}


///
/// @brief      Convert a string into date and time, strcuture format
/// @param      stringDateTime date and time as string, input
/// @param      stringFormat string format, input, see below
/// @param      timeStructure time as strcuture, output
/// @return     0 if success, otherwise error code
///
extern  uint8_t convertString2DateTime(char* stringDateTime, char* stringFormat, struct tm  timeStructure);

///
/// @brief      Standard format for date and time string
/// @param      timeStructure time as structure
/// @return     formated string
/// @note       Sun Jul 19 18:55:13 2015
///
extern  char* convertDateTime2String(struct tm timeStructure);

///
/// @brief      Standard format for date and time string
/// @param      timeEpoch time as epoch
/// @return     formated string
/// @note       Sun Jul 19 18:55:13 2015
///
extern  char* convertDateTime2String_time_t(time_t timeEpoch);

///
/// @brief      Custom format for date and time
/// @param      format see below
/// @param      timeStructure time as structure
/// @return     formated string
///
extern  char* formatDateTime2String(const char * format, struct tm timeStructure);

///
/// @brief      Custom format for time
/// @param      format see below
/// @param      timeEpoch time as epoch
/// @return     formated string
///
extern  char* formatDateTime2String_time_t(const char * format, time_t timeEpoch);


#endif /* TW_MSP430_NTP_H_ */
