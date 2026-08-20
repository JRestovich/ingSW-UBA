#include "led_active_object.h"

#define LED_AO_QUEUE_LENGTH       8U
#define LED_AO_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define LED_AO_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U)
#define LED_AO_TIMEOUT_PERIOD     pdMS_TO_TICKS(500U)

static void task_led(void *parameters);
static void task_led_statechart(h_led_t *self);
static void led_ao_timeout_callback(TimerHandle_t timer);

void led_ao_open(h_led_t *self)
{
    BaseType_t result;

    configASSERT(self != NULL);

    self->ao.h_queue = xQueueCreate(LED_AO_QUEUE_LENGTH, sizeof(led_ev_t));
    configASSERT(self->ao.h_queue != NULL);

    vQueueAddToRegistry(self->ao.h_queue, self->ao.queue_txt);

    self->h_timer = xTimerCreate("ledTimer",
                                 LED_AO_TIMEOUT_PERIOD,
                                 pdFALSE,
                                 self,
                                 led_ao_timeout_callback);
    configASSERT(self->h_timer != NULL);

    result = xTaskCreate(task_led,
                         self->ao.task_txt,
                         LED_AO_TASK_STACK_SIZE,
                         self,
                         LED_AO_TASK_PRIORITY,
                         &self->ao.h_task);
    configASSERT(result == pdPASS);
}

void led_ao_release(h_led_t *self)
{
    configASSERT(self != NULL);

    if (self->ao.h_task != NULL)
    {
        vTaskDelete(self->ao.h_task);
        self->ao.h_task = NULL;
    }

    if (self->h_timer != NULL)
    {
        (void)xTimerStop(self->h_timer, 0U);
        configASSERT(xTimerDelete(self->h_timer, 0U) == pdPASS);
        self->h_timer = NULL;
    }

    if (self->ao.h_queue != NULL)
    {
        vQueueUnregisterQueue(self->ao.h_queue);
        vQueueDelete(self->ao.h_queue);
        self->ao.h_queue = NULL;
    }
}

BaseType_t led_ao_send(h_led_t *self, const led_ev_t *event)
{
    configASSERT(self != NULL);
    configASSERT(event != NULL);

    return xQueueSend(self->ao.h_queue, event, 0U);
}

static void led_ao_timeout_callback(TimerHandle_t timer)
{
    h_led_t *self = pvTimerGetTimerID(timer);
    const led_ev_t event = EV_LED_TIMEOUT;

    configASSERT(self != NULL);
    (void)xQueueSend(self->ao.h_queue, &event, 0U);
}

static void task_led(void *parameters)
{
    h_led_t *self = parameters;
    led_ev_t event;

    configASSERT(self != NULL);

    self->led_sc.state = ST_LED_OFF;
    self->led_sc.ev_in = EV_LED_NONE;
    self->led_sc.tick = 0U;

    for (;;)
    {
        /* The task sleeps until an event is posted to its private queue. */
        if (xQueueReceive(self->ao.h_queue, &event, portMAX_DELAY) == pdPASS)
        {
            self->led_sc.ev_in = event;
            task_led_statechart(self);
        }
    }
}

static void task_led_statechart(h_led_t *self)
{
    /* Add state-transition and LED-output logic using self->led_sc/self->led. */
    (void)self;
}
