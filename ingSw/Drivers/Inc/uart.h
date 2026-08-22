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
typedef void (*uart_callback_t)(void);

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

bool UART_setTxCallback(uart_t *self, uart_callback_t callback);

bool UART_setRxCallback(uart_t *self, uart_callback_t callback);
#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
