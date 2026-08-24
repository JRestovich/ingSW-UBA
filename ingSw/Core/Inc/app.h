/**
 * @file app.h
 * @brief Interfaz de la maquina de estados principal de la aplicacion.
 *
 * Este modulo declara la API publica de inicializacion y ejecucion del flujo
 * principal de la aplicacion, junto con los tipos asociados al estado general
 * y al registro de errores de inicializacion de los perifericos utilizados.
 */

#ifndef _APP_H_
#define _APP_H_

/********************************************************/
/* Includes */
#include <stdbool.h>
#include <stdint.h>
#include "led_active_object.h"
#include "uartDispatcher.h"

/********************************************************/

typedef enum {
    APP_NORMAL,
    APP_PET_LOST,
    APP_CONNECTION_LOST,
    APP_ERROR
} appState;

typedef enum {
    APP_CMD_NEXT_STATE = 'N',
    APP_CMD_PREVIOUS_STATE = 'P',
    APP_CMD_ERROR = 'E',
    APP_CMD_RECOVERY = 'R'
} appCommand_t;

typedef struct
{
    appState state;

    h_led_t statusLed;

    uart_dispatcher_t *uartDispatcher;
    subscriber_t appSubscriber;

    TaskHandle_t appTask;
    QueueHandle_t appQueue;
} app_t;

void APP_init(app_t *app);

void APP_engine(void);

uint8_t APP_getError(void);

#endif /* _APP_H_ */
