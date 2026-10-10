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

void GPS_PARSER_RMC_processMsg(uint8_t *rmc_msg);

void GPS_PARSER_RMC_getPosition(uint8_t *rmc_msg);

void GPS_PARSER_RMC_getTime(uint8_t *rmc_msg);

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __GPS_PARSER_RMC_H */
