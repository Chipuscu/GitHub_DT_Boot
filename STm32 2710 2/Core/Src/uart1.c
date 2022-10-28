/**
 * @file    uart.c
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module is a layer between the HAL UART functions and my Xmodem protocol.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#include "uart1.h"
#include "main.h"
#include "string.h"
#include "flash.h"
#include "stdlib.h"
#include 	"Flash_External.h"
#include "stdio.h"

Buffer_TypeDef					UART1_Data;
ConfStructure              Config;
/**
 * @brief   Receives data from UART.
 * @param   *data: Array to save the received data.
 * @param   length:  Size of the data.
 * @return  status: Report about the success of the receiving.
 */
uart_status uart_receive(uint8_t *data, uint16_t length)
{
  uart_status status = UART_OK;
	for(int i=0;i<length;++i)
	{
		data[i]=Config.RxBuffer[i];
	}
	if(data[0]==NULL)
	{
		status=UART_ERROR;
	}
  return status;
}


 void Buffer_Handler(void)
{
	while(UART1_Data.Read!=UART1_Data.Write)
	{
		uint8_t byteRead=Ring_Buffer_Read(&UART1_Data);

		Config.RxBuffer[UART1_Data.Count++]=byteRead;
	}
	
}


uint8_t Ring_Buffer_Read(Buffer_TypeDef *Buffer)
{
	uint8_t Data;
	if(Buffer->Read==Buffer->Write) 
		return 0;
	Data=Buffer->Memory[Buffer->Read++];
	if(Buffer->Read>=1500)
		Buffer->Read=0;
	return Data;
}

/**
 * @brief   Transmits a string to UART.
 * @param   *data: Array of the data.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_str(uint8_t *data)
{
  uart_status status = UART_ERROR;
  uint16_t length = 0u;

  /* Calculate the length. */
  while ('\0' != data[length])
  {
    length++;
  }

  if (HAL_OK == HAL_UART_Transmit(&huart1, data, length, UART_TIMEOUT))
  {
    status = UART_OK;
  }

  return status;
}

/**
 * @brief   Transmits a single char to UART.
 * @param   *data: The char.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_ch(uint8_t data)
{
  uart_status status = UART_ERROR;

  /* Make available the UART module. */
  if (HAL_UART_STATE_TIMEOUT == HAL_UART_GetState(&huart1))
  {
    HAL_UART_Abort(&huart1);
  }

  if (HAL_OK == HAL_UART_Transmit(&huart1, &data, 1u, UART_TIMEOUT))
  {
    status = UART_OK;
  }
  return status;
}
void Ring_Buffer_Write(Buffer_TypeDef *Buffer,uint8_t Data)
{
	Buffer->Memory[Buffer->Write++]=Data;
	if(Buffer->Write>=1500)
		Buffer->Write=0;
}

uint8_t  Config_Process(void)
{
	char Data[30];
	char Address[10];
	uint32_t AddtoRead;
	mem_set(Address,0,10);
	
	
	if(strstr((char*)Config.RxBuffer,"RADD")!=NULL)
	{
		TachDuLieu((char*)Config.RxBuffer,Address,'(',')');
		AddtoRead = atoi((const char*)Address);
		Flash_ReadBuffer(AddtoRead,Config.TxBuffer,100);
		//uart_transmit_str((uint8_t  *)Config.TxBuffer);
		//Send1(Config.TxBuffer,1024);
	}
	if(strstr((char*)Config.RxBuffer,"UP")!=NULL)
	{
		
		TachDuLieu((char*)Config.RxBuffer,Address,'(',')');
		AddtoRead = atoi((const char*)Address);
		for(int i=0;i<Config.TxCount;++i)
		{
			Flash_ReadBuffer(AddtoRead,Config.TxBuffer,1024);
			HAL_Delay(10);
			flash_write(AddtoRead, (uint32_t*)Config.TxBuffer, 1024/4u);
			HAL_Delay(10);
			AddtoRead+=1024;
			sprintf(Data,"Add=%d\r\t",AddtoRead);
			uart_transmit_str((uint8_t *)Data);
		}
	}
	if(strstr((char*)Config.RxBuffer,"AS")!=NULL)
	{
		flash_jump_to_app();
	}
	return 1;
}

uint8_t TachDuLieu(char* String, char* Buffer,char start, char end)
{
	int16_t	Address1= TimViTriKyTu(start, String);
	int16_t	Address2 = TimViTriKyTu(end,String);
	int16_t	tmpCount,i = 0;

	/* Kiem tra dinh dang du lieu */
	if((Address1 == -1) || Address2 == -1)
		return 0;
	if(Address2 - Address1 <= 1)
		return 0;

	for(tmpCount =Address1 + 1; tmpCount < Address2; tmpCount++)
	{
		Buffer[i++] = String[tmpCount];	
	}
		
	return 1;	
	
	
}
int16_t	TimViTriKyTu(char ch, char *Buffer)
{
	uint8_t		tmpCount = 0;

	/* Do dai du lieu */

	for(tmpCount = 0; tmpCount < strlen(Buffer); tmpCount ++)
	{
		if(Buffer[tmpCount] == ch)
			return tmpCount;
	}
	return -1;
}
