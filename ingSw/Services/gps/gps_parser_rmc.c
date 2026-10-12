
/** \brief Parser for RMC messages.
 **
 ** \addtogroup todo?
 ** @{ */

/* === Header includes ===================================================================== */

#include "gps_parser_rmc.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* === Definitions and macros ============================================================== */
#define VALIDATION_POS            17
#define LATITUDE_DEGREES_POS      19
#define LATITUDE_MINUTES_POS      21
#define LATITUDE_HEMISPHERE_POS   28
#define LONGITUDE_DEGREES_POS     30
#define LONGITUDE_MINUTES_POS     33
#define LONGITUDE_HEMISPHERE_POS  40
#define LATITUDE_DEGREES_LENGTH   2
#define LONGITUDE_DEGREES_LENGTH  3
#define COORDINATE_MINUTES_LENGTH 6
#define STRING_TERMINATOR_LENGTH  1
#define MINUTES_PER_DEGREE        60.0
#define POSITIVE_COORDINATE_SIGN  1.0
#define NEGATIVE_COORDINATE_SIGN  (-1.0)
#define MAX_LATITUDE              (90.0)
#define MIN_LATITUDE              (-90.0)
#define MAX_LONGITUDE             (180)
#define MIN_LONGITUDE             (-180)

#define HOUR_POS                  6
#define HOUR_LENGTH               2
#define MAX_HOUR                  23

/* === Internal data type declarations ===================================================== */

/* === Internal function declarations ====================================================== */
gsp_error_t _getLatitude(const char *rmc_msg, double *lat);
gsp_error_t _getLongitude(const char *rmc_msg, double *lon);

gsp_error_t _getHours(const char *rmc_msg, uint8_t *hour);
gsp_error_t _getMinutes(const char *rmc_msg, uint8_t *hour);
gsp_error_t _getSeconds(const char *rmc_msg, uint8_t *hour);
gsp_error_t _getMiliseconds(const char *rmc_msg, uint16_t *hour);

/* === Internal variable definitions ======================================================= */

/* === External variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */
gsp_error_t _getLatitude(const char *rmc_msg, double *lat) {
    char deg[LATITUDE_DEGREES_LENGTH + STRING_TERMINATOR_LENGTH] = {0};
    char min[COORDINATE_MINUTES_LENGTH + STRING_TERMINATOR_LENGTH] = {0};
    char *end;

    if (!rmc_msg || !lat) {
        return ERROR_POSITION_INVALID;
    }

    if (strlen(rmc_msg) <= LATITUDE_HEMISPHERE_POS) {
        return ERROR_VALIDATION_FAIL;
    }

    strncpy(deg, &rmc_msg[LATITUDE_DEGREES_POS], LATITUDE_DEGREES_LENGTH);
    double degD = strtod(deg, &end);

    strncpy(min, &rmc_msg[LATITUDE_MINUTES_POS], COORDINATE_MINUTES_LENGTH);
    double minD = strtod(min, &end);

    double sign;
    if (rmc_msg[LATITUDE_HEMISPHERE_POS] == 'N') {
        sign = POSITIVE_COORDINATE_SIGN;
    } else if (rmc_msg[LATITUDE_HEMISPHERE_POS] == 'S') {
        sign = NEGATIVE_COORDINATE_SIGN;
    } else {
        return ERROR_POSITION_INVALID;
    }

    *lat = sign * (degD + minD / MINUTES_PER_DEGREE);
    if (*lat > MAX_LATITUDE || *lat < MIN_LATITUDE) {
        return ERROR_LAT_OUT_OF_RANGE;
    }
    return ERROR_OK;
}

