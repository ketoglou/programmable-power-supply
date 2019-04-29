/*
 * File:   main.c
 * Author: iqsoft
 *
 * Created on April 29, 2019, 7:09 PM
 */

//Includes
#include <xc.h>
#include <stdio.h>
#include "config.h"
#include "ISR.h"
#include "USART1.h"

//Basic definitions

//For true false
typedef enum _BOOL { FALSE = 0, TRUE = 1 } boolean; 
//For easier implementations
#define byte unsigned char
#define BAUD_RATE 1//BAUD RATE --> 1 = 19200 , 2 = 57600 , 3 = 115200
#define VOLTAGE_PIN 12
#define CURRENT_PIN 13

// This code is meant to run on a PIC running at 64 MHz.
#define _XTAL_FREQ 64000000

//Define functions
void timer0_init(void);
void USART_handler(byte command);
void ADC_Init(void);
void ADC_Start(byte pin);
void ADC_GetResult(void);
int GetStringSize(void);
void memset(char *st,char x,int size);

//Define variables
byte counter_timer0;
float ADC_VOLTAGE_RESULT;

//---------------------------Interrupt Routines---------------------------------

void __interrupt(irq(IRQ_TMR0)) timer0_isr(void){
    T0CON0bits.EN = 0; //Disable Timer0
    counter_timer0 ++; //every +1 is +0.100msec
    if(counter_timer0 == 2){ 
        LATAbits.LA0 = 0;
    }else if(counter_timer0 == 40){
        LATAbits.LA0 = 1;
        counter_timer0 = 0;
    }
    TMR0L = 0xB0;
    TMR0H = 0x3C;
    ADC_Start(VOLTAGE_PIN);
    PIR3bits.TMR0IF = 0; //Clear interrupt flag
    T0CON0bits.EN = 1; //Enable Timer0
}

void __interrupt(irq(IRQ_U1TX)) uart1_tx_isr(void){
    //Transmit byte for USART
    U1TXB = tx_byte;
    PIE3bits.U1TXIE = 0;
}

void __interrupt(irq(IRQ_U1RX)) uart1_rx_isr(void){
    rx_buffer[rx_counter] = U1RXB;
    rx_counter ++;
}

void __interrupt(irq(IRQ_AD)) adc_isr(void){
    int adc_result = ADRESL;
    adc_result = adc_result | (ADRESH <<8);
    ADC_VOLTAGE_RESULT = (float)adc_result * 0.00122; //5/4095
    PIR1bits.ADIF = 0; //Clear interrupt flag
}

void __interrupt(irq(default)) default_isr(void){
    //Unhandled interrupts go here
}

void main(void) {
    OSCFRQ = 0x08; //64MHz Clock defined by pragma RSTOSC,just write for sure
    OSCTUNE = 0x00; //Stable center frequency
    OSCEN = 0x40;
    while(!OSCSTATbits.HFOR && !OSCSTATbits.ADOR);
    
    //IVTBASE* Registers must change if we use multiple IVTs(bootloader for instance)
    /*IVTBASEU = 0x00;
    IVTBASEH = 0x00;
    IVTBASEL = 0x00;*/
    IVTLOCK = 0x01; //IVTBASE Registers are locked and cannot be written(IVTWAY=1)
    
    //WatchDog Configuration,no Window 
    WDTCON1 = 0x07; //LFINTOSC used,Windows is open at 100% of time
    WDTCON0 = 0x3F; //32 seconds watchdog time,Enable watchdog
    
    //PPS Configuration
    PPSLOCKbits.PPSLOCKED = 0; //PPS selections can change (this bit cannot change because of pragma PPS1WAY=1)
    
    //Initialize modules
    timer0_init();
    USART1_Init(BAUD_RATE);
    ADC_Init();
    
    INTCON0bits.GIEH = 1; //Enable high priority interrupts
    INTCON0bits.GIEL = 1; //Enable low priority interrupts
    INTCON0bits.IPEN = 1; //Enable interrupt priority
    
    //Example LED
    TRISAbits.TRISA0 = 0;
    ANSELAbits.ANSELA0 = 1;
    //Buzzer
    TRISAbits.TRISA1 = 0;
    ANSELAbits.ANSELA1 = 1;
    LATAbits.LA1 = 0;
    
    byte command = 0;
    
    while(1){
        command = USART1_ReceiveCommand();
        USART_handler(command);
    }
}

void timer0_init(void){
    T0CON0 = 0x10;
    T0CON1 = 0x75;
    TMR0L = 0xB0;
    TMR0H = 0x3C;
    counter_timer0 = 0;
    IPR3bits.TMR0IP = 0; //Low priority interrupt
    PIR3bits.TMR0IF = 0; //Clear interrupt flag
    PIE3bits.TMR0IE = 1; //Enable interrupt
    T0CON0bits.EN = 1;
}

void USART_handler(byte command){
    //
    switch(command){
        case 0:
            break;
        case 1: //Voltage status
            memset(tx_buffer,0,TRANSMIT_BUFFER_SIZE);
            sprintf(tx_buffer,"Voltage:%f",ADC_VOLTAGE_RESULT);
            USART1_SendString(tx_buffer,GetStringSize());
            break;
        case 2:
            break;
        default:
            USART1_SendString("Command not recognized!",23);
            break;
    }
    command = 0;
}

//------------------------ADC Functions-----------------------------------------

void ADC_Init(void){
    //Voltage measure
    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSELB4 = 1;
    ADCON0 = 0x94;
    ADCON1 = 0x00; //-------
    ADCON2 = 0x00; //------
    ADREF = 0x00; //VDD,VSS as references
    IPR1bits.ADIP = 0; //Low priority
    PIE1bits.ADIE = 1; //Enable interrupt
    PIR1bits.ADIF = 0; //Clear interrupt flag
}

//0-7 =RA0-RA7,8-15=RB0-RB7,16-23=RC0-RC7
void ADC_Start(byte pin){
    ADPCH = pin;
    ADCON0bits.GO = 1;
}


//-----------------Help Functions-----------------------------------------------

int GetStringSize(void){
    int i;
    for(i=0;i<TRANSMIT_BUFFER_SIZE;i++){
        if(tx_buffer[i] == '\0'){
            break;
        }
    }
    return i;
}

void memset(char *st,char x,int size){
    for(int i=0;i<size;i++){
        st[i] = x;
    }
}