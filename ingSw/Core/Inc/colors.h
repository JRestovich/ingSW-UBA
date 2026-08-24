/**
 * @file app.h
 * @brief Interfaz de la maquina de estados principal de la aplicacion.
 *
 * Este modulo declara la API publica de inicializacion y ejecucion del flujo
 * principal de la aplicacion, junto con los tipos asociados al estado general
 * y al registro de errores de inicializacion de los perifericos utilizados.
 */

#ifndef _COLORS_H_
#define _COLORS_H_

/********************************************************/
/* Includes */
#include <stdbool.h>
#include <stdint.h>
#include "ledRgb.h"

/********************************************************/

#define COLOR_RED   { .color = { 255U,   0U,   0U }, .timeout_ms = 2000U }
#define COLOR_GREEN { .color = {   0U, 255U,   0U }, .timeout_ms = 2000U }
#define COLOR_BLUE  { .color = {   0U,   0U, 255U }, .timeout_ms = 2000U }
#define COLOR_OFF   { .color = {   0U,   0U,   0U }, .timeout_ms = 2000U }

static colorStep_t app_normal_steps[] = {
  COLOR_GREEN,
};

static colorStep_t app_pet_lost_steps[] = {
  COLOR_RED,
  COLOR_OFF,
};

static colorStep_t app_connection_lost_steps[] = {
  COLOR_BLUE,
  COLOR_OFF,
};

static colorStep_t app_error_steps[] = {
  COLOR_RED,
  COLOR_BLUE,
  COLOR_OFF,
};

static colorSequence app_normal_sequence = {
  .seq = app_normal_steps,
  .stepQty = sizeof(app_normal_steps) / sizeof(app_normal_steps[0]),
  .index = 0,
};

static colorSequence app_pet_lost_sequence = {
  .seq = app_pet_lost_steps,
  .stepQty = sizeof(app_pet_lost_steps) / sizeof(app_pet_lost_steps[0]),
  .index = 0,
};

static colorSequence app_connection_lost_sequence = {
  .seq = app_connection_lost_steps,
  .stepQty = sizeof(app_connection_lost_steps) / sizeof(app_connection_lost_steps[0]),
  .index = 0,
};

static colorSequence app_error_sequence = {
  .seq = app_error_steps,
  .stepQty = sizeof(app_error_steps) / sizeof(app_error_steps[0]),
  .index = 0,
};

#endif /* _COLORS_H_ */
