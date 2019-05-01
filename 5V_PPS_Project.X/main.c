/*
 * Lesson:Microprocessors
 * Project:5V Adjustable CV,CC Power Supply
 * Authors: X.Ketoglou,V.Kontotoli
 * Professor:V.Tenentes
 *
 * Manual:
 * Use UART at 19200 bps (or change value of BAUD_RATE)
 * Commands are for reading and writing.
 * All commands append in the end with CR/LF.
 * Read commands are Rx\r\n where x is 0-9 and determines the command type.
 * Write commands are Wxabcd\r\n where x,a,b,c,d is 0-9,x determines the
 * command type and a,b,c,d are number needed to write.
 * 
 * Read Commands:
 * R0\r\n : Read voltage.A string with the value of voltage is send back.
 * R1\r\n : Read current.A string with the value of current is send back.
 * 
 * Write Commands:
 * W0yyyy : This command turn blinking LED ON OR OFF.The values of y's doesn't matter.
 * W1abcd : This command set the AD5272 variable resistor accross to the a,b,c,d
 *          numbers.This has an effect to the voltage output.
 * W2abcd : This command set the other AD5272 variable resistor accross to the 
 *          a,b,c,d numbers.This has an effect to the current limit at the output.
 */

//Includes
#include <xc.h>
#include <stdio.h>
#include "config.h"
#include "USART1.h"
#include "I2C.h"

//Basic definitions

//For true false
typedef enum _BOOL { FALSE = 0, TRUE = 1 } boolean; 
//For easier implementations
#define byte unsigned char
#define BAUD_RATE 1 //BAUD RATE --> 1 = 19200 , 2 = 57600 , 3 = 115200
#define VOLTAGE_PIN 12
#define CURRENT_PIN 13

// This code is meant to run on a PIC running at 64 MHz.
#define _XTAL_FREQ 64000000

//Define functions
void timer0_init(void);
void USART_handler(void);
void ADC_Init(void);
void ADC_Start(byte pin);
void ADC_GetResult(void);
int GetStringSize(void);
void memset(char *st,char x,int size);

//Define variables
byte counter_timer0;
byte led_enable;
float ADC_VOLTAGE_RESULT;

//---------------------------Interrupt Routines---------------------------------

void __interrupt(irq(IRQ_TMR0)) TIMER0_ISR(void){
    T0CON0bits.EN = 0; //Disable Timer0
    counter_timer0 ++; //every +1 is +0.100msec
    if(counter_timer0 == 2){ 
        LATAbits.LA0 = 0;
    }else if(counter_timer0 == 40){
        if(led_enable)
            LATAbits.LA0 = 1;
        counter_timer0 = 0;
    }
    TMR0L = 0xB0;
    TMR0H = 0x3C;
    ADC_Start(VOLTAGE_PIN);
    PIR3bits.TMR0IF = 0; //Clear interrupt flag
    T0CON0bits.EN = 1; //Enable Timer0
}

void __interrupt(irq(IRQ_U1TX)) UART1_TX_ISR(void){
    //Transmit byte for USART
    U1TXB = tx_byte;
    PIE3bits.U1TXIE = 0;
}

void __interrupt(irq(IRQ_U1RX)) UART1_RX_ISR(void){
    rx_buffer[rx_counter] = U1RXB;
    rx_counter ++;
}

void __interrupt(irq(IRQ_AD)) ADC_ISR(void){
    int adc_result = ADRESL;
    adc_result = adc_result | (ADRESH <<8);
    ADC_VOLTAGE_RESULT = (float)adc_result * 0.00122; //5/4095
    PIR1bits.ADIF = 0; //Clear interrupt flag
}

void __interrupt(irq(IRQ_I2C1TX)) I2C_TX_ISR(void){
    I2C1TXB = I2C_TX_BUFFER[I2C_TX_COUNTER];
    I2C_TX_COUNTER ++;
}
void __interrupt(irq(IRQ_I2C1RX)) I2C_RX_ISR(void){
    I2C_RX_BUFFER[I2C_RX_COUNTER] = I2C1RXB;
    I2C_RX_COUNTER ++;
}

