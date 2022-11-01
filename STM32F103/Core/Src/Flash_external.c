#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include	"uart1.h"
#include	"flash.h"
#include	"xmodem.h"
#include	"stdio.h"
#include	"string.h"
#include	"uart1.h"
#include "Flash_External.h"


#define _W25QXX_CS_GPIO               GPIOA
#define _W25QXX_CS_PIN                GPIO_PIN_4
#define _W25QXX_USE_FREERTOS          1
#define _W25QXX_DEBUG                 0
#define FLASH_SPI_DUMMY								0x00
#define Readdata											0x03
#define FLASH_SPI_PAGESIZE 						256
#define PageProgram										 0x02

extern SPI_HandleTypeDef hspi1;


uint8_t SPI_Sendata(uint8_t Data)
{
	uint8_t ret;
	uint16_t Timeout=1000;
//	while(__HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_TXE)==0&&Timeout)
//	{
//			Timeout--;
//	}
//	if(Timeout==0)
//	{
//		return 0;
//	}
	HAL_SPI_TransmitReceive(&hspi1, &Data, &ret, 1, 100);
	Timeout=1000;
//	while((__HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_RXNE)==RESET)&&Timeout)
//	{
//		Timeout--;
//	}
	return ret;
	
}
//###################################################################################################################
void Flash_EnableWrite(void)
{
	FLASH_Enable();
	SPI_Sendata(0x06);
	FLASH_Disable();
	//HAL_Delay(1);
}
//###################################################################################################################
void Erase_Sector(void)
{
	uint16_t Count1=0,Count2=10;
		while(Count1<Count2)
			{
				Flash_Erase_Sector(Count1*4*1024);
				Count1++;
				HAL_Delay(50);
			}
	
}
void Flash_Erase_Sector(uint32_t Address)
{

	Flash_EnableWrite();
	FLASH_Enable();
	SPI_Sendata(0x20);
	/*Gui 3 byte dia chi*/
	SPI_Sendata((Address >> 16)&0xFF);
	SPI_Sendata((Address >> 8)&0xFF);
	SPI_Sendata(Address&0xFF);
	FLASH_Disable();
	HAL_Delay(1);
	Flash_WaitForWriteEnd();

}
//###################################################################################################################
void Flash_WaitForWriteEnd(void)
{
	uint8_t StatusRegister1=0;
	uint32_t	TimeOut = 0xFFFFFFF;
	uint8_t		Status = 0;
	
	HAL_Delay(1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO, _W25QXX_CS_PIN, GPIO_PIN_RESET);
	//FLASH_Enable();
	SPI_Sendata(0x05);
	do
	{
		StatusRegister1 = SPI_Sendata(FLASH_SPI_DUMMY);
		HAL_Delay(1);
	} while ((StatusRegister1 & 0x01) == 0x01);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO, _W25QXX_CS_PIN, GPIO_PIN_SET);
//	while(TimeOut)
//		{
//			Status = (uint8_t)SPI_Sendata(FLASH_SPI_DUMMY);		
//			TimeOut--;
//			if((Status & 1) == 0)
//				break;
//		}
	//FLASH_Disable();
}
//########################################################################################################
void Flash_ReadBuffer1(uint32_t Address,char *Buffer,uint16_t Len)
{
	FLASH_Enable();
	
	SPI_Sendata(Readdata);
	SPI_Sendata((Address >> 16) & 0xFF);		
	SPI_Sendata((Address >> 8) & 0xFF);
	SPI_Sendata(Address  & 0xFF);
	while(Len--)
	{
		*Buffer = SPI_Sendata(FLASH_SPI_DUMMY);	
		Buffer++;
	}
	FLASH_Disable();
}
void Flash_ReadBuffer(int Address, uint8_t *Buffer,uint16_t Len)
{
	volatile uint8_t status;
	FLASH_Enable();
	status=SPI_Sendata(Readdata);
	status=SPI_Sendata((Address >> 16)&0xFF);
	status=SPI_Sendata((Address >> 8)&0xFF);
	status=SPI_Sendata(Address&0xFF);
	while(Len--)
	{
		*Buffer=SPI_Sendata(FLASH_SPI_DUMMY);
		 Buffer++;
	}
	FLASH_Disable();
}
//########################################################################################################
void Flash_Writepage(int Address, uint8_t *Buffer,uint16_t Len)
{
	
	Flash_EnableWrite();
	FLASH_Enable();
	SPI_Sendata(0x02);
	SPI_Sendata((Address >> 16)&0xFF);
	SPI_Sendata((Address >> 8)&0xFF);
	SPI_Sendata(Address&0xFF);
	//HAL_SPI_Transmit(&hspi1,Buffer,Len,100);
	while(Len--)
	{
		SPI_Sendata(*Buffer);
		Buffer++;
	}
	FLASH_Disable();
	//Flash_WaitForWriteEnd();
}
//########################################################################################################
void Flash_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t Len)
{
  uint16_t Addr = 0, count = 0,CountPage=0;
	int temp = 0;
	Addr = WriteAddr % FLASH_SPI_PAGESIZE;	// Da viet den dia chi nay
	
	if (Addr+Len<=FLASH_SPI_PAGESIZE) //So luong Byte +dia chi < 1 page
	{
			Flash_Writepage((int)WriteAddr,pBuffer,Len);
			HAL_Delay(1);
			
	}
	else
	{
		count=FLASH_SPI_PAGESIZE-Addr;
		Flash_Writepage((int)WriteAddr,pBuffer,count); 
		HAL_Delay(1);
		temp=(int)(Len-count); 
		CountPage=temp/FLASH_SPI_PAGESIZE; 
		WriteAddr+=count;				
		pBuffer+=count;
		temp=temp%FLASH_SPI_PAGESIZE; 		
		if(CountPage>0)
		{
			while(CountPage--)
			{
				
				Flash_Writepage((int)WriteAddr,pBuffer,FLASH_SPI_PAGESIZE);
				pBuffer+=FLASH_SPI_PAGESIZE;
				WriteAddr+=FLASH_SPI_PAGESIZE;
				HAL_Delay(1);
				
			}
		}
		Flash_Writepage((int)WriteAddr,pBuffer,(uint16_t)temp);
	}
}
