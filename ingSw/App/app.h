#ifndef _APP_H_
#define _APP_H_

/** \file app.h
 ** \brief Application state machine interface.
 **
 ** \addtogroup Application
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>
#include "led_active_object.h"
#include "uartDispatcher.h"

/* === C++ linkage ========================================================================= */

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

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
    APP_CMD_RECOVERY = 'R',
    APP_CMD_STATE_RESPONSE = 'S'
} appCommand_t;

typedef struct {
    appState state;

    h_led_t statusLed;

    uart_dispatcher_t *uartDispatcher;
    subscriber_t appSubscriber;

    TaskHandle_t appTask;
    QueueHandle_t appQueue;
} app_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

void APP_init(app_t *app);

void APP_engine(void);

uint8_t APP_getError(void);

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */

#endif /* _APP_H_ */
