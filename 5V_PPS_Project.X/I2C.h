#define byte unsigned char
#define I2C_TX_BUFFER_SIZE 10
#define I2C_RX_BUFFER_SIZE 10

//I2C DEVICES DEFINITIONS
#define I2C_CONTROL_WRITE 0x1C
#define I2C_CONTROL_DATA 0x02
#define I2C_RDAC_WRITE 0x04

//I2C COMMUNICATION VARIABLES
byte I2C_TX_COUNTER;
byte I2C_RX_COUNTER;
byte I2C_TX_BUFFER[I2C_TX_BUFFER_SIZE];
byte I2C_RX_BUFFER[I2C_RX_BUFFER_SIZE];
byte I2C_STOP_DETECTED;

//I2C COMMANDS FOR DEVICES
byte AD5272_VOLTAGE_ADDRESS = 0x5E;
byte AD5272_COMMANDS[2] = {0x00,0x00};
        
// Call it to initialize the module with interrupts
void I2C_Init(void);

/* buffer has maximum I2C_BUFFER_SIZE size and contains the commands that will be send,
 * buffer_size is the number of commands in buffer and address is the address to be transmitted.
 * This function return 1 if everything goes well or 0 if I2C module is busy.
 */
unsigned char I2C_Transmit(unsigned char *buffer,unsigned char buffer_size,unsigned char address);

// This function returns 1 if the I2C module setted for receive else returns 0.
//unsigned char I2C_Receive(unsigned char buffer_size,unsigned char address);

/* Returns 0xFF is Receive is done and save the received chars in the *results.
 * If results_size > received_bytes then it returns number of received_bytes
 */ 
//unsigned char I2C_Receive_Ready(unsigned char *results,unsigned char results_size);

//This function must called inside the main while() and must be written by user
void I2C_handler(int value);
