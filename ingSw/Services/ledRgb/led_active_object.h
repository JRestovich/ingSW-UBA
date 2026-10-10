#ifndef LED_ACTIVE_OBJECT_H_
#define LED_ACTIVE_OBJECT_H_

/** \file led_active_object.h
 ** \brief Event-driven RGB LED interface.
 **
 ** \addtogroup LEDActiveObject
 ** @{ */

/* === External file includes ============================================================== */

#include "active_object.h"
#include "ledRgb.h"
#include "timers.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

typedef enum
{
    EV_LED_OFF,
    EV_LED_ON,
    EV_LED_BLINK,
    EV_LED_TIMEOUT,
    EV_LED_NONE
} led_ev_t;

typedef enum
{
    ST_LED_OFF,
    ST_LED_ON,
    ST_LED_BLINK
} led_st_t;

typedef struct
{
    led_st_t state;
    led_ev_t ev_in;
    TickType_t tick;
} led_sc_t;

typedef struct
{
    ledRgb led;
    led_sc_t led_sc;
    active_object_t ao;
    TimerHandle_t h_timer;
} h_led_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

void led_ao_open(h_led_t *self,
                 ledRgb_id id,
                 colorSequence *color_sequence,
                 ledRgb_elec_conn connection);
void led_ao_release(h_led_t *self);
BaseType_t led_ao_send(h_led_t *self, const led_ev_t *event);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* LED_ACTIVE_OBJECT_H_ */
