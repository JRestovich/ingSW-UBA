#ifndef __GPS_TYPES_H
#define __GPS_TYPES_H

/** \brief Data types needed by the GPS module.
 **
 ** \addtogroup todo?
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

typedef struct {
    double latitude;
    double longitude;
} gps_rmc_position_t;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} gps_rmc_time_t;

typedef struct {
    uint8_t day;
    uint8_t month;
    uint8_t year;
} gps_rmc_date_t;

typedef struct {
    gps_rmc_position_t position;
    gps_rmc_time_t time;
    gps_rmc_date_t date;
} gps_rmc_data_t;

typedef enum {
    ERROR_OK,
    ERROR_VALIDATION_FAIL,
    ERROR_VALIDATION_UNKNOW
} gsp_error_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __GPS_TYPES_H */
