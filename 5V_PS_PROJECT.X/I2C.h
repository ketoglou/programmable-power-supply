#ifndef I2C_H
#define	I2C_H

//Compute I2C Bitrate (frequency),if we change Fclock we must clear SMP bit at SSPSTAT register
#define Fosc 48000000 //System oscillator is at 48MHz
#define Fclock 100000 //Desired I2C clock frequency is 100 kHz
#define SSPADD_VALUE ((Fosc/(4*Fclock))-1) //Bitrate that will assigned to SSPADD register
#define I2C_IDLE_STATE (!SSP1STATbits.R_nW || !(SSP1CON2 & 0x1F)) //IDLE STATE FOR I2C

#define I2C_RX_BUFFER_SIZE 5
#define I2C_TX_BUFFER_SIZE 5

#define I2C_IDLE 0
#define I2C_TRANSMIT 1
#define I2C_FINISH_TRANSMIT 2
#define I2C_ERROR 3
unsigned char I2C_STATUS; 

int I2C_rx_counter,I2C_tx_counter;
char I2C_rx_buffer[I2C_RX_BUFFER_SIZE];
char I2C_tx_buffer[I2C_TX_BUFFER_SIZE];

//I2C Functions
int I2C_Transmit(int number_of_bytes_to_send);
void I2C_Init();
void I2C_buffer_move_queue();

#endif	/* I2C_H */

