/**
 * @file    uart.h
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module is a layer between the HAL UART functions and my Xmodem protocol.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

/* Timeout for HAL. */
#define UART_TIMEOUT ((uint16_t)2000u)

/* Status report for the functions. */
typedef enum {
  UART_OK     = 0x00 ,/**< The action was successful. */
  UART_ERROR  = 0xFF  /**< Generic error. */
} uart_status;
typedef struct
{
  uint16_t Read;
  uint16_t Write;
  uint8_t Memory[1500];
  uint16_t Count;
} Buffer_TypeDef;
 typedef struct
{
	uint16_t RxCounter;
	char  RxBuffer[1300];
	uint8_t TxBuffer[1300];
	uint8_t TxCount;
	uint16_t timeout;
	uint8_t NewData;
	uint8_t NewData1;
	
} ConfStructure;

uart_status uart_receive(uint8_t *data, uint16_t length);
uart_status uart_transmit_str(uint8_t *data);
uart_status uart_transmit_ch(uint8_t data);
void Ring_Buffer_Write(Buffer_TypeDef *Buffer,uint8_t Data);
uint8_t Ring_Buffer_Read(Buffer_TypeDef *Buffer);
void Buffer_Handler(void);
uint8_t  Config_Process(void);
int16_t	TimViTriKyTu(char ch, char *Buffer);
uint8_t TachDuLieu(char* String, char* Buffer,char start, char end);
#endif /* UART_H_ */
