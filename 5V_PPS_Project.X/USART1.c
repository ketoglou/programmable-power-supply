
#include "USART1.h"
#include <xc.h>

void USART1_Init(unsigned char baud_rate){
    //Configure RX/TX Pins
    TRISCbits.TRISC6 = 1; //RX
    TRISCbits.TRISC7 = 0; //TX
    ANSELCbits.ANSELC6 = 0; //Clear analog
    ANSELCbits.ANSELC7 = 0;
    U1RXPPS = 0x16; //Make RC6 as RX pin at UART1
    RC7PPS = 0x13; //Make RC7 as TX pin at UART1
    
    //BAUD RATE --> 1 = 19200 , 2 = 57600 , 3 = 115200
    switch(baud_rate){
        case 1:
            U1BRGH = 0x03;
            U1BRGL = 0x40;
            break;
        case 2:
            U1BRGH = 0x01;
            U1BRGL = 0x14;
            break;
        case 3:
            U1BRGH = 0x00;
            U1BRGL = 0x89;
            break;
        default:
            U1BRGH = 0x00;
            U1BRGL = 0x89;
            break;
    }
    
    rx_counter = 0;
    
    U1CON0 = 0xB0; //Enable TX,RX,High speed baud,8-bit asynchronous
    U1CON1 = 0x00; //No send break,tx,rx output driven by shift register,wake receive off
    U2CON2 = 0x80; //Continue receive if overflow,No inverted tx,rx polarities,1 stop bit,no checksum,no handshake
    U1ERRIE = 0x00; //If errors occur during reception,continue receive and handle them as  junk characters
    U1UIR = 0x00; //Clear general interrupt flags 
    PIE3bits.U1RXIE = 1; //Receive interrupt bit enable
    PIE3bits.U1TXIE =1; //Transmit interrupt bit enable
    IPR3bits.U1RXIP = 1; //Receive high priority
    IPR3bits.U1TXIP = 1; //Transmit high priority
    PIR3 = 0x00; //Clear interrupt flags
    U1CON1bits.ON = 1;
}

unsigned char USART1_SendByte(unsigned char byte){
    if(!PIE3bits.U1TXIE){ 
        tx_byte = byte;
        PIE3bits.U1TXIE = 1; //When we begin a transmission we make U1TXIE = 1,when we finish it we make U1TXIE=0(through interrupt routine)
        return 1;
    }
    return 0;
}

unsigned char USART1_SendString(char *str,int size){
    int attempts =3000,i;
    for(i=0;i<size;i++){
        if(!USART1_SendByte(*(str + i))){
            attempts --;
            i --; //send again the same byte
        }
        if(!attempts)
            return 0;
    }
    attempts =3000;
    while(!USART1_SendByte('\r') && (attempts --)); //CR
    while(!USART1_SendByte('\n') && (attempts --)); //LF
    return 1;
}


//All must be command Cxy\r\n where x = 0-9,y = 0-9
unsigned char USART1_ReceiveCommand(void){
    if(rx_counter == 0){
        return 0;
    }else if(rx_counter == 1 && rx_buffer[0] != 'C'){
        rx_counter = 0;
    }else if(rx_counter == 2 && ((rx_buffer[1] < 48) || (rx_buffer[1] > 57))){
        rx_counter = 0;
    }else if(rx_counter == 3 && ((rx_buffer[2] < 48) || (rx_buffer[2] > 57))){
        rx_counter = 0;
    }else if(rx_counter == 4 && rx_buffer[3] != '\r'){
        rx_counter = 0;
    }else if(rx_counter == 5){
        if(rx_buffer[0] != 'C' || (rx_buffer[1] < 48) || (rx_buffer[1] > 57) || (rx_buffer[2] < 48) || (rx_buffer[2] > 57) || rx_buffer[3] != '\r' || rx_buffer[4] != '\n'){
            rx_counter = 0;
            return 0;
        }
        unsigned char temp;
        temp = ((rx_buffer[1]-48) * 10) + (rx_buffer[2]-48);
        rx_counter = 0;
        return temp;
    }
    return 0;
}
