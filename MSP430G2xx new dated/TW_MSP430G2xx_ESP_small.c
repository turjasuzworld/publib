/*
 * TW_MSP430G2xx_ESP_small.c
 *
 *  Created on: May 27, 2020
 *      Author: Turjasu
 */

#include "TW_MSP430G2xx_ESP_small.h"

/*
 *  Variables
 */

volatile                          unsigned char           _MdmBuffer[_bufferMax];
                                                          //_MdmIPAddr[16] = {NULL};
                                  unsigned char           _APpassword[30]="TM#4C1294NCPDTI3",
                                                          _APname[30]="TurjasuzWorldAP8";//INTIALIZED VALUE, CHECK FOR THIS
volatile                          unsigned int            _MdmBuffCnt = 0;
                                  char                    *temp_mdm = NULL;

const   char                                    _AT_reply[] = "OK";
const   char                                    _AT_CWMODE_CUR_reply[]="OK";
const   char                                    _AT_CWJAP_CUR_reply[]="WIFI GOT IP";
const   char                                    _AT_CIFSR_reply[]="+CIFSR:STAIP";
const   char                                    _AT_PING_reply[]="OK";
const   char                                    _AT_CIPSTART_reply[]="CONNECT";
const   char                                    _AT_CIPSEND_reply[]="OK";
const   char                                    _AT_CIPCLOSE_reply[]="SUCCESS";




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




/*
 * baudrate will be 9600 to any suitable value.
 * Oversampling is off. please read datasheet before pushing values
 */
uint8_t ConfigureEspUART(long unsigned int baudrate, uint8_t interrupt_polling)
{
      UCA0CTL1 |= UCSWRST;                     // **Reset USCI state machine**
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
           UCA0MCTL = UCBRS_6;              // cpu @ 1mhz, BAUD @ 115200
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
            __bis_SR_register(GIE);
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
 *  AT Command reply comparison
 *  This is a blocking call !!
 *
 *  @param: returns the pointer to the found reply using strstr()
 */
char*     checkReply(const char* bufferToCheck, const char* replyFormat)
{
    static  char*   string_temp = NULL;
    do {
        _delay_cycles(SystemFreq*2);
        string_temp = strstr((const char*)bufferToCheck, replyFormat);


    } while (string_temp == NULL);

    return string_temp;
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


         _MdmBuffer[_MdmBuffCnt] = UCA0RXBUF;                  // TX -> RXed character
         _MdmBuffCnt++;
         if(_MdmBuffCnt==_bufferMax) _MdmBuffCnt=0; // Added on 2 May 2020
 }
