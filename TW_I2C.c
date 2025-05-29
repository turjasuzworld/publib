/* Initializes the USCIB0 as I2C Master
 * 7 bit addressing and many functions are there
 * like single and page write, read
 * Author: Turjasu
 * DATED: 22.06.2018
 * VERSION 1.01
 * CHECKED WORKING OF ALL SOURCE CODES WITH 24C64. USE I2CSLV_Save_Burst FOR ANY GENERAL I2C SLAVE LIKE RTC OR PORT EXPANDERS
 * ADDED SPECIAL READ BURST SOURCE CODE FOR DS1307
 * Tested in: MSP430G2553
 */


#include <msp430.h>
#include "TW_I2C.h"




void BasicInitUSCI(unsigned char Mode) // put slave address in 7 bit format 1010 A2A1A0, eg 1010111 with 0 in beginning
{
      P1SEL |= SCL + SDA;                       // Assign I2C pins to USCI_B0
      P1SEL2|= SCL + SDA;                       // Assign I2C pins to USCI_B0
      UCB0CTL1 |= UCSWRST;                      // Enable SW reset
      switch (Mode)
      {
      case 0:
          UCB0CTL0 &= ~UCMST;
          break;

      case 1:
            UCB0CTL0 |= UCMST;
            break;
        default:
            UCB0CTL0 |= UCMST;
            break;
      }
      UCB0CTL0 |= UCMODE_3+UCSYNC;         // I2C Master, synchronous mode
      UCB0CTL1 = UCSSEL_2+UCTR+UCSWRST;              // Use SMCLK, keep SW reset
      UCB0BR0 = 50;                             // fSCL = SMCLK/12 = ~100kHz
      UCB0BR1 = 0;
      UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
      _delay_cycles(100000);                    // 100 mS delay
}

//====================== Save one byte of data to the EEPROM ======================

void EEPROM24C64_Save(unsigned short addr, unsigned char SLAddr, unsigned char data)
  {
      unsigned short temp;
      UCB0I2CSA = SLAddr;
      UCB0CTL1 |= UCTR;                             // Transmitter mode
      IFG2 &= ~UCB0TXIFG;                           // Clears the TX Flag
      UCB0CTL1 |= UCTXSTT;                          // I2C start condition
      while(!(IFG2 & UCB0TXIFG));
      IFG2 &= ~UCB0TXIFG;                           // Clears the flag due to start condition
      temp = addr;
      UCB0TXBUF = (temp >>8);                       // Put the MSB address on bus
      while(!(IFG2 & UCB0TXIFG));
      IFG2 &= ~UCB0TXIFG;
      UCB0TXBUF = addr;                             // Put the LSB address on bus
      while(!(IFG2 & UCB0TXIFG));
      IFG2 &= ~UCB0TXIFG;
      UCB0TXBUF = data;
      while(!(IFG2 & UCB0TXIFG));
      IFG2 &= ~UCB0TXIFG;

      UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
      _delay_cycles(100000);                  // needed to ensure the writing time reqd.

  }

//==================Save Multiple Bytes of data into eeprom, but in Page mode ===
void EEPROM24C64_Save_Burst(unsigned char SLAddr, unsigned short addr, unsigned char *data, int count)
    {
        unsigned short temp;
         int n;
         UCB0I2CSA = SLAddr;
         UCB0CTL1 |= UCTR;                             // Transmitter mode
         IFG2 &= ~UCB0TXIFG;                           // Clears the TX Flag
         UCB0CTL1 |= UCTXSTT;                          // I2C start condition
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;                           // Clears the flag due to start condition
         temp = addr;
         UCB0TXBUF = (temp >>8);                       // Put the MSB address on bus
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;
         UCB0TXBUF = addr;                             // Put the LSB address on bus
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;
         for(n=0;n < count; n++ )
         {
         UCB0TXBUF = *data;
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;
         data++;
         _delay_cycles(1000);
         }

         UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
         _delay_cycles(1000000);                  // needed to ensure the writing time reqd.


    }
