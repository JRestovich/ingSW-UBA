#ifndef __UARTCOMM_H
#define __UARTCOMM_H

/** \brief Asynchronous UART communication service interface.
 **
 ** \addtogroup UARTComm UART Communication
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "queue.h"
#include "semphr.h"
#include "stream_buffer.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

#define UART_RX_CHUNK_LEN         64U
#define UART_RX_STREAM_BUFFER_LEN 256U
#define UART_TX_MAX_PAYLOAD      64U

/* === Data type declarations ============================================================== */

typedef struct uart_driver uart_driver_t;

typedef struct
{
	uint8_t	data[UART_TX_MAX_PAYLOAD];
	uint16_t length;
} uart_msg_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

uart_driver_t* UARTCOMM_get(uart_num uartNum);

void UARTCOMM_close(uart_driver_t *uartDriver);

bool UARTCOMM_sendAsync(uart_driver_t *uartDriver, const uint8_t *pData, uint16_t size);

StreamBufferHandle_t UARTCOMM_getRxStream(const uart_driver_t *uartDriver);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __UARTCOMM_H */
