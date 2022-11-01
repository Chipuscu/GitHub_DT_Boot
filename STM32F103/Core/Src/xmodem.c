/**
 * @file    xmodem.c
 * @author  Ferenc Nemeth
 * @date    21 Dec 2018
 * @brief   This module is the implementation of the Xmodem protocol.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth
 */

#include "xmodem.h"
#include "stm32f1xx_it.h"
#include "stdio.h"
#include "string.h"
#include "uart1.h"
#include 	"Flash_External.h"

/* Global variables. */
//static uint8_t xmodem_packet_number = 1u;         /**< Packet number counter. */
static uint32_t xmodem_actual_flash_address = 32768; /**< Address where we have to write. */
static uint8_t x_first_packet_received = false;   /**< First packet or not. */

/* Local functions. */
static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length);
static xmodem_status xmodem_handle_packet(uint8_t size);
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number);
extern ConfStructure              Config;
				PacketStruct							Packet;							
/**
 * @brief   This function is the base of the Xmodem protocol.
 *          When we receive a header from UART, it decides what action it shall take.
 * @param   void
 * @return  void
 */
void xmodem_receive(void)
{
	
  volatile xmodem_status status = X_OK;
  x_first_packet_received = false;
	uint8_t error_number = 0u;
	Config.TxCount=0;

  /* Loop until there isn't any error (or until we jump to the user application). */
  while (X_OK == status)
  {
    uint8_t header=0;
    /* Get the header from UART. */
    uart_status comm_status = uart_receive((uint8_t *)&header, 1);
    /* Spam the host (until we receive something) with ACSII "C", to notify it, we want to use CRC-16. */
    if ((UART_OK != comm_status) && (false == x_first_packet_received))
    {
			HAL_Delay(1000);
      (void)uart_transmit_ch(X_C);
			
    }
    /* Uart timeout or any other errors. */
    else if ((UART_OK != comm_status) && (true == x_first_packet_received))
    {
			
      status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
    }
    else
    {
    }
    /* The header can be: SOH, STX, EOT and CAN. */
    switch(header)
    {
      xmodem_status packet_status = X_ERROR;
      case X_SOH:
      case X_STX:
        /* If the handling was successful, then send an ACK. */
        packet_status = xmodem_handle_packet(header);
        if (X_OK == packet_status)
        {
          uart_transmit_ch(X_ACK);
					HAL_Delay(500);
        }
				else if (X_ERROR_FLASH == packet_status)
        {
          error_number = X_MAX_ERRORS;
          status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
        }
				
        /* Error while processing the packet, either send a NAK or do graceful abort. */
        else
        {
          status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
        }
				Config.TxCount++;
        break;
				
      /* End of Transmission. */
      case X_EOT:
        /* ACK, feedback to user (as a text), then jump to user application. */
        (void)uart_transmit_ch(X_ACK);
				HAL_Delay(150);
				memset(Config.RxBuffer,0,1300);
				(void)uart_transmit_str((uint8_t*)"\n\rFirmware updated?\n\r");
        //flash_jump_to_app();
        break;
      /* Abort from host. */
      case X_CAN:
        status = X_ERROR;
        break;
      default:
        /* Wrong header. */
        if (UART_OK == comm_status)
        {
          status = xmodem_error_handler(&error_number, X_MAX_ERRORS);
        }
        break;
    }
		
  }
}


/**
 * @brief   Calculates the CRC-16 for the input package.
 * @param   *data:  Array of the data which we want to calculate.
 * @param   length: Size of the data, either 128 or 1024 bytes.
 * @return  status: The calculated CRC.
 */
static uint16_t xmodem_calc_crc(uint8_t *data, uint16_t length)
{
      uint16_t crc = 0u;
    while (length)
    {
        length--;
        crc = crc ^ ((uint16_t)*data++ << 8u);
        for (uint8_t i = 0u; i < 8u; i++)
        {
            if (crc & 0x8000u)
            {
                crc = (crc << 1u) ^ 0x1021u;
            }
            else
            {
                crc = crc << 1u;
            }
        }
    }
    return crc;
//		 uint32_t i, j;

//  for (i = 0; i < length; i++)
//  {
//    j = (crc >> 8) ^data[i];
//    crc = (crc << 8) ^ _crc(j);
//  }

//  return crc;
}
//static uint16_t _crc(uint32_t n)
//{
//  uint32_t i, acc;

