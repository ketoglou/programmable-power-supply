/*USART HEADER FILE
 * Use USART_ReceiveByte for receive byte and  USART_SendString for send bytes
 * if USART is not use receive or transmit functions static must be removed from the one that is not used
 */
#define RECEIVE_BUFFER_SIZE 32
#define TRANSMIT_BUFFER_SIZE 32

char rx_counter;
char rx_buffer[RECEIVE_BUFFER_SIZE];
char tx_buffer[TRANSMIT_BUFFER_SIZE];
char tx_byte;


void USART1_Init(unsigned char baud_rate);
unsigned char USART1_SendByte(unsigned char byte);
unsigned char USART1_SendString(char *str,int size);
unsigned char USART1_ReceiveCommand(void);