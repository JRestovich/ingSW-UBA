#include "unity.h"
#include "gps_parser_rmc.h"

// Valid Frame
const char gpsValidMsg1[] =
    "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

const char gpsValidMsg2[] =
    "GPRMC,192451.900,A,3256.711,N,06039.972,E,682.6,282.8,101026,000.0,W*72";

// Invalid frame
const char gpsInvalidMsg[] =
    "GPRMC,192448.900,V,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

// Short message
const char gpsShortMsg[] = "GPRMC,";

// Validation unknown frame
const char gpsUnknownMsg[] = "GPRMC,192448.900,B,";

const char gpsLatLongOutOfRange1[] =
    "GPRMC,192451.900,A,9056.711,S,18039.972,E,682.6,282.8,101026,000.0,W*72";

const char gpsLatLongOutOfRange2[] =
    "GPRMC,192451.900,A,9056.711,N,18039.972,W,682.6,282.8,101026,000.0,W*72";

// Resultados esperados
gps_rmc_data_t expectedFrame1 = {
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

gps_rmc_data_t expectedFrame2 = {
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

void setUp(void) {
}

void tearDown(void) {
}

void test_gps_dummy(void) {
    TEST_ASSERT_TRUE(1);
}

void test_gps_validatedData(void) {
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_isValidMsg(&gpsValidMsg1));
}

void test_gps_validateNullData(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(NULL));
}

void test_gps_validateShortMsg(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(&gpsShortMsg));
}

void test_gps_invalidData(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(&gpsInvalidMsg));
}

void test_gps_validateUnknown(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_UNKNOW, GPS_PARSER_RMC_isValidMsg(&gpsUnknownMsg));
}

void test_gps_getLatS(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(&gpsValidMsg1, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame1.position.latitude, pos.latitude);
}

void test_gps_getLatN(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(&gpsValidMsg2, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame2.position.latitude, pos.latitude);
}

void test_gps_getLatOutOfRangeS(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_LAT_OUT_OF_RANGE,
                      GPS_PARSER_RMC_getPosition(&gpsLatLongOutOfRange1, &pos));
}

void test_gps_getLatOutOfRangeN(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_LAT_OUT_OF_RANGE,
                      GPS_PARSER_RMC_getPosition(&gpsLatLongOutOfRange2, &pos));
}

void test_gps_getLongW(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(gpsValidMsg1, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame1.position.longitude, pos.longitude);
}

void test_gps_getLongE(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(gpsValidMsg2, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame2.position.longitude, pos.longitude);
}