gsp_error_t _getLongitude(const char *rmc_msg, double *lon) {
    char deg[LONGITUDE_DEGREES_LENGTH + STRING_TERMINATOR_LENGTH] = {0};
    char min[COORDINATE_MINUTES_LENGTH + STRING_TERMINATOR_LENGTH] = {0};
    char *end;

    if (!rmc_msg || !lon) {
        return ERROR_POSITION_INVALID;
    }

    if (strlen(rmc_msg) <= LONGITUDE_HEMISPHERE_POS) {
        return ERROR_VALIDATION_FAIL;
    }

    strncpy(deg, &rmc_msg[LONGITUDE_DEGREES_POS], LONGITUDE_DEGREES_LENGTH);
    double degD = strtod(deg, &end);

    strncpy(min, &rmc_msg[LONGITUDE_MINUTES_POS], COORDINATE_MINUTES_LENGTH);
    double minD = strtod(min, &end);

    double sign;
    if (rmc_msg[LONGITUDE_HEMISPHERE_POS] == 'E') {
        sign = POSITIVE_COORDINATE_SIGN;
    } else if (rmc_msg[LONGITUDE_HEMISPHERE_POS] == 'W') {
        sign = NEGATIVE_COORDINATE_SIGN;
    } else {
        return ERROR_POSITION_INVALID;
    }

    *lon = sign * (degD + minD / MINUTES_PER_DEGREE);
    if (*lon > MAX_LONGITUDE || *lon < MIN_LONGITUDE) {
        return ERROR_LONG_OUT_OF_RANGE;
    }
    return ERROR_OK;
}

gsp_error_t _getHours(const char *rmc_msg, uint8_t *hour) {
    char hh[HOUR_LENGTH + STRING_TERMINATOR_LENGTH] = {0};
    char *end;

    if (!rmc_msg || !hour) {
        return ERROR_INPUT_DATA;
    }

    if (strlen(rmc_msg) < HOUR_POS + HOUR_LENGTH) {
        return ERROR_VALIDATION_FAIL;
    }

    strncpy(hh, &rmc_msg[HOUR_POS], HOUR_LENGTH);
    unsigned long hhI = strtoul(hh, &end, 10);

    if (hh[0] < '0' || hh[0] > '9' || hh[1] < '0' || hh[1] > '9' || end != hh + HOUR_LENGTH) {
        return ERROR_INPUT_DATA;
    }

    *hour = (uint8_t)hhI;
    if (*hour > MAX_HOUR) {
        return ERROR_HH_OUT_OF_RANGE;
    }
    return ERROR_OK;
}

gsp_error_t _getMinutes(const char *rmc_msg, uint8_t *hour) {
}

gsp_error_t _getSeconds(const char *rmc_msg, uint8_t *hour) {
}

gsp_error_t _getMiliseconds(const char *rmc_msg, uint16_t *hour) {
}

/* === External function definitions ======================================================= */

gsp_error_t GPS_PARSER_RMC_isValidMsg(const char *rmc_msg) {
    if (!rmc_msg || strlen(rmc_msg) <= VALIDATION_POS) {
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

void GPS_PARSER_RMC_processMsg(const char *rmc_msg) {
}

gsp_error_t GPS_PARSER_RMC_getPosition(const char *rmc_msg, gps_rmc_position_t *position) {
    gps_rmc_position_t pos = {0};
    gsp_error_t err = GPS_PARSER_RMC_isValidMsg(rmc_msg);

    err |= _getLatitude(rmc_msg, &pos.latitude);

    err |= _getLongitude(rmc_msg, &pos.longitude);

    position->latitude = pos.latitude;
    position->longitude = pos.longitude;

    return err;
}

gsp_error_t GPS_PARSER_RMC_getTime(const char *rmc_msg, gps_rmc_time_t *time) {
    if (!time) {
        return ERROR_INPUT_DATA;
    }

    gps_rmc_time_t _time = {0};
    gsp_error_t err = GPS_PARSER_RMC_isValidMsg(rmc_msg);

    err |= _getHours(rmc_msg, &_time.hours);

    time->hours = _time.hours;
    return err;
}

/* === End of documentation ================================================================ */

/** @} End of the module definition for Doxygen */
