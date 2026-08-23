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
	QueueHandle_t	queue_rx;

	uint8_t				ring_storage[UART_RX_RING_BUFFER_LEN];
	ring_buffer_t 		ring_rx;
	SemaphoreHandle_t	sem_rx_byte_avail;
	uint8_t				rx_buffer[UART_RX_RING_BUFFER_LEN];
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

	ring_buffer_init(&uartDriver->ring_rx, (char*)uartDriver->ring_storage, UART_RX_RING_BUFFER_LEN);

	uartDriver->sem_rx_byte_avail = xSemaphoreCreateCounting((UART_RX_RING_BUFFER_LEN - 1), 0ul);
	if (NULL == uartDriver->sem_rx_byte_avail) {
    	return false;
    }
	vQueueAddToRegistry(uartDriver->sem_rx_byte_avail, "Task UART byte available semaphore");

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
    (void)uartDriver;
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

bool UARTCOMM_readAsync(uart_driver_t *uartDriver, uint8_t *pData, uint16_t size)
{
    uint16_t received = 0U;

    if (uartDriver == NULL || !uartDriver->ready || pData == NULL || size == 0U) {
        return false;
    }

    while (received < size &&
           xSemaphoreTake(uartDriver->sem_rx_byte_avail, 0U) == pdPASS) {
        if (!ring_buffer_dequeue(&uartDriver->ring_rx, (char *)&pData[received])) {
            break;
        }
        received++;
    }

    return received > 0U;
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

    if (!UART_readAsync(uartDriver->uartInterface, uartDriver->rx_buffer,
                        sizeof(uartDriver->rx_buffer))) {
        vTaskDelete(NULL);
    }

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		uint16_t size;

		xSemaphoreTake(uartDriver->sem_rx_cplt_cb, portMAX_DELAY);
		size = uartDriver->rx_size;
		ring_buffer_queue_arr(&uartDriver->ring_rx,
							  (const char *)uartDriver->rx_buffer, size);
		for (uint16_t i = 0U; i < size; i++) {
			(void)xSemaphoreGive(uartDriver->sem_rx_byte_avail);
		}

		if (!UART_readAsync(uartDriver->uartInterface, uartDriver->rx_buffer,
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
