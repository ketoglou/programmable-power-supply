/*
 * File:   main.c
 * Author: Ketoglou Xaris,Kontotoli Voula
 */


#include <xc.h>
#include "config.h"
#include "USART.h"
#include "I2C.h"

#define _XTAL_FREQ 48000000
#define BAUD_RATE 3

void timer0_init();
void ADC_Init();
void ADC_start();
int getADC_result();
void ADC_change_channel(unsigned char channel);

boolean ADC_ready = FALSE;

//High priority interrupt handler function
void  __interrupt(high_priority) high_isr(void){
    //Transmit byte for USART
    if(PIE1bits.TXIE && PIR1bits.TXIF && TXSTA1bits.TRMT){
        TXREG1 = tx_byte;
        PIE1bits.TXIE = 0;
    }
    //Receive byte for USART
    if(PIR1bits.RCIF){
        if(!RCSTA1bits.FERR){
            if(RCSTA1bits.OERR)
                RCSTA1bits.OERR = 0;
            rx_buffer[rx_counter] = RCREG1;
            rx_counter ++;
            if(rx_counter == (RECEIVE_BUFFER_SIZE+1)){
				//In situation like this many things can be done,i prefer reset USART
                USART_Init(BAUD_RATE);
            }         
        }else{
            RCSTA1bits.SPEN = 0;
            USART_Init(BAUD_RATE);
        }
    }
}

//Low priority interrupt handler function
void  __interrupt(low_priority) low_isr(void){
    //Start ADC
    if(PIR1bits.ADIF){
        ADC_ready = TRUE;
        PIR1bits.ADIF = 0;
    }
    //I2C Transmit Conditions handle
    if(PIR1bits.SSPIF){
        if(SSPSTATbits.S && !SSP1CON2bits.SEN){
            //Start bit detected last(First interrupt)
            if(I2C_STATUS == I2C_IDLE){
                if(!SSPCON1bits.WCOL && !SSPSTATbits.BF && I2C_IDLE_STATE){
                    SSPBUF = I2C_tx_buffer[0];
                    I2C_buffer_move_queue();
                    I2C_tx_counter --;
                    I2C_STATUS = I2C_TRANSMIT;
                }else{
                    I2C_STATUS = I2C_ERROR;
                }
            }else if(I2C_STATUS == I2C_TRANSMIT){
                //When we are in this it means that a byte shifted out
                if(!SSPCON2bits.ACKSTAT){  //ACK received
                    if(!I2C_tx_counter){
                        SSPCON2bits.PEN = 1;
                    }else{
                        SSPBUF = I2C_tx_buffer[0];
                        I2C_buffer_move_queue();
                        I2C_tx_counter --;
                    }
                }else{ //NACK received
                    I2C_STATUS = I2C_ERROR;
                }
            }
        }else if(SSPSTATbits.P && !SSP1CON2bits.PEN){
            I2C_STATUS = I2C_FINISH_TRANSMIT;
        }
        PIR1bits.SSPIF = 0;
    }
    
    //Timer0 handle
    if(INTCONbits.TMR0IF){
        //VOULA CODE GOES HERE
        INTCONbits.TMR0IF = 0;
    }
}

void main(void) {
    OSCCON = 0x70;
    OSCCON2 = 0x94;
    OSCTUNE = 0x80;
    while(!OSCCONbits.HFIOFS && !OSCCON2bits.HFIOFR && !OSCCON2bits.PLLRDY);
    
    USART_Init(BAUD_RATE);
    I2C_Init();
    timer0_init();
    
    //Buzzer
    TRISAbits.RA1 = 0;
    
    //Enable interrupts
    RCONbits.IPEN = 1; //set priority in interrupts
    INTCONbits.GIE = 1; //IPEN=1,enable high and low priority interrupts
    INTCONbits.PEIE = 1;

    while(1){
        //main code goes here
        LATAbits.LA0 = 1;
        LATAbits.LA1 = 1;
        __delay_ms(700);
        LATAbits.LA0 = 0;
        LATAbits.LA1 = 0;
        __delay_ms(700);
    }
    return;
}


/*Timer0 Initialize function*/
void timer0_init(){
    //Initialize code for Timer0
    TRISAbits.RA0 = 0; //Make RA0 output
    //VOULA CODE GOES HERE
}

//************ADC FUNCTIONS****************
void ADC_Init(){
    //Configure A/D port pins(need to change and ADCON0 if change pin)
    TRISAbits.RA0 = 1;
    ANSELAbits.ANSA0 = 1;
    TRISAbits.RA1 = 1;
    ANSELAbits.ANSA1 = 1;
    
    PMD1bits.ADCMD = 0; //just to make sure that module is enabled
    
    //Enable interrupts
    PIE1bits.ADIE = 1; //enable ADC interrupt
    IPR1bits.ADIP = 0; //set low priority to ADC
    PIR1bits.ADIF = 0;
    
    ADCON1 = 0x00;               //TRIGSEL= SPECIAL TRIGGER SELECT BIT
    ADCON2 = 0x1E;         
    ADCON0 = 0x01;
}

void ADC_start(){
    if(!ADCON0bits.GO)
        ADCON0bits.GO = 1;
}

int GetADC_result(){
    if(ADC_ready){
        int result = (ADRESH << 2) | (ADRESL >> 6) ;
        ADC_ready = FALSE;
        return result;
    }
    return -1;
}

//Values from 0-27(analog channels),28=temperature diode,29=ctmu,30=DAC,31=FVR BUF2
void ADC_change_channel(unsigned char channel){
    ADCON0bits.GO = 0; //stop read if you read previous channel
    ADCON0bits.CHS = channel;
}
