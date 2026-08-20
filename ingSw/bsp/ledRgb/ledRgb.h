/* ******************************************************************************
  * @file           : pwm.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LEDRGB_H
#define __LEDRGB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "pwm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
typedef struct struct_pwm_t pwm_t;

typedef struct {
	uint8_t red;
    uint8_t green;
    uint8_t blue;
} color_s;

typedef struct {    
	color_s color;
	uint32_t timeout_ms;
} colorStep_t;

typedef struct
{
    colorStep_t *seq;
    uint8_t stepQty;
    uint8_t index;
} colorSequence;

typedef enum {
    LED_RGB_STATUS_1,
    LED_RGB_COUNT,
} ledRgb_id;

typedef enum {
    LED_RGB_COMMON_CATHODE,
    LED_RGB_COMMON_ANODE,
} ledRgb_elec_conn;

typedef struct
{
    pwm_t *pwm;
    colorSequence *colorSeq;
    ledRgb_id id;
    bool valid;
} ledRgb;

bool LED_RGB_ctor(ledRgb *led, ledRgb_id id, colorSequence *colSeq,
                  ledRgb_elec_conn connection);

bool LED_RGB_init(ledRgb *self);

bool LED_RGB_start(ledRgb *self);

bool LED_RGB_stop(ledRgb *self);

bool LED_RGB_setColorSequence(ledRgb *self, colorSequence *colSeq);

bool LED_RGB_setColor(ledRgb *self, uint8_t index);

bool LED_RGB_nextStep(ledRgb *self, bool cyclic);

#ifdef __cplusplus
}
#endif

#endif /* __LEDRGB_H */
