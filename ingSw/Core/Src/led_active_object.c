#include "led_active_object.h"

#define LED_AO_QUEUE_LENGTH       8U
#define LED_AO_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define LED_AO_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U)
#define LED_AO_TIMEOUT_PERIOD     pdMS_TO_TICKS(1U)

static void task_led(void *parameters);
static void task_led_statechart(h_led_t *self);
static void led_ao_timeout_callback(TimerHandle_t timer);
static void led_ao_stop_timer(h_led_t *self);
static void led_ao_schedule_timeout(h_led_t *self);

void led_ao_open(h_led_t *self,
                 ledRgb_id id,
                 colorSequence *color_sequence,
                 ledRgb_elec_conn connection)
{
    BaseType_t result;
    bool led_created;
    bool led_initialized;

    configASSERT(self != NULL);
    if (self == NULL)
    {
        return;
    }

    led_created = LED_RGB_ctor(&self->led, id, color_sequence, connection);
    configASSERT(led_created);

    led_initialized = LED_RGB_init(&self->led);
    configASSERT(led_initialized);

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
    if (self == NULL)
    {
        return;
    }

    if (self->ao.h_task != NULL)
    {
        vTaskDelete(self->ao.h_task);
        self->ao.h_task = NULL;
    }

    if (self->h_timer != NULL)
    {
        (void)xTimerStop(self->h_timer, 0U);
        (void)xTimerDelete(self->h_timer, 0U);
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
    if (self == NULL || event == NULL || self->ao.h_queue == NULL)
    {
        return pdFAIL;
    }

    return xQueueSend(self->ao.h_queue, event, 0U);
}

static void led_ao_timeout_callback(TimerHandle_t timer)
{
    h_led_t *self = pvTimerGetTimerID(timer);
    const led_ev_t event = EV_LED_TIMEOUT;

    if (self != NULL && self->ao.h_queue != NULL)
    {
        (void)xQueueSend(self->ao.h_queue, &event, 0U);
    }
}

static void task_led(void *parameters)
{
    h_led_t *self = parameters;
    led_ev_t event;

    if (self == NULL)
    {
        vTaskDelete(NULL);
        return;
    }

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
    switch (self->led_sc.ev_in)
    {
        case EV_LED_OFF:
            led_ao_stop_timer(self);
            if (LED_RGB_stop(&self->led))
            {
                self->led_sc.state = ST_LED_OFF;
            }
            break;

        case EV_LED_ON:
            led_ao_stop_timer(self);
            if (LED_RGB_start(&self->led) && LED_RGB_setColor(&self->led, 0U))
            {
                self->led_sc.state = ST_LED_ON;
            }
            break;

        case EV_LED_BLINK:
            if (self->led_sc.state != ST_LED_BLINK)
            {
                if (LED_RGB_start(&self->led) && LED_RGB_nextStep(&self->led, true))
                {
                    self->led_sc.state = ST_LED_BLINK;
                }
            }
            if (self->led_sc.state == ST_LED_BLINK)
            {
                led_ao_schedule_timeout(self);
            }
            break;

        case EV_LED_TIMEOUT:
            if (self->led_sc.state == ST_LED_BLINK)
            {
                if (LED_RGB_nextStep(&self->led, true))
                {
                    led_ao_schedule_timeout(self);
                }
            }
            break;

        case EV_LED_NONE:
        default:
            break;
    }

    self->led_sc.ev_in = EV_LED_NONE;
}

static void led_ao_stop_timer(h_led_t *self)
{
    if (self->h_timer != NULL)
    {
        (void)xTimerStop(self->h_timer, 0U);
    }
}

static void led_ao_schedule_timeout(h_led_t *self)
{
    TickType_t period;

    if (self->h_timer == NULL || self->led.colorSeq == NULL ||
        self->led.colorSeq->seq == NULL || self->led.colorSeq->stepQty == 0U)
    {
        return;
    }

    period = pdMS_TO_TICKS(
        self->led.colorSeq->seq[self->led.colorSeq->index].timeout_ms);
    if (period == 0U)
    {
        period = 1U;
    }

    /* xTimerChangePeriod also starts/restarts the one-shot timer. */
    (void)xTimerChangePeriod(self->h_timer, period, 0U);
}
