#include "app.h"
#include "colors.h"

#define APP_QUEUE_SIZE 5

static void appTaskHandler(void *parameters);
static void appApplyStateSequence(app_t *app);

void APP_init(app_t *app)
{
    if (app == NULL)
    {
        return;
    }

    app->state = APP_NORMAL;
    app->appTask = NULL;
    app->appQueue = NULL;
    app->uartDispatcher = NULL;
    app->appSubscriber.subsystem = SUBSYSTEM_LED_RGB;
    app->appSubscriber.queue_sub = NULL;

    led_ao_open(&app->statusLed,
                LED_RGB_STATUS_1,
                &app_normal_sequence,
                LED_RGB_COMMON_CATHODE);

    appApplyStateSequence(app);

    app->uartDispatcher = UARTDISPATCHER_get(UART_2);
    if (app->uartDispatcher == NULL)
    {
        // ERROR
        return;
    }

    app->appQueue = xQueueCreate(APP_QUEUE_SIZE, sizeof(protocolData_u));
    if (app->appQueue == NULL)
    {
        // ERROR
        return;
    }

    app->appSubscriber.queue_sub = app->appQueue;

    BaseType_t result = xTaskCreate(appTaskHandler,
                                    "App",
                                    configMINIMAL_STACK_SIZE,
                                    app,
                                    tskIDLE_PRIORITY + 1U,
                                    &app->appTask);

    if (result != pdPASS)
    {
        // ERROR
        return;
    }
    
    if (!UARTDISPATCHER_subscribe(app->uartDispatcher, &app->appSubscriber))
    {
        // ERROR
        return;
    }
}

static void appTaskHandler(void *parameters)
{
    app_t *app = parameters;
    protocolData_u data = {0};

    if (app == NULL || app->appQueue == NULL)
    {
        vTaskDelete(NULL);
        return;
    }

    for (;;)
    {
        appState previousState;

        if (xQueueReceive(app->appQueue, &data, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

        previousState = app->state;

        switch (app->state)
        {
            case APP_NORMAL:
                if (data.frame.cmd == APP_CMD_NEXT_STATE)
                {
                    app->state = APP_PET_LOST;
                }
                else if (data.frame.cmd == APP_CMD_PREVIOUS_STATE)
                {
                    app->state = APP_CONNECTION_LOST;
                }
                else if (data.frame.cmd == APP_CMD_ERROR)
                {
                    app->state = APP_ERROR;
                }
                break;

            case APP_PET_LOST:
                if (data.frame.cmd == APP_CMD_NEXT_STATE)
                {
                    app->state = APP_CONNECTION_LOST;
                }
                else if (data.frame.cmd == APP_CMD_PREVIOUS_STATE)
                {
                    app->state = APP_NORMAL;
                }
                else if (data.frame.cmd == APP_CMD_ERROR)
                {
                    app->state = APP_ERROR;
                }
                break;

            case APP_CONNECTION_LOST:
                if (data.frame.cmd == APP_CMD_NEXT_STATE)
                {
                    app->state = APP_NORMAL;
                }
                else if (data.frame.cmd == APP_CMD_PREVIOUS_STATE)
                {
                    app->state = APP_PET_LOST;
                }
                else if (data.frame.cmd == APP_CMD_ERROR)
                {
                    app->state = APP_ERROR;
                }
                break;

            case APP_ERROR:
                if (data.frame.cmd == APP_CMD_RECOVERY)
                {
                    app->state = APP_NORMAL;
                }
                break;

            default:
                app->state = APP_ERROR;
                break;
        }

        if (app->state != previousState)
        {
            appApplyStateSequence(app);
        }
    }
}

static void appApplyStateSequence(app_t *app)
{
    colorSequence *sequence;
    const led_ev_t event = EV_LED_BLINK;

    if (app == NULL)
    {
        return;
    }

    switch (app->state)
    {
        case APP_NORMAL:
            sequence = &app_normal_sequence;
            break;

        case APP_PET_LOST:
            sequence = &app_pet_lost_sequence;
            break;

        case APP_CONNECTION_LOST:
            sequence = &app_connection_lost_sequence;
            break;

        case APP_ERROR:
        default:
            sequence = &app_error_sequence;
            break;
    }

    taskENTER_CRITICAL();
    (void)LED_RGB_setColorSequence(&app->statusLed.led, sequence);
    taskEXIT_CRITICAL();

    (void)led_ao_send(&app->statusLed, &event);
}
