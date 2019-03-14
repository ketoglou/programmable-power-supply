/*
 * File:   main.c
 * Author: orion
 *
 * Created on February 28, 2019, 9:19 PM
 */


#include <xc.h>
#include "config.h"
#include "USART.h"

#define _XTAL_FREQ 48000000
#define BAUD_RATE 3

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

void main(void) {
    OSCCON = 0x70;
    OSCCON2 = 0x94;
    OSCTUNE = 0x80;
    while(!OSCCONbits.HFIOFS && !OSCCON2bits.HFIOFR && !OSCCON2bits.PLLRDY);
    
    USART_Init(BAUD_RATE);
    
    //Enable interrupts
    RCONbits.IPEN = 1; //set priority in interrupts
    INTCONbits.GIE = 1; //IPEN=1,enable high and low priority interrupts
    INTCONbits.PEIE = 1;
    
    TRISA = 0x00;
    ANSELA = 0;
    
    char okrec[7];
    while(1){
        //USART_SendString("voulaki\n",8);
        LATA = 0x01;
        __delay_ms(1000);
        LATA = 0x00;
        __delay_ms(1000);
        while (USART_ReceiveString(okrec,2) == -1);
        if (okrec[0] == 'o' && okrec[1] == 'k') 
            USART_SendString("voulaki\n",8);
        
    }
    return;
}
