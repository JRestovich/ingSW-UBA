/** \brief GPS RMC data dispatch to subscribed queues.
 **
 ** \addtogroup GPSDispatcher GPS Dispatcher
 ** @{ */

/* === Header includes ===================================================================== */

#include "gpsDispatcher.h"

/* === Definitions and macros ============================================================== */

#define MAX_SUBSCRIBER_QTY 8U

/* === Internal data type declarations ===================================================== */

struct gps_dispatcher {
    bool ready;
    uart_num id;
    uart_driver_t *uartComm;

    TaskHandle_t task_parser;
    StreamBufferHandle_t rx_stream;

    gps_parser_t parser;
    gps_subscriber_t subscribers[MAX_SUBSCRIBER_QTY];
};

/* === Internal function declarations ====================================================== */

static void task_rx_parser(void *parameters);
static bool _open(gps_dispatcher_t *gpsDispatcher, uart_num uartNum);
static bool _dispatch(gps_dispatcher_t *gpsDispatcher, const gps_rmc_data_t *data);
static void _onRmc(const gps_rmc_data_t *data);
static void _initSubscribers(gps_dispatcher_t *gpsDispatcher);

/* === Internal variable definitions ======================================================= */

static const uart_config_t gpsUartConfig = {
    .baudrate = BAUDRATE_9600, .stop_bits = ONE_STOP_BIT, .parity = NONE};

static gps_dispatcher_t gpsDispatcherInstance = {0};

/* === External variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */

static bool _open(gps_dispatcher_t *gpsDispatcher, uart_num uartNum) {
    if (gpsDispatcher == NULL || (unsigned)uartNum >= UART_COUNT) {
        return false;
    }
    if (gpsDispatcher->ready) {
        return gpsDispatcher->id == uartNum;
    }

    gpsDispatcher->id = uartNum;
    gpsDispatcher->uartComm = UARTCOMM_get(uartNum, &gpsUartConfig);
    if (gpsDispatcher->uartComm == NULL) {
        return false;
    }

    gpsDispatcher->rx_stream = UARTCOMM_getRxStream(gpsDispatcher->uartComm);
    if (gpsDispatcher->rx_stream == NULL) {
        return false;
    }

    _initSubscribers(gpsDispatcher);
    GPS_PARSER_init(&gpsDispatcher->parser, _onRmc);

    BaseType_t ret =
        xTaskCreate(task_rx_parser, "Task GPS Parser", (configMINIMAL_STACK_SIZE),
                    (void *)gpsDispatcher, (tskIDLE_PRIORITY + 1ul), &gpsDispatcher->task_parser);
    if (pdPASS != ret) {
        return false;
    }

    gpsDispatcher->ready = true;
    return true;
}

static void task_rx_parser(void *parameters) {
    gps_dispatcher_t *gpsDispatcher = parameters;
    char rxDataBuffer[UART_RX_CHUNK_LEN];

    for (;;) {
        size_t rxBytesQty = xStreamBufferReceive(gpsDispatcher->rx_stream, rxDataBuffer,
                                                 sizeof(rxDataBuffer), portMAX_DELAY);
        GPS_PARSER_consume(&gpsDispatcher->parser, rxDataBuffer, rxBytesQty);
    }
}

static void _onRmc(const gps_rmc_data_t *data) {
    (void)_dispatch(&gpsDispatcherInstance, data);
}

static void _initSubscribers(gps_dispatcher_t *gpsDispatcher) {
    for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
        gpsDispatcher->subscribers[index].queue_sub = NULL;
    }
}

static bool _dispatch(gps_dispatcher_t *gpsDispatcher, const gps_rmc_data_t *data) {
    bool delivered = false;

    if (gpsDispatcher == NULL || data == NULL) {
        return false;
    }

    for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
        gps_subscriber_t *subscriber = &gpsDispatcher->subscribers[index];

        if (subscriber->queue_sub == NULL) {
            continue;
        }

        if (xQueueSend(subscriber->queue_sub, data, 0U) == pdPASS) {
            delivered = true;
        }
    }

    return delivered;
}

/* === External function definitions ======================================================= */

gps_dispatcher_t *GPSDISPATCHER_get(uart_num uartNum) {
    if ((unsigned)uartNum >= UART_COUNT || !_open(&gpsDispatcherInstance, uartNum)) {
        return NULL;
    }

    return &gpsDispatcherInstance;
}

void GPSDISPATCHER_close(gps_dispatcher_t *gpsDispatcher) {
    uart_driver_t *uartComm;

    if (gpsDispatcher == NULL || !gpsDispatcher->ready) {
        return;
    }

    gpsDispatcher->ready = false;
    if (gpsDispatcher->task_parser != NULL) {
        vTaskDelete(gpsDispatcher->task_parser);
        gpsDispatcher->task_parser = NULL;
    }

    uartComm = gpsDispatcher->uartComm;
    gpsDispatcher->rx_stream = NULL;
    gpsDispatcher->uartComm = NULL;
    _initSubscribers(gpsDispatcher);

    UARTCOMM_close(uartComm);
}

bool GPSDISPATCHER_subscribe(gps_dispatcher_t *gpsDispatcher, const gps_subscriber_t *subscriber) {
    uint8_t available_index = MAX_SUBSCRIBER_QTY;

    if (gpsDispatcher == NULL || !gpsDispatcher->ready || subscriber == NULL ||
        subscriber->queue_sub == NULL) {
        return false;
    }

    for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
        // Keep the first available slot while checking for duplicates.
        if (gpsDispatcher->subscribers[index].queue_sub == NULL) {
            if (available_index == MAX_SUBSCRIBER_QTY) {
                available_index = index;
            }
            continue;
        }

        // Avoid repeated subscribers.
        if (gpsDispatcher->subscribers[index].queue_sub == subscriber->queue_sub) {
            return false;
        }
    }

    if (available_index >= MAX_SUBSCRIBER_QTY) {
        return false;
    }

    gpsDispatcher->subscribers[available_index] = *subscriber;
    return true;
}

bool GPSDISPATCHER_unsubscribe(gps_dispatcher_t *gpsDispatcher,
                               const gps_subscriber_t *subscriber) {
    if (gpsDispatcher == NULL || !gpsDispatcher->ready || subscriber == NULL ||
        subscriber->queue_sub == NULL) {
        return false;
    }

    for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
        if (gpsDispatcher->subscribers[index].queue_sub == subscriber->queue_sub) {

            gpsDispatcher->subscribers[index].queue_sub = NULL;
            return true;
        }
    }

    return false;
}

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */
