#ifndef _TW_I2C_h

//=========== I2C ==================
#define     SCL                 BIT6
#define     SDA                 BIT7

extern void BasicInitUSCI(unsigned char);// slave address with 0 prefix, 1= I2C Master/0 = I2C Slave

extern void EEPROM24C64_Save(unsigned short , unsigned char, unsigned char);    // Saves one byte of data to i2c eeprom

extern void EEPROM24C64_Save_Burst(unsigned char, unsigned short , unsigned char *, int);    // Saves multiple byte - ONE PAGE

extern void EEPROM24C64_RRead_Burst(unsigned char, unsigned short , int , unsigned char *);  // Reads multiple BYTES -- ONE PAGE

extern void I2CSLV_Save_Burst(unsigned char , unsigned char , unsigned char *, int );      // Writes into any general i2c slave

extern void EEPROM24C02_RRead_Burst(unsigned char,unsigned char , int , unsigned char *);

extern void I2CSLV_RRead_Burst(unsigned char,unsigned char , int , unsigned char *); // Reads from i2c slave


extern int  ReadFromDS1307MultiBytes(unsigned short ,int , unsigned char *);  // READS THE RTC CHIP DS1307 USING USCIBx


# endif
