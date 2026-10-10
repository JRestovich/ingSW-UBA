#ifndef _COLORS_H_
#define _COLORS_H_

/** \file colors.h
 ** \brief RGB color sequences for application states.
 **
 ** \addtogroup ApplicationColors
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>
#include "ledRgb.h"

/* === C++ linkage ========================================================================= */

/* === Definitions and macros ============================================================== */

#define COLOR_RED   { .color = { 255U,   0U,   0U }, .timeout_ms = 2000U }
#define COLOR_GREEN { .color = {   0U, 255U,   0U }, .timeout_ms = 2000U }
#define COLOR_BLUE  { .color = {   0U,   0U, 255U }, .timeout_ms = 2000U }
#define COLOR_OFF   { .color = {   0U,   0U,   0U }, .timeout_ms = 2000U }

/* === Data type declarations ============================================================== */

/* === Internal variable definitions ======================================================= */

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

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */

#endif /* _COLORS_H_ */
