#include "unity.h"
#include "gps_parser_rmc.h"

// Valid Frame
const char gpsValidMsg[] =
    "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

// Invalid frame
const char gpsInvalidMsg[] =
    "GPRMC,192448.900,V,3256.499,S,06039.935,W,755.8,100.4,101026,000.0,W*75";

const char gpsShortMsg[] = "GPRMC,192448.900";

// Resultados esperados
double expected_latitude = -32.94165000;
double expected_longitude = -60.66558333;

uint8_t expected_hours = 19;
uint8_t expected_minutes = 24;
uint8_t expected_seconds = 48;
uint16_t expected_milliseconds = 900;

uint8_t expected_day = 10;
uint8_t expected_month = 10;
uint16_t expected_year = 2026;

void setUp(void) {
}

void tearDown(void) {
}

void test_gps_dummy(void) {
    TEST_ASSERT_TRUE(1);
}

void test_gps_validatedData(void) {
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_isValidMsg(&gpsValidMsg));
}

void test_gps_validateNullData(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(NULL));
}

void test_gps_validateShortMsg(void) {
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(&gpsShortMsg));
}
