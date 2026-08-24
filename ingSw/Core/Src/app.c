#include "app.h"

#define APP_QUEUE_SIZE 5

static void appTaskHandler(void *parameters);

static colorStep_t rgb_steps[] = {
  { .color = { 0, 0, 0 }, .timeout_ms = 3000 },
  { .color = { 66, 66, 66 }, .timeout_ms = 3000 },
  { .color = { 100, 100, 100 }, .timeout_ms = 3000 },
};

static colorSequence rgb_sequence = {
  .seq = rgb_steps,
  .stepQty = sizeof(rgb_steps) / sizeof(rgb_steps[0]),
  .index = 0,
};

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
                &rgb_sequence,
                LED_RGB_COMMON_CATHODE);

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
        if (xQueueReceive(app->appQueue, &data, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

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
    }
}
