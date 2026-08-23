#include "uartDispatcher.h"
#include <string.h>

#define MAX_SUBSCRIBER_QTY 8U

static void task_rx_parser(void *parameters);
static bool _open(uart_dispatcher_t *uartDispatcher, uart_num uartNum);

struct uart_dispatcher {
	bool ready;
	uart_num id;
	uart_driver_t *uartComm;

	TaskHandle_t	task_parser;
	StreamBufferHandle_t rx_stream;

	subscriber_t subscribers[MAX_SUBSCRIBER_QTY];
	uint8_t subscriber_count;
};

static uart_dispatcher_t uartDispatchers[UART_COUNT] = {0};

uart_dispatcher_t *UARTDISPATCHER_get(uart_num uartNum)
{
	if (uartNum >= UART_COUNT || !_open(&uartDispatchers[uartNum], uartNum)) {
		return NULL;
	}

	return &uartDispatchers[uartNum];
}

static bool _open(uart_dispatcher_t *uartDispatcher, uart_num uartNum)
{
	if (uartDispatcher == NULL || uartNum >= UART_COUNT) {
		return false;
	}
	if (uartDispatcher->ready) {
		return true;
	}

	uartDispatcher->id = uartNum;
	uartDispatcher->uartComm = UARTCOMM_get(uartNum);
	if (uartDispatcher->uartComm == NULL) {
		return false;
	}

	uartDispatcher->rx_stream = UARTCOMM_getRxStream(uartDispatcher->uartComm);
	if (uartDispatcher->rx_stream == NULL) {
		return false;
	}

	uartDispatcher->subscriber_count = 0U;
	memset(uartDispatcher->subscribers, 0, sizeof(uartDispatcher->subscribers));

	BaseType_t ret = xTaskCreate(task_rx_parser, "Task Rx Parser", (configMINIMAL_STACK_SIZE),
    				  (void *)uartDispatcher,
					  (tskIDLE_PRIORITY + 1ul), &uartDispatcher->task_parser);
    if (pdPASS != ret) {
    	return false;
    }
	
	uartDispatcher->ready = true;
	return true;
}

void UARTDISPATCHER_close(uart_dispatcher_t *uartDispatcher)
{
	uart_driver_t *uartComm;

	if (uartDispatcher == NULL || !uartDispatcher->ready) {
		return;
	}

	uartDispatcher->ready = false;
	if (uartDispatcher->task_parser != NULL) {
		vTaskDelete(uartDispatcher->task_parser);
		uartDispatcher->task_parser = NULL;
	}

	uartComm = uartDispatcher->uartComm;
	uartDispatcher->rx_stream = NULL;
	uartDispatcher->uartComm = NULL;
	uartDispatcher->subscriber_count = 0U;
	memset(uartDispatcher->subscribers, 0, sizeof(uartDispatcher->subscribers));

	UARTCOMM_close(uartComm);
}

static void task_rx_parser(void *parameters)
{
	(void)parameters;

	for (;;) {
	}

}
