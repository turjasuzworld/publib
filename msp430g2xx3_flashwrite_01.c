/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430G2xx3 Demo - Flash In-System Programming, Copy SegC to SegD
//
//  Description: This program first erases flash seg C, then it increments all
//  values in seg C, then it erases seg D, then copies seg C to seg D.
//  Assumed MCLK 771kHz - 1428kHz.
//  //* Set Breakpoint on NOP in the Mainloop to avoid Stressing Flash *//
//
//               MSP430G2xx3
//            -----------------
//        /|\|              XIN|-
//         | |                 |
//         --|RST          XOUT|-
//           |                 |
//
//  D. Dang
//  Texas Instruments Inc.
//  December 2010
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <msp430.h>

//blink led


const char  fw_c000_2[82]={0xB2,0x40,0x80,0x5A,0x20,0x01,0xD2,0xD3,0x22,0x00,0xD2,0xE3,0x21,0x00,0x0D,0x12,
                           0x0E,0x12,0x3D,0x40,0x8C,0x10,0x3E,0x40,0x03,0x00,0x1D,0x83,0x0E,0x73,0xFD,0x23,
                           0x0D,0x93,0xFB,0x23,0x3E,0x41,0x3D,0x41,0x00,0x3C,0xEF,0x3F,0x31,0x40,0x00,0x04,
                           0xB0,0x12,0x4E,0xC0,0x0C,0x43,0xB0,0x12,0x00,0xC0,0x1C,0x43,0xB0,0x12,0x48,0xC0,
                           0x32,0xD0,0x10,0x00,0xFD,0x3F,0x03,0x43,0x03,0x43,0xFF,0x3F,0x03,0x43,0x1C,0x43,
                           0x30,0x41};

char  value;                                // 8-bit value to write to segment A

// Function prototypes
__attribute__((section(".my_code")))
void iap(void);
typedef void (*FunctionPtr)(void);
void write_SegC (char value);
void copy_C2D (void);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
  {											
    while(1);                               // do not load, trap CPU!!	
  }
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1MHz
  DCOCTL = CALDCO_1MHZ;
  FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
  value = 0;                                // initialize value
//  __disable_interrupt();
  iap();
//  __enable_interrupt();
////  while(1);
////  while(1)                                  // Repeat forever
////  {
////    write_SegC(value++);                    // Write segment C, increment value
////    copy_C2D();                             // Copy segment C to D
//
    __no_operation();                       // SET BREAKPOINT HERE
      FunctionPtr myFunction = (FunctionPtr) 0xD000; // Cast address to function pointer
      myFunction();
//    ((void ( * )())0xC000)();

//  }
}

//#pragma DATA_SECTION(iap, ".text")
//__attribute__((section(".my_code")))
void iap(void) {
    char *Flash_ptr;                          // Flash pointer
    unsigned int i;

    Flash_ptr = (char *) 0xD000;//0x1040;              // Initialize Flash pointer
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit
    *Flash_ptr = 0;                           // Dummy write to erase Flash segment

    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

    for (i=0; i<sizeof(fw_c000_2); i++)
    {
      *Flash_ptr++ = fw_c000_2[i];                   // Write value to flash
      //Flash_ptr++;
      //if(Flash_ptr > 0xC036) Flash_ptr = (char *) 0xc000;
    }

//    Flash_ptr = (char *) 0xFFFE;
//    *Flash_ptr = 0x0;
//    Flash_ptr++;
//    *Flash_ptr = 0xC0;

    FCTL1 = FWKEY;                            // Clear WRT bit
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void write_SegC (char value)
{
  char *Flash_ptr;                          // Flash pointer
  unsigned int i;

  Flash_ptr = (char *) 0xc3a0;//0x1040;              // Initialize Flash pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash segment

  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i=0; i<64; i++)
  {
    *Flash_ptr++ = value;                   // Write value to flash
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void copy_C2D (void)
{
  char *Flash_ptrC;                         // Segment C pointer
  char *Flash_ptrD;                         // Segment D pointer
  unsigned int i;

  Flash_ptrC = (char *) 0x1040;             // Initialize Flash segment C pointer
  Flash_ptrD = (char *) 0x1000;             // Initialize Flash segment D pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  *Flash_ptrD = 0;                          // Dummy write to erase Flash segment D
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i=0; i<64; i++)
  {
    *Flash_ptrD++ = *Flash_ptrC++;          // copy value segment C to segment D
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}
