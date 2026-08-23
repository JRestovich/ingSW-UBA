#include "uartDispatcher.h"
#include <string.h>

#define MAX_SUBSCRIBER_QTY 8U

typedef enum {
	PARSER_IDLE = 0U,
	PARSER_SUBSYSTEM,
	PARSER_CMD,
	PARSER_SIZE_1,
	PARSER_SIZE_2,
	PARSER_PAYLOAD
} PARSER_STATE_E;

static void task_rx_parser(void *parameters);
static bool _open(uart_dispatcher_t *uartDispatcher, uart_num uartNum);
static bool _dispatch(uart_dispatcher_t *uartDispatcher,
                      const protocolData_u *message);
static void _clearMessage(protocolData_u *message);
static void _initSubscribers(uart_dispatcher_t *uartDispatcher);

struct uart_dispatcher {
	bool ready;
	uart_num id;
	uart_driver_t *uartComm;

	TaskHandle_t	task_parser;
	StreamBufferHandle_t rx_stream;

	subscriber_t subscribers[MAX_SUBSCRIBER_QTY];
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

	_initSubscribers(uartDispatcher);

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
	_initSubscribers(uartDispatcher);

	UARTCOMM_close(uartComm);
}

static void task_rx_parser(void *parameters)
{
	uart_dispatcher_t *uartDispatcher = parameters;
	protocolData_u message = {0};
	uint8_t rxDataBuffer[UART_RX_CHUNK_LEN];
	PARSER_STATE_E parser_state = PARSER_IDLE;
	uint8_t payload_size = 0U;
	size_t payload_index = 0U;

	for (;;) {
		size_t rxBytesQty = xStreamBufferReceive(uartDispatcher->rx_stream,
										  rxDataBuffer, sizeof(rxDataBuffer),
										  portMAX_DELAY);
		if (rxBytesQty == 0U) {
			continue;
		}

		for (size_t idx = 0U; idx < rxBytesQty; idx++) {
			uint8_t byte = rxDataBuffer[idx];

			switch (parser_state) {
			case PARSER_IDLE:
					if (byte == (uint8_t)PROTOCOL_START_BYTE) {
						_clearMessage(&message);
						message.frame.startByte = byte;
						parser_state = PARSER_SUBSYSTEM;
					}
					break;

				case PARSER_SUBSYSTEM:
					message.frame.subsystem = byte;
					parser_state = PARSER_CMD;
					break;

				case PARSER_CMD:
					message.frame.cmd = byte;
					parser_state = PARSER_SIZE_1;
					break;

			case PARSER_SIZE_1:
					if (byte < (uint8_t)'0' || byte > (uint8_t)'9') {
						_clearMessage(&message);
						parser_state = PARSER_IDLE;
						break;
					}
					message.frame.payload_size[0] = byte;
					parser_state = PARSER_SIZE_2;
					break;

				case PARSER_SIZE_2:
					message.frame.payload_size[1] = byte;
					if (!PROTOCOL_payloadSizeToUint8(message.frame.payload_size, &payload_size) ||
						payload_size > PAYLOAD_MAX_SIZE) {
						_clearMessage(&message);
						parser_state = PARSER_IDLE;
						break;
					}

					payload_index = 0U;
					if (payload_size == 0U) {
						(void)_dispatch(uartDispatcher, &message);
						_clearMessage(&message);
						parser_state = PARSER_IDLE;
					} else {
						parser_state = PARSER_PAYLOAD;
					}
					break;

				case PARSER_PAYLOAD:
					message.frame.payload[payload_index++] = byte;
					if (payload_index == payload_size) {
						(void)_dispatch(uartDispatcher, &message);
						_clearMessage(&message);
						parser_state = PARSER_IDLE;
					}
					break;

			default:
					_clearMessage(&message);
					parser_state = PARSER_IDLE;
					break;
			}
		}
	}
}

static void _clearMessage(protocolData_u *message)
{
	memset(message, 0, sizeof(*message));
}

