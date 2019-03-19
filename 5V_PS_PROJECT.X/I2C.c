#include <xc.h>
#include "I2C.h"

void I2C_Init(){ 
    //Pins initialized
    ANSELBbits.ANSB0 = 0;  //Set the pin to digital (SDA)
    ANSELBbits.ANSB1 = 0;  //Set the pin to digital (SCL)
    TRISBbits.RB0 = 1; //Set pin as input (SDA) 
    TRISBbits.RB1 = 1; //Set pin as input (SCL)
    
    PMD1bits.MSSPMD = 0; //Enable the module(just for sure)
        
    //Interrupt initialize
    IPR1bits.SSPIP = 0;  //Low priority for MSSP Interrupt flag
    IPR2bits.BCLIP = 0;  //Low priority for Bus Collision Interrupt flag
    PIE1bits.SSPIE = 1;  //Enable MSSP Interrupt flag
    PIE2bits.BCLIE = 1;  //Enable Bus Collision Interrupt flag
    PIR1bits.SSPIF = 0;  //Clear MSSP Interrupt flag
    PIR2bits.BCLIF = 0;  //CLear Bus Collision Interrupt flag
    
    //Registers initialize
    SSP1ADD = SSPADD_VALUE;  //Set I2C Baud Rate
    SSP1CON2 = 0x00; 
    SSP1CON3 = 0x64; //Enable Start,Stop,Bus collision interrupt
    SSP1STAT = 0x80;
    SSP1CON1 = 0x28;
}

int I2C_Transmit(int number_of_bytes_to_send){
    //If I2C is not idle return 0
    if(!I2C_IDLE_STATE)
        return 0;
    I2C_STATUS = I2C_IDLE;
    I2C_tx_counter = number_of_bytes_to_send;
    SSP1CON2bits.SEN = 1; //Set Start condition
    return 1;
}

//Move the queue 1 element forward
void I2C_buffer_move_queue(){
    int i;
    for(i=0;i<(I2C_TX_BUFFER_SIZE-1);i++){
        I2C_tx_buffer[i] = I2C_tx_buffer[i+1];
    }
}


