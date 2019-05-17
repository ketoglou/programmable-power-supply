#include <xc.h>
#include "I2C.h"

void I2C_Init(void){
    //Configure SDA,SCL Pins
    TRISBbits.TRISB1 = 0; //SCL
    TRISBbits.TRISB0 = 0; //SDA
    LATBbits.LATB1 = 0; // Clear PORTB write latches
    LATBbits.LATB0 = 0;
    ANSELBbits.ANSELB1 = 0; //RB1 clear analog
    ANSELBbits.ANSELB0 = 0; //RB2 clear analog
    ODCONBbits.ODCB1 = 1; //RB1 as open-drain
    ODCONBbits.ODCB0 = 1; //RB2 as open-drain
    RB1I2C = 0x01;// Standard GPIO slew rate,Internal pull-ups disabled,I2C specific thresholds
    SLRCONBbits.SLRB1 = 0; // No slew rate limiting
    SLRCONBbits.SLRB0 = 0;
    I2C1SCLPPS = 0x09; //RB1 PPS Input
    I2C1SDAPPS = 0x08; //RB2 PPS Input
    RB1PPS = 0x21; //RB1 PPS Output
    RB0PPS = 0x22; //RB2 PPS Output
    
    I2C1CON0 = 0x04;
    I2C1CON1 = 0x80;
    I2C1CON2 = 0x24; //I2C Clock = I2C1CLK(500kHz)/4 = 125kHz
    I2C1CLK = 0x03; //MFINTOSC as clock (500kHz)
           
    I2C1PIR = 0x00;
    I2C1ERR = 0x00; 
    
    I2C_STOP_DETECTED = 1;
    
    IPR3bits.I2C1TXIP = 0; //Transmit interrupt low priority
    IPR2bits.I2C1RXIP = 0; //Receive interrupt low priority
    PIR3bits.I2C1TXIF = 0; //Transmit interrupt flag clear
    PIR2bits.I2C1RXIF = 0; //Receive interrupt flag clear
    PIE3bits.I2C1TXIE = 1; //Transmit interrupt enable
    PIE2bits.I2C1RXIE = 1; //Receive interrupt enable
    
    //General interrupts
    I2C1PIRbits.PC1IF = 0;//Clear STOP interrupt flag
    I2C1PIEbits.PC1IE = 1; //Enable STOP interrupt
    IPR3bits.I2C1IP = 0; //General interrupt low priority
    PIR3bits.I2C1IF = 0; //General interrupt flag clear
    PIE3bits.I2C1IE = 1; //General interrupt enable
    
    I2C1CON0bits.EN = 1; //Enable I2C module
}

unsigned char I2C_Transmit(unsigned char *buffer,unsigned char buffer_size,unsigned char address){
    if(I2C_STOP_DETECTED && I2C1STAT0bits.BFRE && I2C1CNT == 0){
        I2C_STOP_DETECTED = 0;
        for(unsigned char i=0;i<(buffer_size-1);i++){
            I2C_TX_BUFFER[i] =  buffer[i+1];
        }
        I2C1ADB1 = address;
        I2C1CNT = buffer_size;
        I2C1TXB = buffer[0];
        I2C_TX_COUNTER = 0;
        I2C1CON0bits.S = 1;
        return 1;
    }
    return 0;
}

/*unsigned char I2C_Receive(unsigned char buffer_size,unsigned char address){
    //We check i2c_rx_counter to make sure that previous data was read 
    if(I2C_STOP_DETECTED && I2C1STAT0bits.BFRE){
        I2C_STOP_DETECTED = 0;
        I2C1ADB1 = (address |0x01);
        I2C1CNT = buffer_size;
        I2C_RX_COUNTER =  0;
        I2C1CON0bits.S = 1;
        return 1;
    }
    return 0;
}

unsigned char I2C_Receive_Ready(unsigned char *results,unsigned char results_size){
    if(results_size == I2C_RX_COUNTER){
        for(unsigned char i=0;i<I2C_RX_COUNTER;i++){
            results[i] = I2C_RX_BUFFER[i];
        }
        I2C_RX_COUNTER = 0;
        return 0xFF;
    }
    return I2C_RX_COUNTER;
}*/

void I2C_handler(int value){
    AD5272_COMMANDS[0] = (byte)(0x04 | (value >> 8));
    AD5272_COMMANDS[1] = (byte)value;
    I2C_Transmit(AD5272_COMMANDS,2,AD5272_VOLTAGE_ADDRESS);
    while(!I2C_STOP_DETECTED);
}