//  for (n <<= 8, acc = 0, i = 8; i > 0; i--, n <<= 1)
//  {
//    acc = ((n ^ acc) & 0x8000) ? ((acc << 1) ^ 0x1021): (acc << 1);
//  }

//  return (uint16_t)(acc);
//}

/**
 * @brief   This function handles the data packet we get from the xmodem protocol.
 * @param   header: SOH or STX.
 * @return  status: Report about the packet.
 */
static xmodem_status xmodem_handle_packet(uint8_t header)
{
//	uint8_t Tem[2];
	char Data[50];
  xmodem_status status = X_OK;
  uint16_t size = 1024;
	uint16_t t=0;
	
  /* 2 bytes for packet number, 1024 for data, 2 for CRC*/
   uart_status comm_status = UART_OK;
  /* Get the packet number, data and CRC from UART. */
	
	Packet.received_packet_number[0]=Config.RxBuffer[t++];
	Packet.received_packet_number[1]=Config.RxBuffer[t++];
	t++;
	for(int pos=0;pos<1024;pos++)
	{
		Packet.received_packet_data[pos]=Config.RxBuffer[t++];
	}
	Packet.received_packet_crc[0]=Config.RxBuffer[t++];
	Packet.received_packet_crc[1]=Config.RxBuffer[t++];
  /* Merge the two bytes of CRC. */
  uint16_t crc_received = ((Packet.received_packet_crc[0] << 8u) | Packet.received_packet_crc[1]);
  /* We calculate it too. */
	uint16_t crc_calculated=xmodem_calc_crc(Packet.received_packet_data, 1024);
	
	
	 /* Communication error. */
  if (UART_OK != comm_status)
  {
    status |= X_ERROR_UART;
  }
  if (crc_calculated != crc_received)
  {
     /* The calculated and received CRC are different. */
     status |= X_ERROR_CRC;
	}
	 /* If it is the first packet, then erase the memory. */
	if ((X_OK == status) && (false == x_first_packet_received))
  {
    if (FLASH_OK == flash_erase(FLASH_APP_START_ADDRESS))
    {
      x_first_packet_received = true;
    }
    else
    {
      status |= X_ERROR_FLASH;
    }
  }

	
/*************************/
    /* Do the actual flashing (if there weren't any errors). */
   if ((X_OK == status))
    {
			Flash_WriteBuffer(Packet.received_packet_data,xmodem_actual_flash_address,1024);
			//flash_write(xmodem_actual_flash_address, (uint32_t*)Packet.received_packet_data, 1024/4u);
			HAL_Delay(100);
			memset(Packet.received_packet_data,0,1024);
			
      /* Flashing error. */
    }

  /* Raise the packet number and the address counters (if there weren't any errors). */
  if (X_OK == status)
  {
    xmodem_actual_flash_address += size;
  }

  return status;
}

/**
 * @brief   Handles the xmodem error.
 *          Raises the error counter, then if the number of the errors reached critical, do a graceful abort, otherwise send a NAK.
 * @param   *error_number:    Number of current errors (passed as a pointer).
 * @param   max_error_number: Maximal allowed number of errors.
 * @return  status: X_ERROR in case of too many errors, X_OK otherwise.
 */
static xmodem_status xmodem_error_handler(uint8_t *error_number, uint8_t max_error_number)
{
  xmodem_status status = X_OK;
  /* Raise the error counter. */
  (*error_number)++;
  /* If the counter reached the max value, then abort. */
  if ((*error_number) >= max_error_number)
  {
    /* Graceful abort. */
    (void)uart_transmit_ch(X_CAN);
    (void)uart_transmit_ch(X_CAN);
    status = X_ERROR;
  }
  /* Otherwise send a NAK for a repeat. */
  else
  {
    (void)uart_transmit_ch(X_NAK);
    status = X_OK;
  }
  return status;
}
