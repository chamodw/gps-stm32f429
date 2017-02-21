#ifndef __UART_H_
#define __UART_H_
#include "stm32f429i_discovery.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_usart.h"
#define USART1_BUFFER_SIZE 200


void CW_USART1_Init();
uint16_t CW_USART1_NewData();
uint16_t CW_USART1_GetLine(uint8_t* dest, uint16_t size);
uint16_t CW_USART1_CountChar(uint8_t c);
uint16_t CW_USART1_GetData(uint8_t* dest, uint16_t size);
uint16_t CW_USART1_IRqHandler(UART_HandleTypeDef* handle);
void CW_USART1_SendChar(uint8_t c);
void CW_USART1_SendStr(uint8_t* str);
#endif
