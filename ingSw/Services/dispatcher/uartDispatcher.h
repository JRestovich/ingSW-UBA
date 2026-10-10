#ifndef __UARTDISPATCHER_H
#define __UARTDISPATCHER_H

/** \brief UART message dispatcher interface.
 **
 ** \addtogroup UARTDispatcher UART Dispatcher
 ** @{ */

/* === External file includes ============================================================== */

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

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

typedef struct uart_dispatcher uart_dispatcher_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

uart_dispatcher_t *UARTDISPATCHER_get(uart_num uartNum);

void UARTDISPATCHER_close(uart_dispatcher_t *uartDispatcher);

bool UARTDISPATCHER_sendAsync(uart_dispatcher_t *uartDispatcher, const protocolData_u *message);

bool UARTDISPATCHER_subscribe(uart_dispatcher_t *uartDispatcher, const subscriber_t *subscriber);

bool UARTDISPATCHER_unsubscribe(uart_dispatcher_t *uartDispatcher, const subscriber_t *subscriber);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __UARTDISPATCHER_H */