void __interrupt(irq(IRQ_I2C1)) I2C_GENERAL_ISR(void){
    if(I2C1PIRbits.PC1IF)
        I2C_STOP_DETECTED = 1;
    I2C1PIR = 0x00;
}

void __interrupt(irq(default)) DEFAULT_ISR(void){
    //Unhandled interrupts go here
}

//------------------------main--------------------------------------------------

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
    I2C_Init();
    
    INTCON0bits.GIEH = 1; //Enable high priority interrupts
    INTCON0bits.GIEL = 1; //Enable low priority interrupts
    INTCON0bits.IPEN = 1; //Enable interrupt priority
    
    //AD5272 Initialize
    AD5272_COMMANDS[0] = I2C_CONTROL_WRITE;
    AD5272_COMMANDS[1] = I2C_CONTROL_DATA;
    I2C_Transmit(AD5272_COMMANDS,2,AD5272_VOLTAGE_ADDRESS);
    while(!I2C_STOP_DETECTED);
    I2C_Transmit(AD5272_COMMANDS,2,AD5272_CURRENT_ADDRESS);
    while(!I2C_STOP_DETECTED);
    AD5272_COMMANDS[0] = I2C_RDAC_WRITE;
    
    //Example LED
    TRISAbits.TRISA0 = 0;
    ANSELAbits.ANSELA0 = 1;
    led_enable = 1;
    //Buzzer
    TRISAbits.TRISA1 = 0;
    ANSELAbits.ANSELA1 = 1;
    LATAbits.LA1 = 0;
    //AD5272 RESET DISABLE
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 0;
    LATBbits.LB2 = 1;
    LATBbits.LB3 = 1;
    
    byte receive_command;
    
    while(1){
        receive_command = USART1_ReceiveCommand();
        if(receive_command)
            USART_handler();
    }
}

//--------------------------Basic functions-------------------------------------

void USART_handler(void){
    memset(tx_buffer,0,TRANSMIT_BUFFER_SIZE);
    if(COMMAND_WR){ //READ COMMAND
        switch(COMMAND){
            case 0: //Voltage status
                sprintf(tx_buffer,"Voltage:%f V",ADC_VOLTAGE_RESULT);
                break;
            case 1:
                sprintf(tx_buffer,"Current:%f A",ADC_VOLTAGE_RESULT);
                break;
            default:
                sprintf(tx_buffer,"Command not recognized!");
                break;
        }
    }else{  //WRITE COMMAND
        switch(COMMAND){
            case 0:
                led_enable = !led_enable;
                if(led_enable)
                    sprintf(tx_buffer,"Blinking LED is ON!");
                else
                    sprintf(tx_buffer,"Blinking LED is OFF!");
                break;
            case 1:
                I2C_handler(I2C_AD5272_VOLTAGE_SET,COMMAND_WRITE_NUMBER);
                sprintf(tx_buffer,"Voltage seted!");
                break;
            case 2:
                I2C_handler(I2C_AD5272_CURRENT_SET,COMMAND_WRITE_NUMBER);
                sprintf(tx_buffer,"Current Limit seted!");
                break;
            default:
                sprintf(tx_buffer,"Command not recognized!");
                break;
        }
    }
    USART1_SendString(tx_buffer,GetStringSize());
}

//-------------------Timers initialize------------------------------------------

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
//------------------------ADC Functions-----------------------------------------

void ADC_Init(void){
    //Voltage measure
    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSELB4 = 1;
    ADCON0 = 0x94;
    ADCON1 = 0x00;
    ADCON2 = 0x00;
    ADREF = 0x00; //VDD,VSS as references
    IPR1bits.ADIP = 0; //Low priority
    PIE1bits.ADIE = 1; //Enable interrupt
    PIR1bits.ADIF = 0; //Clear interrupt flag
}

//0-7 =RA0-RA7,8-15=RB0-RB7,16-23=RC0-RC7
void ADC_Start(byte pin){
    if(!ADCON0bits.GO){
        ADPCH = pin;
        ADCON0bits.GO = 1;
    }
}


//-----------------Help Functions-----------------------------------------------

int GetStringSize(void){
    int i;
    for(i=0;i<TRANSMIT_BUFFER_SIZE;i++){
        if(tx_buffer[i] == 0){
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