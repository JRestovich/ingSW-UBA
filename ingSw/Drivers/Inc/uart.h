/* ******************************************************************************
  * @file           : pwm.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
typedef struct struct_uart_t uart_t;
typedef void (*uart_txCallback_t)(void *context);
typedef void (*uart_rxCallback_t)(uint16_t size, void *context);

typedef enum {
	UART_1,
	UART_2,
	UART_3,
	UART_COUNT
} uart_num;

typedef enum {
	BAUDRATE_9600 = 9600,
	BAUDRATE_115200 = 115200
} BAUDRATE_t;

typedef enum {
	ONE_STOP_BIT,
	TWO_STOP_BIT
} STOP_BITS_t;

typedef enum {
	NONE,
	ODD,
	EVEN,
} PARITY_t;

uart_t* UART_ctor(uart_num uartNum);

bool UART_init(uart_t *self, BAUDRATE_t baud, STOP_BITS_t stopBits, PARITY_t parity);

bool UART_setTxCallback(uart_t *self, uart_txCallback_t callback, void *context);

bool UART_setRxCallback(uart_t *self, uart_rxCallback_t callback, void *context);

bool UART_send(uart_t *self, const uint8_t *pData, uint16_t size);

bool UART_sendAsync(uart_t *self, const uint8_t *pData, uint16_t size);

bool UART_read(uart_t *self, uint8_t *pData, uint16_t size, uint16_t *rxLen);

bool UART_readAsync(uart_t *self, uint8_t *pData, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
