
/** \brief Parser for RMC messages.
 **
 ** \addtogroup todo?
 ** @{ */

/* === Header includes ===================================================================== */

#include "gps_parser_rmc.h"
#include <string.h>

/* === Definitions and macros ============================================================== */

/* === Internal data type declarations ===================================================== */

/* === Internal function declarations ====================================================== */

/* === Internal variable definitions ======================================================= */

/* === External variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */

/* === External function definitions ======================================================= */

gsp_error_t GPS_PARSER_RMC_isValidMsg(char *rmc_msg) {
    if (!rmc_msg) {
        return ERROR_VALIDATION_FAIL;
    }
    return ERROR_OK;
}

void GPS_PARSER_RMC_processMsg(char *rmc_msg) {
}

void GPS_PARSER_RMC_getPosition(char *rmc_msg) {
}

void GPS_PARSER_RMC_getTime(char *rmc_msg) {
}

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */
