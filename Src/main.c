/**
  ******************************************************************************
  * @file    BSP/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    13-March-2015
  * @brief   This example code shows how to use the STM32429I-Discovery BSP Drivers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stlogo.h"
#include "usart.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

 


static uint8_t DemoIndex = 0;
#ifdef EE_M24LR64
uint8_t NbLoop = 1;
#endif /* EE_M24LR64 */
__IO uint8_t ubKeyPressed = RESET; 

BSP_DemoTypedef BSP_examples[]=
{
  {MEMS_demo, "MEMS", 0}, 
};

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Display_DemoDescription(void);
static void Error_Handler(void);
void CW_MPU_Init();

uint8_t* CW_GPS_DecodeVTG(uint8_t* vtg_sntc);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{ 
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();
  
  /* Configure LED3 and LED4 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4); 
  
  /* Configure the system clock to 180 MHz */
  SystemClock_Config();
 

  /* Configure USER Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  //Initialize USART
  CW_USART1_Init();

//Initialize I2C
  CW_MPU_Init();

  I2C_GenerateSTART(I2C1, ENABLE);
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  

  /*##-1- Initialize the LCD #################################################*/
  /* Initialize the LCD */
  BSP_LCD_Init();

    /* Initialize the LCD Layers */
  BSP_LCD_LayerDefaultInit(1, LCD_FRAME_BUFFER);
  
  Display_DemoDescription();
  

  uint8_t temp_buf[100];
  uint8_t location = 0;
  /* Wait For User inputs */
  if(BSP_PB_GetState(BUTTON_KEY) == RESET)
  {
//	  while (BSP_PB_GetState(BUTTON_KEY) == RESET);


	  BSP_LCD_Clear(LCD_COLOR_BLUE);;
	  BSP_LCD_SetBackColor(LCD_COLOR_BLUE); 
	BSP_LCD_SetFont(&Font24);
	  uint16_t nlcount;

	  while(1)
	  {

		  while (1)
		  {
			  nlcount = CW_USART1_CountChar('\n');
			  if (nlcount > 0)
				  break;
		  }
		  uint16_t read_chars = CW_USART1_GetLine(temp_buf, 100);
		  if (read_chars > 0)
		  {
			//CW_USART1_SendStr(temp_buf);
			  if (strstr((const char*) temp_buf, "VTG"))
			  {
				  uint8_t* v_str= CW_GPS_DecodeVTG(temp_buf);
				  if (v_str)
				  {
					  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 , "                ", CENTER_MODE);
					  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 , v_str, CENTER_MODE);
				  }
				  else
					  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2, "NO DATA", CENTER_MODE);
			  }
		  }
	  }



  }
  return 0;
}


void CW_MPU_Init()
{
	I2C_InitTypeDef initI2c;
	 GPIO_InitTypeDef  GPIO_InitStruct;

	initI2c.ClockSpeed = 100000;
	initI2c.DutyCycle = I2C_DUTYCYCLE_2;
	initI2c.OwnAddress1 = 0;
	initI2c.AddressingMode = I2C_ADDRESSINGMODE_7BIT;

	I2C_HandleTypeDef hi2c;
	hi2c.Instance = I2C3;
	hi2c.Init = initI2c;

	/* Configure the GPIOs ---------------------------------------------------*/
	/* Enable GPIO clock */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure I2C Tx as alternate function  */
	GPIO_InitStruct.Pin       = GPIO_PIN_8;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = DISCOVERY_I2Cx_SCL_SDA_AF;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure I2C Rx as alternate function  */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


	/* Configure the Discovery I2Cx peripheral -------------------------------*/
	/* Enable I2C3 clock */
	__I2C3_CLK_ENABLE();

	/* Force the I2C Peripheral Clock Reset */
	__I2C3_FORCE_RESET();

	/* Release the I2C Peripheral Clock Reset */
	__I2C3_RELEASE_RESET();

	/* Enable and set Discovery I2Cx Interrupt to the highest priority */
	HAL_NVIC_SetPriority(I2C3_EV_IRQn, 0x00, 0);
	HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);

	/* Enable and set Discovery I2Cx Interrupt to the highest priority */
	HAL_NVIC_SetPriority(I2C3_ER_IRQn, 0x00, 0);
	HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);

	HAL_I2C_Init(&hi2c);

	__HAL_I2C_ENABLE(&hi2c);


}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the Over-Drive mode */
  HAL_PWREx_EnableOverDrive();
    
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
  * @brief  Display main demo messages
  * @param  None
  * @retval None
  */
static void Display_DemoDescription(void)
{
  uint8_t desc[50];
  
  /* Set LCD Foreground Layer  */
  BSP_LCD_SelectLayer(1);
  
  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
  
  /* Clear the LCD */ 
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  
  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);  
  
  /* Display LCD messages */
  BSP_LCD_DisplayStringAt(0, 10, (uint8_t*)"MEMS Demo", CENTER_MODE);
  BSP_LCD_SetFont(&Font16);
  
  /* Draw Bitmap */
  BSP_LCD_DrawBitmap((BSP_LCD_GetXSize() - 80)/2, 65, (uint8_t *)stlogo);
  
  BSP_LCD_SetFont(&Font8);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 20, (uint8_t*)"Copyright (c) Chamod Weerasinghe 2015", CENTER_MODE);
  
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, BSP_LCD_GetYSize()/2 + 15, BSP_LCD_GetXSize(), 60);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE); 
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 + 30, (uint8_t*)"Press USER Button to start:", CENTER_MODE);
  sprintf((char *)desc,"%s example", BSP_examples[DemoIndex].DemoName);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 + 45, (uint8_t *)desc, CENTER_MODE);   
}

/**
  * @brief  Check for user input
  * @param  None
  * @retval Input state (1 : active / 0 : Inactive)
  */
uint8_t CheckForUserInput(void)
{
  if(BSP_PB_GetState(BUTTON_KEY) == RESET)
  {
    while (BSP_PB_GetState(BUTTON_KEY) == RESET);
    return 1;
  }
  return 0;
}

/**
  * @brief  Toggle LEDs
  * @param  None
  * @retval None
  */
void Toggle_Leds(void)
{
    /*
  static uint8_t ticks = 0;
  
  if(ticks++ > 100)
  {
    BSP_LED_Toggle(LED3);
    BSP_LED_Toggle(LED4);
    ticks = 0;
  }*/
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
 if (GPIO_Pin == KEY_BUTTON_PIN)
 {
   ubKeyPressed = SET;
 }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

 
 /**
   * @brief  This function is executed in case of error occurrence.
   * @param  None
   * @retval None
   */
 static void Error_Handler(void)
 {
   /* Turn LED4 on */
   BSP_LED_On(LED4);
   while(1)
   {
   }
 }

uint8_t speed_str[20];
uint8_t* CW_GPS_DecodeVTG(uint8_t* vtg_sntc)
{
	uint16_t l = strlen((const char*)vtg_sntc);
	uint16_t i;
	uint8_t comma_count = 0;
	uint8_t beg=0, end=0;
	for (i = 0; i < l; i++)
	{
		if (vtg_sntc[i] == ',')
		{
			comma_count++;
			if (comma_count == 7)
				beg = i+1;
			if (comma_count == 8)
			{
				end = i;
				break;
			}
		}
	}
	if (end < beg+1)
		return NULL;
	else
	{
		strncpy((char*)speed_str, (const char*)(vtg_sntc + beg), end-beg);
		return speed_str;
	}
		

}

/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
