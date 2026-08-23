#include "uartComm.h"

#include <string.h>

#define UART_TX_QUEUE_LEN 8U

static bool _open(uart_driver_t *uartDriver, uart_num uartNum);
static void UART_rxCompleteCallback(uint16_t size, void *context);
static void UART_txCompleteCallback(void *context);
static void task_uart_rx(void *parameters);
static void task_uart_tx(void *parameters);

struct uart_driver {
	bool ready;
	uart_num	id;
    uart_t  *uartInterface;

	TaskHandle_t	task_tx;
	QueueHandle_t	queue_tx;

	TaskHandle_t	task_rx;
	uint8_t				rx_stream_storage[UART_RX_STREAM_BUFFER_LEN];
	StaticStreamBuffer_t	rx_stream_control;
	StreamBufferHandle_t	rx_stream;
	uint8_t				rx_buffer[UART_RX_CHUNK_LEN];
	volatile uint16_t	rx_size;

	/* binary semaphores signaled from the ISR */
	SemaphoreHandle_t	sem_tx_cplt_cb;
	SemaphoreHandle_t	sem_rx_cplt_cb;
};

static uart_driver_t uartDrivers[UART_COUNT] = {0};

uart_driver_t* UARTCOMM_get(uart_num uartNum) {
	if (uartNum >= UART_COUNT) {
		return NULL;
	}

    if (!_open(&uartDrivers[uartNum], uartNum)) {
        return NULL;
    }
    return &uartDrivers[uartNum];
}

static bool _open(uart_driver_t *uartDriver, uart_num uartNum) {
	if (uartDriver == NULL || uartNum >= UART_COUNT) {
		return false;
	}

    if (uartDriver->ready) {
        return true;
    }

    BaseType_t ret;

    uartDriver->id = uartNum;
    uartDriver->uartInterface = UART_ctor(uartNum);

    if (uartDriver->uartInterface == NULL ||
        !UART_init(uartDriver->uartInterface, BAUDRATE_115200, ONE_STOP_BIT, NONE))
    {
        return false;
    }

    /* Before a queue is used it must be explicitly created.
	 * Check the queue was created successfully.
     * Add queue to registry. */
	uartDriver->queue_tx = xQueueCreate(UART_TX_QUEUE_LEN, sizeof(uart_msg_t));
	if (NULL == uartDriver->queue_tx) {
    	return false;
    }
	vQueueAddToRegistry(uartDriver->queue_tx, "Task UART Tx Queue Handle");

	uartDriver->rx_stream = xStreamBufferCreateStatic(
		UART_RX_STREAM_BUFFER_LEN, 1U, uartDriver->rx_stream_storage,
		&uartDriver->rx_stream_control);
	if (uartDriver->rx_stream == NULL) {
		return false;
	}

	uartDriver->sem_tx_cplt_cb = xSemaphoreCreateBinary();
	if (NULL == uartDriver->sem_tx_cplt_cb) {
    	return false;
    }
	vQueueAddToRegistry(uartDriver->sem_tx_cplt_cb, "Task UART TX Complete callback Semaphore");

	uartDriver->sem_rx_cplt_cb = xSemaphoreCreateBinary();
	if (NULL == uartDriver->sem_rx_cplt_cb) {
    	return false;
    }
	vQueueAddToRegistry(uartDriver->sem_rx_cplt_cb, "Task UART RX Complete callback Semaphore");

    if (!UART_setRxCallback(uartDriver->uartInterface, UART_rxCompleteCallback,
                            uartDriver) ||
        !UART_setTxCallback(uartDriver->uartInterface, UART_txCompleteCallback,
                            uartDriver)) {
        return false;
    }

    /* Before a task is executed it must be explicitly created.
	 * Check the task was created successfully. */
    ret = xTaskCreate(task_uart_tx, "Task UART Tx", (configMINIMAL_STACK_SIZE),
					  (void *)uartDriver,
					  (tskIDLE_PRIORITY + 1ul), &uartDriver->task_tx);
    if (pdPASS != ret) {
    	return false;
    }

    ret = xTaskCreate(task_uart_rx, "Task UART Rx", (configMINIMAL_STACK_SIZE),
    				  (void *)uartDriver,
					  (tskIDLE_PRIORITY + 1ul), &uartDriver->task_rx);
    if (pdPASS != ret) {
    	return false;
    }

    uartDriver->ready = true;

    return true;
}

