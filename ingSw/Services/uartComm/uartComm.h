/* ******************************************************************************
  * @file           : uartcomm.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UARTCOMM_H
#define __UARTCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "uart.h"
#include "ring_buffer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "queue.h"
#include "semphr.h"

/* Private includes ----------------------------------------------------------*/

#define UART_RX_RING_BUFFER_LEN 64
#define UART_TX_MAX_PAYLOAD      64U

typedef struct uart_driver uart_driver_t;

typedef struct
{
	uint8_t	data[UART_TX_MAX_PAYLOAD];
	uint16_t length;
} uart_msg_t;

uart_driver_t* UARTCOMM_get(uart_num uartNum);

void UARTCOMM_close(uart_driver_t *uartDriver);

bool UARTCOMM_sendAsync(uart_driver_t *uartDriver, const uint8_t *pData, uint16_t size);

bool UARTCOMM_readAsync(uart_driver_t *uartDriver, uint8_t *pData, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __UARTCOMM_H */
