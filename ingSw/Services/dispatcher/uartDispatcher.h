/* ******************************************************************************
  * @file           : uartDistpacher.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UARTDISPATCHER_H
#define __UARTDISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "uartComm/uartComm.h"
#include "uart.h"
#include "protocol.h"

#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "queue.h"
#include "semphr.h"
#include "stream_buffer.h"

/* Private includes ----------------------------------------------------------*/
typedef struct uart_dispatcher uart_dispatcher_t;

uart_dispatcher_t *UARTDISPATCHER_get(uart_num uartNum);

void UARTDISPATCHER_close(uart_dispatcher_t *uartDispatcher);

bool UARTDISPATCHER_sendAsync(uart_dispatcher_t *uartDispatcher,
                              const protocolData_u *message);

bool UARTDISPATCHER_subscribe(uart_dispatcher_t *uartDispatcher,
                              const subscriber_t *subscriber);

bool UARTDISPATCHER_unsubscribe(uart_dispatcher_t *uartDispatcher,
                              const subscriber_t *subscriber);

#ifdef __cplusplus
}
#endif

#endif /* __UARTDISPATCHER_H */
