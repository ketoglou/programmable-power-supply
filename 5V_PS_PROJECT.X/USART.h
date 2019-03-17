/*USART HEADER FILE
 * Use USART_ReceiveByte for receive byte and  USART_SendString for send bytes
 * if USART is not use receive or transmit functions static must be removed from the one that is not used
 */
#define RECEIVE_BUFFER_SIZE 32

int rx_counter;
char rx_buffer[RECEIVE_BUFFER_SIZE];
volatile char tx_byte;

int Initialize_Baud_Rate(unsigned char baud_rate);
void USART_Init(int baud_rate);
int USART_SendByte(unsigned char byte);
char USART_ReceiveByte();
int USART_SendString(char *str,int size);
int USART_ReceiveString(char *buffer,int size);