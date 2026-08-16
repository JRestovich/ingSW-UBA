/* ******************************************************************************
  * @file           : pwm.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
typedef struct struct_pwm_t pwm_t;

typedef enum {
	TIMER1,
	TIMER2,
	TIMER3,
	TIMER4,
  TIMER_COUNT
} timer_num;

typedef enum {
	PWM_CHANNEL_1,
	PWM_CHANNEL_2,
	PWM_CHANNEL_3,
	PWM_CHANNEL_4,
} pwm_channel;

typedef enum {
	PWM_POLARITY_HIGH,
	PWM_POLARITY_LOW,
} pwm_polarity;

pwm_t* PWM_ctor(timer_num tim);

bool PWM_hasChannel(pwm_t *self, pwm_channel channel);

bool PWM_init(pwm_t *self);

bool PWM_start(pwm_t *self, pwm_channel channel);

bool PWM_stop(pwm_t *self, pwm_channel channel);

bool PWM_setDuty(pwm_t *self, pwm_channel channel, uint8_t duty_pcnt);

bool PWM_setPolarity(pwm_t *self, pwm_channel channel, pwm_polarity polarity);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