static void _initSubscribers(uart_dispatcher_t *uartDispatcher)
{
	for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
		uartDispatcher->subscribers[index].subsystem = SUBSYSTEM_NONE;
		uartDispatcher->subscribers[index].queue_sub = NULL;
	}
}

static bool _dispatch(uart_dispatcher_t *uartDispatcher,
                      const protocolData_u *message)
{
	bool delivered = false;

	if (uartDispatcher == NULL || message == NULL ||
		!PROTOCOL_isValidSubsystem(message->frame.subsystem)) {
		return false;
	}

	for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
		subscriber_t *subscriber = &uartDispatcher->subscribers[index];

		if (subscriber->subsystem != (subsystem_e)message->frame.subsystem ||
			subscriber->queue_sub == NULL) {
			continue;
		}

		if (xQueueSend(subscriber->queue_sub, message, 0U) == pdPASS) {
			delivered = true;
		}
	}

	if (message->frame.subsystem == SUBSYSTEM_COMM_TEST) {
		UARTDISPATCHER_sendAsync(uartDispatcher, message);
	}

	return delivered;
}

bool UARTDISPATCHER_sendAsync(uart_dispatcher_t *uartDispatcher,
							  const protocolData_u *message)
{
	uint8_t payload_size;

	if (uartDispatcher == NULL || !uartDispatcher->ready || message == NULL ||
		message->frame.startByte != (uint8_t)PROTOCOL_START_BYTE ||
		!PROTOCOL_payloadSizeToUint8(message->frame.payload_size, &payload_size) ||
		payload_size > PAYLOAD_MAX_SIZE) {
		return false;
	}

	return UARTCOMM_sendAsync(uartDispatcher->uartComm, message->rawData,
						  PROTOCOL_HEADER_LEN + payload_size);
}

bool UARTDISPATCHER_subscribe(uart_dispatcher_t *uartDispatcher,
							  const subscriber_t *subscriber)
{
	uint8_t available_index = MAX_SUBSCRIBER_QTY;

	if (uartDispatcher == NULL || !uartDispatcher->ready || subscriber == NULL ||
		subscriber->queue_sub == NULL ||
		!PROTOCOL_isValidSubsystem((uint8_t)subscriber->subsystem)) {
		return false;
	}

	for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
		// Keep the first available slot while checking for duplicates.
		if (uartDispatcher->subscribers[index].subsystem == SUBSYSTEM_NONE &&
			uartDispatcher->subscribers[index].queue_sub == NULL) {
			if (available_index == MAX_SUBSCRIBER_QTY) {
				available_index = index;
			}
			continue;
		}

		// Avoid repeated subscribers.
		if (uartDispatcher->subscribers[index].subsystem == subscriber->subsystem &&
			uartDispatcher->subscribers[index].queue_sub == subscriber->queue_sub) {
			return false;
		}
	}

	if (available_index >= MAX_SUBSCRIBER_QTY) {
		return false;
	}

	uartDispatcher->subscribers[available_index] = *subscriber;
	return true;
}

bool UARTDISPATCHER_unsubscribe(uart_dispatcher_t *uartDispatcher,
                              const subscriber_t *subscriber)
{
	if (uartDispatcher == NULL || !uartDispatcher->ready || subscriber == NULL ||
		subscriber->queue_sub == NULL ||
		!PROTOCOL_isValidSubsystem((uint8_t)subscriber->subsystem)) {
		return false;
	}

	for (uint8_t index = 0U; index < MAX_SUBSCRIBER_QTY; index++) {
		if (uartDispatcher->subscribers[index].subsystem == subscriber->subsystem &&
			uartDispatcher->subscribers[index].queue_sub == subscriber->queue_sub) {
				
			uartDispatcher->subscribers[index].subsystem = SUBSYSTEM_NONE;
			uartDispatcher->subscribers[index].queue_sub = NULL;
			return true;
		}
	}

	return false;
}
