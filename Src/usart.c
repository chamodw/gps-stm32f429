#include "usart.h"

//Private variables
uint8_t usart1_buffer[USART1_BUFFER_SIZE];
uint16_t usart1_index; //Circular buffer write index
uint16_t usart1_outex; //Circular buffer read index
uint16_t usart1_size; //Amount of data in the buffer
UART_HandleTypeDef UartHandle;


//Private functions
void Error_Handler();


void USART1_Init()
{
	UartHandle.Instance          = USART1;

	UartHandle.Init.BaudRate     = 9600;
	UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits     = UART_STOPBITS_1;
	UartHandle.Init.Parity       = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode         = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* Enable USART1 clock */
	__HAL_RCC_USART1_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/  
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_9;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* UART RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*##-3- Configure the NVIC for UART ########################################*/
	/* NVIC for USART1 */
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 1); 
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}

}
uint16_t USART1_NewData()
{
	return usart1_size;
}

uint16_t USART1_GetData(uint8_t* dest, uint16_t size)
{
	uint16_t copied = 0;	
	while(size--)
	{
		*dest = usart1_buffer[usart1_outex];
		dest++;
		copied++;
		if (usart1_outex == usart1_index)
			break;
	}
	return copied;
}
uint16_t USARR1_CountChar(char c)
{
	uint16_t count = 0;
	uint16_t i;
	for(i = usart1_outex; i < usart1_index; i++)
	{
		if (usart1_buffer[i] == c)
				count++;
		if (i == USART1_BUFFER_SIZE)
			i = 0;

	}
	return count;
}

void USART1_IRQHandler(void)
{
    // RXNE handler UART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	if(UartHandle.Instance->SR& USART_FLAG_RXNE  == USART_FLAG_RXNE)
	{
		/* If received 't', toggle LED and transmit 'T' */
		usart1_buffer[usart1_index++] = UartHandle.Instance->DR;
		if (usart1_index == USART1_BUFFER_SIZE)
			usart1_index = 0;
		usart1_size++;
		if (usart1_size == USART1_BUFFER_SIZE)
		{
			//TODO: Handle buffer overflow
		}


		//USART_SendData(USART1, 'T');
		/* Wait until Tx data register is empty, not really 
		 * required for this example but put in here anyway.
		 */
		/*
			 while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
			 {
			 }*/
	}
     
    /* ------------------------------------------------------------ */
    /* Other USART1 interrupts handler can go here ...             */
}   


void Error_Handler()
{

	BSP_LED_On(LED4);
	while(1);
}
