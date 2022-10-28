#ifndef __FLASH_EXTENAL__
#define __FLASH_EXTENAL__

#include "stm32f2xx.h"                  // Device header

#define FLASH_Enable() HAL_GPIO_WritePin(_W25QXX_CS_GPIO, _W25QXX_CS_PIN, GPIO_PIN_RESET)
#define FLASH_Disable() HAL_GPIO_WritePin(_W25QXX_CS_GPIO, _W25QXX_CS_PIN, GPIO_PIN_SET);



uint8_t SPI_Sendata(uint8_t Data);
void Flash_ReadBuffer(int Address, uint8_t *Buffer,uint16_t Len);
void Flash_Erase_Sector(uint32_t Address);
void Erase_Sector(void);
void Flash_WaitForWriteEnd(void);
void Flash_DisableWrite(void);
void Flash_EnableWrite(void);
void Flash_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t Len);
void Flash_Writepage(int Address, uint8_t *Buffer,uint16_t Len);
void Flash_ReadBuffer1(uint32_t Address,char *Buffer,uint16_t Len);
//void Flash_ReadBuffer(int Address, uint16_t *Buffer,uint16_t Len);

#endif
