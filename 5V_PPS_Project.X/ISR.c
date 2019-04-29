#include "ISR.h"


//Interrupt routines

void __interrupt(irq(IRQ_TMR0)) timer0_isr(void){
    counter_timer0 ++; //every +1 is +0.13107sec
    if(counter_timer0 == 6){ //6*0.13107 = 0.7864
        LATAbits.LA0 = 1;
    }else if(counter_timer0 == 8){ //8*0.13107 = 1.04856
        LATAbits.LA0 = 0;
        counter_timer0 = 0;
    }
    PIR3bits.TMR0IF = 0; //Clear interrupt flag
}
