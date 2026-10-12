#ifndef __GPS_PARSER_RMC_H
#define __GPS_PARSER_RMC_H

/** \brief Parser for RMC messages.
 **
 ** \addtogroup todo?
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>
#include "gps_types.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

gsp_error_t GPS_PARSER_RMC_isValidMsg(const char *rmc_msg);

void GPS_PARSER_RMC_processMsg(const char *rmc_msg);

gsp_error_t GPS_PARSER_RMC_getPosition(const char *rmc_msg, gps_rmc_position_t *position);

gsp_error_t GPS_PARSER_RMC_getTime(const char *rmc_msg, gps_rmc_time_t *time);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __GPS_PARSER_RMC_H */