//=======================Save Multiple Bytes of data into 24C02 eeprom or EQV., but in Page mode ===================
void I2CSLV_Save_Burst(unsigned char SLAddr, unsigned char addr, unsigned char *data, int count)
    {/*
     / Adding the address of the slave should be done by putting a 0 before
      * the 7 bit address of the slave. E.g. SLV Address = 1101000
      / prefix a 0 to the first position and make it 01101000 = 0x68
      */
         int n;
         UCB0I2CSA = SLAddr;
         UCB0CTL1 |= UCTR;                             // Transmitter mode
         IFG2 &= ~UCB0TXIFG;                           // Clears the TX Flag
         UCB0CTL1 |= UCTXSTT;                          // I2C start condition
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;                           // Clears the flag due to start condition

         UCB0TXBUF = addr;                             // Put the LSB address on bus
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;
         for(n=0;n < count; n++ )
         {
         UCB0TXBUF = *data;
         while(!(IFG2 & UCB0TXIFG));
         IFG2 &= ~UCB0TXIFG;
         data++;
         _delay_cycles(1000);
         }

         UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
         while(UCB0CTL1 & UCTXSTP);             // Wait untill the stop condition is generated
         _delay_cycles(10000);                  // needed to ensure the writing time reqd.


    }

//==================== Read Multiple Bytes from the EEPROM ========================
void EEPROM24C64_RRead_Burst(unsigned char SLAddr,unsigned short addr, int count, unsigned char *DstArray)
    {
        unsigned char   junk;
        unsigned short temp;
        int track=0;
        UCB0I2CSA = SLAddr;                     // Set slave address
        UCB0CTL1 |= UCTXSTT;                    // I2C start condition

        while(!(IFG2 & UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;
        temp = addr;
        UCB0TXBUF = (temp >>8);                       // Put the MSB address on bus

        while(!(IFG2 & UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;

        UCB0TXBUF = addr;                             // Put the LSB address on bus
        while(!(IFG2 & UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;


        UCB0CTL1 &= ~UCTR;                      // Slave mode

        UCB0CTL1 |= UCTXSTT;                    // I2C Restart condition
        while (UCB0CTL1 & UCTXSTT);             // Loop until I2C STT is sent
        junk= UCB0RXBUF;
        while(!(IFG2 & UCB0RXIFG));             // Wait till the firt data is received
        IFG2 &= ~UCB0RXIFG;
 //       _delay_cycles(100000);
//        DstArray++;
        while(track<count)
            {
                *DstArray = UCB0RXBUF;
                track++;
                DstArray++;
                while(!(IFG2 & UCB0RXIFG));             // Wait till the firt data is received
                IFG2 &= ~UCB0RXIFG;
                _delay_cycles(100000);
            }
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
        while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
        _delay_cycles(100000);

    }
//=============================Read 24c02 EEPROM with only 8 bits of mem addr===========================================
void EEPROM24C02_RRead_Burst(unsigned char SLVAdr,unsigned char addr, int count, unsigned char *DstArray)
    {/*
     / Adding the address of the slave should be done by putting a 0 before
      * the 7 bit address of the slave. E.g. SLV Address = 1101000
      / prefix a 0 to the first position and make it 01101000 = 0x68
      */
        unsigned char   junk;
        int track=0;
        UCB0I2CSA = SLVAdr;
        UCB0CTL1 |= UCTXSTT;                    // I2C start condition

        while(!(IFG2 & UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;
        UCB0TXBUF = addr;                       // Put the address on bus

        while(!(IFG2 & UCB0TXIFG));
        IFG2 &= ~UCB0TXIFG;
        UCB0CTL1 &= ~UCTR;                      // Slave mode

        UCB0CTL1 |= UCTXSTT;                    // I2C Restart condition
        while (UCB0CTL1 & UCTXSTT);             // Loop until I2C STT is sent
        junk= UCB0RXBUF;
        IFG2 &= ~UCB0RXIFG;
        _delay_cycles(100000);
        DstArray++;
        while(track<count)
            {
                *DstArray = UCB0RXBUF;
                track++;DstArray++;
                IFG2 &= ~UCB0RXIFG;
//              if(track==(count-1))         UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
                _delay_cycles(100000);
            }
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
        while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent
        _delay_cycles(100000);

    }
//=======================================Read Multiple bytes from general I2C slave 24c02 I2CSLV=================================
void I2CSLV_RRead_Burst(unsigned char SLAddr,unsigned char addr, int count, unsigned char *DstArray)
    {/*
     / Adding the address of the slave should be done by putting a 0 before
      * the 7 bit address of the slave. E.g. SLV Address = 1101000
      / prefix a 0 to the first position and make it 01101000 = 0x68
      */
        unsigned char   junk;
        int track=0;
        UCB0I2CSA = SLAddr;                     // Set slave address
 //       UCB0CTL1 |= UCTXSTT;                    // I2C start condition
  //      UCB0TXBUF = addr;                       // Put the LSB address on bus, TXIFG will be autocleared


 //       while(IFG2 & UCB0TXIFG);
//        IFG2 &= ~UCB0TXIFG;


        UCB0CTL1 &= ~UCTR;                      // Slave mode

        UCB0CTL1 |= UCTXSTT;                    // I2C Restart condition
 //       junk= UCB0RXBUF;


        while(!(IFG2 & UCB0RXIFG));             // Wait till the firt data is received

        while((track<count)&&(IFG2 & UCB0RXIFG))
            {
                *DstArray = UCB0RXBUF;
                track++;
                DstArray++;
                _delay_cycles(1000);                 // Small delay to receive the next data
                while(!(IFG2 & UCB0RXIFG));          // Wait till the next data is received
            }
        IFG2 &= ~UCB0RXIFG;                         // Clear the RX FLAG incase if it is set
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition after 1st TX
        while (UCB0CTL1 & UCTXSTP);             // Loop until I2C STT is sent

    }
//========================= DS1307 Read Multibytes=================================================
int             ReadFromDS1307MultiBytes(unsigned short Addr,int BytesToRead, unsigned char *ReadTime)  // Reads a multiple bytes to provided Array pointer and checks unack condition
{                                                                                                       // Can be used for RAM bytes as well

    volatile unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
    volatile unsigned char  MemLoc = 0,temp=0;      // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    volatile int errorcode = 0,var;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    UCB0CTL1 |= UCSWRST;                            // Enable SW reset
    UCB0CTL1 |= UCTR;                               // Enabling the Tx mode
    addr = 0xD0;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB0I2CSA = addr;                               // Slave Address is fed
    UCB0CTL1 &= ~UCSWRST;                           // Release USCI
    UCB0CTL1 |= UCTXSTT;                            // I2C TX, start condition
//    while(UCB0CTL1 & UCTXSTT);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(10000);                           // 10 ms delay, change this value when using XT2 or other freq.
    if(!(UCB0CTL1 & UCTXNACK))
        {
            if(IFG2 & UCB0TXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB0TXBUF = MemLoc;
                    IFG2 &= ~UCB0TXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(IFG2 & UCB0TXIFG));                //wait till data is written into the slave
                }

            UCB0CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB0CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB0CTL1 & UCTXSTT);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(IFG2 & UCB0RXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToRead; ++var)
            {
                    if(IFG2 & UCB0RXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            *ReadTime = UCB0RXBUF;
                            IFG2 &= ~UCB0RXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(IFG2 & UCB0RXIFG));                //wait till data is written into the slave
                        }
                    ReadTime++;
            }

        }

    UCB0CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB0CTL1 & UCTXSTP);                      // Wait till the stop condition has been sent
    IFG2 &= ~UCB0TXIFG;                            // Clear any pending TX flags
    IFG2 &= ~UCB0RXIFG;                            // Clear any RX flags
    if(UCB0CTL1 & UCTXNACK)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB0CTL1 &= ~UCTXNACK;                              // Clear any NACK flags due to non responding slave
    }

    return errorcode;
}
