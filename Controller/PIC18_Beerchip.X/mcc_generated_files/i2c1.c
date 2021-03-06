/**
  MSSP1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    i2c1.c

  @Summary
    This is the generated header file for the MSSP1 driver using 
    PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for I2C1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.65.2
        Device            :  PIC18F14K50
        Driver Version    :  2.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.45
        MPLAB 	          :  MPLAB X 4.15	
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "i2c1.h"

#define I2C1_SLAVE_ADDRESS 0x5C 
#define I2C1_SLAVE_MASK    0x7F


/**
 Section: Global Variables
*/

volatile uint8_t    I2C1_slaveWriteData      = 0x55;

static void (*I2C1_InterruptHandler)(I2C1_SLAVE_DRIVER_STATUS i2c_bus_state);

/**
 Section: Local Functions
*/
void I2C1_StatusCallback(I2C1_SLAVE_DRIVER_STATUS i2c_bus_state);


/**
  Prototype:        void I2C1_Initialize(void)
  Input:            none
  Output:           none
  Description:      I2C1_Initialize is an
                    initialization routine that takes inputs from the GUI.
  Comment:          
  Usage:            I2C1_Initialize();

*/
void I2C1_Initialize(void)
{
    // initialize the hardware
    // R_nW write_noTX; P stopbit_notdetected; S startbit_notdetected; BF RCinprocess_TXcomplete; SMP Standard Speed; UA dontupdate; CKE disabled; D_nA lastbyte_address; 
    SSPSTAT = 0x80;
    // SSPEN enabled; WCOL no_collision; CKP disabled; SSPM 7 Bit Polling; SSPOV no_overflow; 
    SSPCON1 = 0x26;
    // ACKEN disabled; GCEN disabled; PEN disabled; ACKDT acknowledge; RSEN disabled; RCEN disabled; ACKSTAT received; SEN enabled; 
    SSPCON2 = 0x01;
    // MSK 127; 
    SSPMSK = 0xFE;
    // SSPADD 92; 
    SSPADD = 0x5C;
	
    // clear the slave interrupt flag
    PIR1bits.SSPIF = 0;
    // enable the master interrupt
    PIE1bits.SSPIE = 1;

}

void I2C1_ISR ( void )
{
    uint8_t     i2c_data                = 0x55;

    // NOTE: The slave driver will always acknowledge
    //       any address match.

    PIR1bits.SSPIF = 0;        // clear the slave interrupt flag
    i2c_data        = SSPBUF;  // read SSPBUF to clear BF
    if( 1 == SSPSTATbits.R_nW )
    {
        if((1 == SSPSTATbits.D_nA) && (1 == SSPCON2bits.ACKSTAT))
        {
            // callback routine can perform any post-read processing
            if( I2C1_InterruptHandler != NULL )
                I2C1_InterruptHandler(I2C1_SLAVE_READ_COMPLETED);
        }
        else
        {
            // callback routine should write data into SSPBUF
            if( I2C1_InterruptHandler != NULL )
                I2C1_InterruptHandler(I2C1_SLAVE_READ_REQUEST);
        }
    }
    else if(0 == SSPSTATbits.D_nA)
    {
        // this is an I2C address

        // callback routine should prepare to receive data from the master
        if( I2C1_InterruptHandler != NULL )
            I2C1_InterruptHandler(I2C1_SLAVE_WRITE_REQUEST);
    }
    else
    {
        I2C1_slaveWriteData   = i2c_data;

        // callback routine should process I2C1_slaveWriteData from the master
        if( I2C1_InterruptHandler != NULL )
            I2C1_InterruptHandler(I2C1_SLAVE_WRITE_COMPLETED);
    }

    SSPCON1bits.CKP    = 1;    // release SCL

} // end I2C1_ISR()

void I2C1_SetInterruptHandler(void (* InterruptHandler)(I2C1_SLAVE_DRIVER_STATUS)){
    I2C1_InterruptHandler = InterruptHandler;
}

/**

    Example implementation of the callback

    This slave driver emulates an EEPROM Device.
    Sequential reads from the EEPROM will return data at the next
    EEPROM address.

    Random access reads can be performed by writing a single byte
    EEPROM address, followed by 1 or more reads.

    Random access writes can be performed by writing a single byte
    EEPROM address, followed by 1 or more writes.

    Every read or write will increment the internal EEPROM address.

    When the end of the EEPROM is reached, the EEPROM address will
    continue from the start of the EEPROM.
*/

#if 0
void I2C1_StatusCallback(I2C1_SLAVE_DRIVER_STATUS i2c_bus_state)
{
    static uint8_t EEPROM_Buffer[] =
    {
        0xB0,0xB1,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
        0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
        0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
        0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f
    };

    static uint8_t eepromAddress    = 0;
    static uint8_t slaveWriteType   = SLAVE_NORMAL_DATA;


    switch (i2c_bus_state)
    {
        case I2C1_SLAVE_WRITE_REQUEST:
            // the master will be sending the eeprom address next
            slaveWriteType  = SLAVE_DATA_ADDRESS;
            break;


        case I2C1_SLAVE_WRITE_COMPLETED:

            switch(slaveWriteType)
            {
                case SLAVE_DATA_ADDRESS:
                    eepromAddress   = I2C1_slaveWriteData;
                    break;


                case SLAVE_NORMAL_DATA:
                default:
                    // the master has written data to store in the eeprom
                    EEPROM_Buffer[eepromAddress++]    = I2C1_slaveWriteData;
                    if(sizeof(EEPROM_Buffer) <= eepromAddress)
                    {
                        eepromAddress = 0;    // wrap to start of eeprom page
                    }
                    break;

            } // end switch(slaveWriteType)

            slaveWriteType  = SLAVE_NORMAL_DATA;
            break;

        case I2C1_SLAVE_READ_REQUEST:
            SSPBUF = EEPROM_Buffer[eepromAddress++];
            if(sizeof(EEPROM_Buffer) <= eepromAddress)
            {
                eepromAddress = 0;    // wrap to start of eeprom page
            }
            break;

        case I2C1_SLAVE_READ_COMPLETED:
        default:;

    } // end switch(i2c_bus_state)

}
#endif