void UARTCOMM_close(uart_driver_t *uartDriver)
{
	if (uartDriver == NULL || !uartDriver->ready) {
		return;
	}

	(void)UART_setRxCallback(uartDriver->uartInterface, NULL, NULL);
	(void)UART_setTxCallback(uartDriver->uartInterface, NULL, NULL);
	uartDriver->ready = false;

	if (uartDriver->task_rx != NULL) {
		vTaskDelete(uartDriver->task_rx);
		uartDriver->task_rx = NULL;
	}
	if (uartDriver->task_tx != NULL) {
		vTaskDelete(uartDriver->task_tx);
		uartDriver->task_tx = NULL;
	}
	if (uartDriver->queue_tx != NULL) {
		vQueueDelete(uartDriver->queue_tx);
		uartDriver->queue_tx = NULL;
	}
	if (uartDriver->sem_tx_cplt_cb != NULL) {
		vSemaphoreDelete(uartDriver->sem_tx_cplt_cb);
		uartDriver->sem_tx_cplt_cb = NULL;
	}
	if (uartDriver->sem_rx_cplt_cb != NULL) {
		vSemaphoreDelete(uartDriver->sem_rx_cplt_cb);
		uartDriver->sem_rx_cplt_cb = NULL;
	}

	uartDriver->rx_stream = NULL;
	uartDriver->rx_size = 0U;
	uartDriver->uartInterface = NULL;
}

bool UARTCOMM_sendAsync(uart_driver_t *uartDriver, const uint8_t *pData, uint16_t size)
{
    uart_msg_t message = {0};

    if (uartDriver == NULL || !uartDriver->ready || pData == NULL || size == 0U ||
        size > UART_TX_MAX_PAYLOAD) {
        return false;
    }

    memcpy(message.data, pData, size);
    message.length = size;

    if (xQueueSend(uartDriver->queue_tx, &message, 0U) != pdPASS) {
        return false;
    }

    return true;
}

StreamBufferHandle_t UARTCOMM_getRxStream(const uart_driver_t *uartDriver)
{
    if (uartDriver == NULL || !uartDriver->ready) {
        return NULL;
    }

    return uartDriver->rx_stream;
}

/* Task UART TX thread */
static void task_uart_tx(void *parameters)
{
	uart_driver_t *uartDriver = (uart_driver_t*)parameters;
    uart_msg_t uart_msg;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		xQueueReceive(uartDriver->queue_tx, &uart_msg, portMAX_DELAY);

        bool ret = UART_sendAsync(uartDriver->uartInterface, uart_msg.data, uart_msg.length);

		if(!ret) {
			continue;
		}

		xSemaphoreTake(uartDriver->sem_tx_cplt_cb, portMAX_DELAY);
	}
}

/* Task UART RX thread */
static void task_uart_rx(void *parameters)
{
	uart_driver_t *uartDriver = (uart_driver_t*)parameters;

	while (!UART_readAsync(uartDriver->uartInterface, uartDriver->rx_buffer,
									 sizeof(uartDriver->rx_buffer))) {
		vTaskDelay(pdMS_TO_TICKS(1U));
	}

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		uint16_t size;

		xSemaphoreTake(uartDriver->sem_rx_cplt_cb, portMAX_DELAY);
		size = uartDriver->rx_size;
		(void)xStreamBufferSend(uartDriver->rx_stream, uartDriver->rx_buffer, size, 0U);

		while (!UART_readAsync(uartDriver->uartInterface, uartDriver->rx_buffer,
									 sizeof(uartDriver->rx_buffer))) {
			vTaskDelay(pdMS_TO_TICKS(1U));
		}
	}
}

static void UART_rxCompleteCallback(uint16_t size, void *context)
{
    uart_driver_t *uartDriver = context;
    BaseType_t higher_priority_task_woken = pdFALSE;

    uartDriver->rx_size = size;
    xSemaphoreGiveFromISR(uartDriver->sem_rx_cplt_cb, &higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void UART_txCompleteCallback(void *context)
{
    uart_driver_t *uartDriver = context;
    BaseType_t higher_priority_task_woken = pdFALSE;

    xSemaphoreGiveFromISR(uartDriver->sem_tx_cplt_cb, &higher_priority_task_woken);

    portYIELD_FROM_ISR(higher_priority_task_woken);
}
