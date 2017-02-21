#include "usart.h"

//Private variables
uint8_t usart1_buffer[USART1_BUFFER_SIZE];
uint16_t usart1_index; //Circular buffer write index
uint16_t usart1_outex; //Circular buffer read index
uint16_t usart1_size; //Amount of data in the buffer
UART_HandleTypeDef UartHandle;


//Private functions
void Error_Handler();


void CW_USART1_Init()
{
	/*##-1- Configure the UART peripheral ######################################*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART1 configured as follow:
	   - Word Length = 8 Bits
	   - Stop Bit = One Stop bit
	   - Parity = None
	   - BaudRate = 9600 baud
	   - Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance          = USART1;

	UartHandle.Init.BaudRate     = 9600;
	UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits     = UART_STOPBITS_1;
	UartHandle.Init.Parity       = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode         = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}
	usart1_index = 0;
	usart1_outex = 0;
	usart1_size = 0;

	USART1->CR1 |= USART_CR1_RXNEIE;
}
uint16_t USART1_NewData()
{
	return usart1_size;
}

/* 
 * Copies the data in the buffer into dest upto a newline
 * returns number of bytes copied(including \n and \0
 */

uint16_t CW_USART1_GetLine(uint8_t* dest, uint16_t size)
{
	uint16_t nl = CW_USART1_CountChar((uint8_t)'\n');
	uint16_t copied = 0;
	if (nl)
	{
		size--;
		while(size--)
		{
			uint8_t c = usart1_buffer[usart1_outex];
			*dest = c;
			copied++;
			*dest++;
			usart1_outex++;
			if (usart1_outex == USART1_BUFFER_SIZE)
				usart1_outex = 0;
			if (c == '\n')
				break;
		}
		*dest = '\0';
		return copied+1;

	}
	else
		return 0;
}

/* 
 * Copy the data in the buffer to dest limited upto size
 * Returns number of bytes copied
 */
uint16_t CW_USART1_GetData(uint8_t* dest, uint16_t size)
{
	uint16_t copied = 0;	
	while(size--)
	{
		if (usart1_outex == usart1_index)
			break;

		*dest = usart1_buffer[usart1_outex];
		usart1_size--;
		usart1_outex++;
		if (usart1_outex == USART1_BUFFER_SIZE)
			usart1_outex = 0;
		copied++;
	}
	return copied;
}
uint16_t CW_USART1_CountChar(uint8_t c)
{
	uint16_t count = 0;
	uint16_t i = usart1_outex;
	while( i != usart1_index)
	{

		if (usart1_buffer[i] == c)
				count++;
		i++;
		if (i == USART1_BUFFER_SIZE)
			i = 0;
	}
	return count;
}

void CW_USART1_SendStr(uint8_t* str)
{
	while(*str)
	{
		CW_USART1_SendChar(*str);
		str++;
	}
}

void CW_USART1_SendChar(uint8_t c)
{
	USART1->SR &= ~USART_FLAG_TC;

	USART1->DR = c;

	while(!(USART1->SR & USART_FLAG_TC));
	USART1->SR &= ~USART_FLAG_TC;
}

void CW_USART1_IRQHandler(UART_HandleTypeDef* handle)
{
	uint8_t flag;

	volatile unsigned int IIR;

	IIR = USART1->SR;
	if (IIR & USART_FLAG_RXNE)                   // read interrupt
	{
		usart1_buffer[usart1_index++] = UartHandle.Instance->DR;
		if (usart1_index == USART1_BUFFER_SIZE)
			usart1_index = 0;
		usart1_size++;
		USART1->SR &= ~USART_FLAG_RXNE;             // clear interrupt
	}
	
	if (IIR & USART_FLAG_TXE )  //TX register empty 
	{
		USART1->CR1 &= ~(USART_CR1_TXEIE);
	}
	if (IIR & USART_FLAG_NE)
		flag = 1;
	if (IIR & USART_FLAG_FE)
		flag = 2;
	if (IIR & USART_FLAG_PE)
		flag = 2;


	return;



	/* ------------------------------------------------------------ */
	/* Other USART1 interrupts handler can go here ...             */
}   

void Error_Handler()
{

	BSP_LED_On(LED4);
	while(1);
}

