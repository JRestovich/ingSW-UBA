#ifndef LED_ACTIVE_OBJECT_H_
#define LED_ACTIVE_OBJECT_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "active_object.h"
#include "ledRgb.h"
#include "timers.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/
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

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
void led_ao_open(h_led_t *self);
void led_ao_release(h_led_t *self);
BaseType_t led_ao_send(h_led_t *self, const led_ev_t *event);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* LED_ACTIVE_OBJECT_H_ */
/********************** end of file ******************************************/
