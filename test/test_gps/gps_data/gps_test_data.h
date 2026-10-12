#ifndef __GPS_TEST_DATA_H
#define __GPS_TEST_DATA_H

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

/* === External variable declarations ====================================================== */
// Valid Frame
static const char gpsValidMsg1[] =
    "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

static const char gpsValidMsg2[] =
    "GPRMC,192451.900,A,3256.711,N,06039.972,E,682.6,282.8,101026,000.0,W*72";

// Invalid frame
static const char gpsInvalidMsg[] =
    "GPRMC,192448.900,V,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

// Short message
static const char gpsShortMsg[] = "GPRMC,";

// Validation unknown frame
static const char gpsUnknownMsg[] = "GPRMC,192448.900,B,";

static const char gpsLatLongOutOfRange1[] =
    "GPRMC,192451.900,A,9056.711,S,18039.972,E,682.6,282.8,101026,000.0,W*72";

static const char gpsLatLongOutOfRange2[] =
    "GPRMC,192451.900,A,9056.711,N,18039.972,W,682.6,282.8,101026,000.0,W*72";

// Position cases: ranges, hemispheres, zero and boundaries.
static const char gpsPositionLatitudeOutOfRangeOnly[] =
    "GPRMC,192448.900,A,9100.000,N,06039.935,W,755.8,100.4,101026,000.0,W*66";

static const char gpsPositionLongitudeOutOfRangeOnly[] =
    "GPRMC,192448.900,A,3256.499,S,18100.000,E,755.8,100.4,101026,000.0,W*6C";

static const char gpsPositionNorthWest[] =
    "GPRMC,192448.900,A,3256.499,N,06039.935,W,755.8,100.4,101026,000.0,W*68";

static const char gpsPositionSouthEast[] =
    "GPRMC,192448.900,A,3256.499,S,06039.935,E,755.8,100.4,101026,000.0,W*67";

static const char gpsPositionZeroCoordinates[] =
    "GPRMC,192448.900,A,0000.000,N,00000.000,E,755.8,100.4,101026,000.0,W*7F";

static const char gpsPositionPositiveLimits[] =
    "GPRMC,192448.900,A,9000.000,N,18000.000,E,755.8,100.4,101026,000.0,W*7F";

static const char gpsPositionNegativeLimits[] =
    "GPRMC,192448.900,A,9000.000,S,18000.000,W,755.8,100.4,101026,000.0,W*70";

// Expected results
static const gps_rmc_data_t expectedFrame1 = {
    .position =
        {
            .latitude = -32.94165000,
            .longitude = -60.66558333,
        },
    .time =
        {
            .hours = 19,
            .minutes = 24,
            .seconds = 48,
            .miliseconds = 900,
        },
    .date =
        {
            .day = 10,
            .month = 10,
            .year = 2026,
        },
};

static const gps_rmc_data_t expectedFrame2 = {
    .position =
        {
            .latitude = 32.94518333333333,
            .longitude = 60.6662,
        },
    .time =
        {
            .hours = 19,
            .minutes = 24,
            .seconds = 51,
            .miliseconds = 900,
        },
    .date =
        {
            .day = 10,
            .month = 10,
            .year = 2026,
        },
};

/* === External function declarations ====================================================== */

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __GPS_TEST_DATA_H */
