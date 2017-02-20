#include "usart.h"

//Private variables
uint8_t usart1_buffer[USART1_BUFFER_SIZE];
uint16_t usart1_index; //Circular buffer write index
uint16_t usart1_outex; //Circular buffer read index
uint16_t usart1_size; //Amount of data in the buffer
UART_HandleTypeDef UartHandle;


//Private functions
void Error_Handler();

#define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

#define __USART_SETUP             1                       //  0
#define __USART_USED              0x01                    //  1
#define __USART_DETAILS           0x00                    //  2
#define __USART_INTERRUPTS        0x01                    //  3
#define __USART1_BAUDRATE         115200                  //  4
#define __USART1_DATABITS         0x00000000
#define __USART1_STOPBITS         0x00000000
#define __USART1_PARITY           0x00000000
#define __USART1_FLOWCTRL         0x00000000
#define __USART1_REMAP            0x00000000
#define __USART1_CR1              0x000000A0
#define __USART1_CR2              0x00000000
#define __USART1_CR3              0x00000000

 /*----------------------------------------------------------------------------
  Define SYSCLK
  *----------------------------------------------------------------------------*/
#if !defined  (HSE_VALUE) 
#define HSE_VALUE    ((uint32_t)8000000) /*!< Default value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
#define HSI_VALUE    ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALU*/

#define __HSI 16000000UL

 #define __PLLMULL (((__RCC_CFGR_VAL & CFGR_PLLMULL_MASK) >> 18) + 2)
 
 #if   ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x00) 
   #define __SYSCLK   __HSI                        // HSI is used as system clock
 #elif ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x01)
   #define __SYSCLK   __HSE                        // HSE is used as system clock
 #elif ((__RCC_CFGR_VAL & CFGR_SW_MASK) == 0x02)
   #if (__RCC_CFGR_VAL & CFGR_PLLSRC_MASK)         // HSE is PLL clock source
     #if (__RCC_CFGR_VAL & CFGR_PLLXTPRE_MASK)     // HSE/2 is used
       #define __SYSCLK  ((__HSE >> 1) * __PLLMULL)// SYSCLK = HSE/2 * pllmull
     #else                                         // HSE is used
       #define __SYSCLK  ((__HSE >> 0) * __PLLMULL)// SYSCLK = HSE   * pllmul
     #endif
   #else                                           // HSI/2 is PLL clock source
     #define __SYSCLK  ((__HSI >> 1) * __PLLMULL)  // SYSCLK = HSI/2 * pllmul
   #endif
 #else
    #error "ask for help"
 #endif

/*----------------------------------------------------------------------------
 *   Define  HCLK
*----------------------------------------------------------------------------*/
#define __HCLKPRESC  ((__RCC_CFGR_VAL & CFGR_HPRE_MASK) >> 4)
#if (__HCLKPRESC & 0x08)
#define __HCLK        (__SYSCLK >> ((__HCLKPRESC & 0x07)+1))
#else
#define __HCLK        (__SYSCLK)
#endif

#define __PCLK2PRESC  ((__RCC_CFGR_VAL & CFGR_PRE2_MASK) >> 11)
 #if (__PCLK2PRESC & 0x04)
   #define __PCLK2       (__HCLK >> ((__PCLK2PRESC & 0x03)+1))
 #else
   #define __PCLK2       (__HCLK)
 #endif

 /*----------------------------------------------------------------------------
  Define  Baudrate setting (BRR) for USART1 
  *----------------------------------------------------------------------------*/
 #define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
 #define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
 #define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
 #define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))
 

void CW_USART1_Init2()
{

	BSP_LED_On(LED3);
	HAL_Delay(200);
	BSP_LED_Off(LED3);
	HAL_Delay(1000);

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
/*
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
	AFIO->MAPR   &= ~(1 << 2);                              // clear USART1 remap
	if      ((__USART1_REMAP & 0x04) == 0x00) {             // USART1 no remap
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;                   // enable clock for GPIOA
		GPIOA->CRH   &= ~(0xFFUL  << 4);                      // Clear PA9, PA10
		GPIOA->CRH   |=  (0x0BUL  << 4);                      // USART1 Tx (PA9)  alternate output push-pull
		GPIOA->CRH   |=  (0x04UL  << 8);                      // USART1 Rx (PA10) input floating
	}
	else {                                                  // USART1    remap
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                   // enable clock for Alternate Function
		AFIO->MAPR   |= __USART1_REMAP;                       // set   USART1 remap
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;                   // enable clock for GPIOB
		GPIOB->CRL   &= ~(0xFFUL  << 24);                     // Clear PB6, PB7
		GPIOB->CRL   |=  (0x0BUL  << 24);                     // USART1 Tx (PB6)  alternate output push-pull
		GPIOB->CRL   |=  (0x04UL  << 28);                     // USART1 Rx (PB7) input floating
	}
*/
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;                   // enable clock for USART1

	USART1->BRR  = __USART_BRR(__PCLK2, __USART1_BAUDRATE); // set baudrate
	USART1->CR1  = __USART1_DATABITS;                       // set Data bits
	USART1->CR2  = __USART1_STOPBITS;                       // set Stop bits
	USART1->CR1 |= __USART1_PARITY;                         // set Parity
	USART1->CR3  = __USART1_FLOWCTRL;                       // Set Flow Control

	USART1->CR1 |= (USART_CR1_RE );// USART_CR1_TE);           // RX, TX enable

	if (__USART_INTERRUPTS & 0x01) {                        // interrupts used
		USART1->CR1 |= __USART1_CR1;
		USART1->CR2 |= __USART1_CR2;
		USART1->CR3 |= __USART1_CR3;
		NVIC->ISER[1] |= (1 << (USART1_IRQn & 0x1F));   // enable interrupt
	}

	USART1->CR1 &= ~USART_CR1_TXEIE; // Disable tx interrupt
	USART1->CR1 |= USART_CR1_UE;                            // USART enable


	usart1_index = 0;
	usart1_outex = 0;
	usart1_size = 0;
}


void CW_USART1_Init()
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



	/* In USART mode, the following bits must be kept cleared: 
	 *       - LINEN bit in the USART_CR2 register
	 *             - HDSEL, SCEN and IREN bits in the USART_CR3 register */
	UartHandle.Instance->CR2 &= ~USART_CR2_LINEN;
	UartHandle.Instance->CR3 &= ~(USART_CR3_IREN | USART_CR3_SCEN | USART_CR3_HDSEL);
	 /* Enable the Peripheral */
   __HAL_USART_ENABLE(&UartHandle);


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
		if (usart1_outex == usart1_index)
			break;

		*dest = usart1_buffer[usart1_outex];
		usart1_size--;
		usart1_outex++;
		copied++;
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

void CW_USART1_IRQHandler(UART_HandleTypeDef* handle)
{


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
	
	if (IIR & USART_FLAG_TXE && (USART1->CR1 & USART_CR1_TXEIE))  //TX register empty 
	{
		USART1->CR1 &= ~USART_CR1_TXEIE;
	}

	return;



	/* ------------------------------------------------------------ */
	/* Other USART1 interrupts handler can go here ...             */
}   


void Error_Handler()
{

	BSP_LED_On(LED4);
	while(1);
}

