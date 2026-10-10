
/** \brief Parser for RMC messages.
 **
 ** \addtogroup todo?
 ** @{ */

/* === Header includes ===================================================================== */

#include "gps_parser_rmc.h"
#include <stdlib.h>
#include <string.h>

/* === Definitions and macros ============================================================== */
#define VALIDATION_POS 17
/* === Internal data type declarations ===================================================== */

/* === Internal function declarations ====================================================== */
gsp_error_t _getLatitude(char *rmc_msg, double *lat);

/* === Internal variable definitions ======================================================= */

/* === External variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */
gsp_error_t _getLatitude(char *rmc_msg, double *lat) {
    char deg[3] = {0};
    char min[7] = {0};
    char *end;
    double dec, uni;

    if (!rmc_msg || !lat) {
        return ERROR_POSITION_INVALID;
    }

    if (strlen(rmc_msg) <= 28) { // largo total
        return ERROR_VALIDATION_FAIL;
    }

    dec = (rmc_msg[19] - '0') * 10;
    uni = rmc_msg[20] - '0';

    strncpy(deg, &rmc_msg[19], 2);
    double degD = strtod(deg, &end);

    strncpy(min, &rmc_msg[21], 6);
    double minD = strtod(min, &end);

    double sign;
    if (rmc_msg[28] == 'N') {
        sign = 1;
    } else if (rmc_msg[28] == 'S') {
        sign = -1;
    } else {
        return ERROR_POSITION_INVALID;
    }

    *lat = sign * (degD + minD / 60.0);
    return ERROR_OK;
}

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
    gsp_error_t err = GPS_PARSER_RMC_isValidMsg(rmc_msg);

    if (err != ERROR_OK) {
        return err;
    }

    err = _getLatitude(rmc_msg, &pos.latitude);
    if (err != ERROR_OK) {
        return err;
    }

    position->latitude = pos.latitude;

    return ERROR_OK;
}

void GPS_PARSER_RMC_getTime(char *rmc_msg) {
}

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */
