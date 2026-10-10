
/** \brief Parser for RMC messages.
 **
 ** \addtogroup todo?
 ** @{ */

/* === Header includes ===================================================================== */

#include "gps_parser_rmc.h"
#include <string.h>

/* === Definitions and macros ============================================================== */
#define VALIDATION_POS 17
/* === Internal data type declarations ===================================================== */

/* === Internal function declarations ====================================================== */

/* === Internal variable definitions ======================================================= */

/* === External variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */

/* === External function definitions ======================================================= */

gsp_error_t GPS_PARSER_RMC_isValidMsg(char *rmc_msg) {
    if (!rmc_msg || strlen(rmc_msg) < VALIDATION_POS) {
        return ERROR_VALIDATION_FAIL;
    }
    if (rmc_msg[VALIDATION_POS] == 'A') {
        return ERROR_OK;
    } else if (rmc_msg[VALIDATION_POS] == 'V') {
        return ERROR_VALIDATION_FAIL;
    } else {
        return ERROR_VALIDATION_UNKNOW;
    }
}

void GPS_PARSER_RMC_processMsg(char *rmc_msg) {
}

gsp_error_t GPS_PARSER_RMC_getPosition(char *rmc_msg, gps_rmc_position_t *position) {
    gps_rmc_position_t pos = {0};
    pos.latitude = -32.94165000;

    position->latitude = pos.latitude;

    return ERROR_OK;
}

void GPS_PARSER_RMC_getTime(char *rmc_msg) {
}

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */
