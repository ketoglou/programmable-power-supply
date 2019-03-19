#include <pic18f25k50.h>
#include "USART.h"

//Initialize USART,must be called at the beggining of the program
void USART_Init(int baud_rate){
    //Configure RX/TX Pins
    TRISCbits.RC6 = 1; //TX
    TRISCbits.RC7 = 1; //RX
    ANSELCbits.ANSC6 = 0; //Clear analog
    ANSELCbits.ANSC7 = 0;
    
    rx_counter = 0;
    
    PMD0bits.UARTMD = 0; //Enable the module just for sure
    TXSTA1 = 0x24;
    Initialize_Baud_Rate(baud_rate);
    RCSTA1 = 0x90;
    
    //Initialize interrupts
    IPR1bits.TXIP = 1; //USART Transmit interrupt = high priority
    IPR1bits.RCIP = 1; //USART Receive interrupt = high priority
    PIE1bits.RCIE = 1; //Enable receive interrupt
    PIE1bits.TXIE = 0; //Disable transmit interrupt for now
}

//Initialize the baud rate for the EUSART module
int Initialize_Baud_Rate(unsigned char baud_rate){
    
    BAUDCON1 = 0x00;
    //If SYNC=0,BRG16=0,BRGH=1,else this function must change
    switch(baud_rate){
        case 1:
            SPBRG1 = 155; //19200 br,0,16% error
            break;
        case 2:
            SPBRG1 = 51; //57600 br,0,16% error
            break;
        case 3:
            SPBRG1 = 25; //115200,0,16% error
            break;
        default:
            return 0;
    }
    return 1;
}

int USART_SendByte(unsigned char byte){
    if(!PIE1bits.TXIE){
        tx_byte = byte;
        PIE1bits.TXIE = 1; //When we begin a transmission we make TXIE = 1,when we finish it we make TXIE=0(through interrupt routine)
        return 1;
    }
    return 0;
}

int USART_SendString(char *str,int size){
    int attempts =3000,i;
    while(!USART_SendByte('\r') && (attempts --)); //CR
    while(!USART_SendByte('\n') && (attempts --)); //LF
    for(i=0;i<size;i++){
        if(!USART_SendByte(*(str + i))){
            attempts --;
            i --; //send again the same byte
        }
        if(!attempts)
            return 0;
    }
    return 1;
}

char USART_ReceiveByte(){
    if(rx_counter > 0){
        rx_counter --;
        char temp_char = rx_buffer[0];
        for(int i =0;i<rx_counter;i++){
            rx_buffer[i] = rx_buffer[i+1];
        }
        return temp_char;
    }
    return 0; //NULL
}

//buffer size must be at least equal to int size
int USART_ReceiveString(char *buffer,int size){
    if(rx_counter < size){
        return -1;
    }
    for(int i=0;i<size;i++){
        buffer[i] = USART_ReceiveByte();
    }
    return 1;
}


