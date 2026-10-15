#ifndef __GPSDISPATCHER_H
#define __GPSDISPATCHER_H

/** \brief GPS RMC data dispatcher interface.
 **
 ** \addtogroup GPSDispatcher GPS Dispatcher
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include "uartComm/uartComm.h"
#include "gps_parser.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

typedef struct gps_dispatcher gps_dispatcher_t;

/** Queue items must have sizeof(gps_rmc_data_t). Each valid RMC is copied to all subscribers. */
typedef struct {
    QueueHandle_t queue_sub;
} gps_subscriber_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

/** Single instance bound to uartNum until close. A different UART returns NULL while open.
 * Owns the UART RX stream; do not share it with UARTDISPATCHER on the same UART.
 */
gps_dispatcher_t *GPSDISPATCHER_get(uart_num uartNum);

void GPSDISPATCHER_close(gps_dispatcher_t *gpsDispatcher);

bool GPSDISPATCHER_subscribe(gps_dispatcher_t *gpsDispatcher, const gps_subscriber_t *subscriber);

bool GPSDISPATCHER_unsubscribe(gps_dispatcher_t *gpsDispatcher, const gps_subscriber_t *subscriber);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __GPSDISPATCHER_H */
