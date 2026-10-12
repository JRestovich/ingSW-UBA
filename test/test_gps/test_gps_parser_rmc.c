#include "unity.h"
#include "gps_parser_rmc.h"
#include "gps_test_data.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_gps_validatedData(void) {
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_isValidMsg(gpsValidMsg1));
}

void test_gps_validateNullData(void) {
    TEST_ASSERT_BITS_HIGH(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(NULL));
}

void test_gps_validateShortMsg(void) {
    TEST_ASSERT_BITS_HIGH(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(gpsShortMsg));
}

void test_gps_invalidData(void) {
    TEST_ASSERT_BITS_HIGH(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_isValidMsg(gpsInvalidMsg));
}

void test_gps_validateUnknown(void) {
    TEST_ASSERT_BITS_HIGH(ERROR_VALIDATION_UNKNOW, GPS_PARSER_RMC_isValidMsg(gpsUnknownMsg));
}

void test_gps_getLatS(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(gpsValidMsg1, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame1.position.latitude, pos.latitude);
}

void test_gps_getLatN(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getPosition(gpsValidMsg2, &pos));
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, expectedFrame2.position.latitude, pos.latitude);
}

void test_gps_getLatOutOfRangeS(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_BITS_HIGH(ERROR_LAT_OUT_OF_RANGE,
                          GPS_PARSER_RMC_getPosition(gpsLatLongOutOfRange1, &pos));
}

void test_gps_getLatOutOfRangeN(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_BITS_HIGH(ERROR_LAT_OUT_OF_RANGE,
                          GPS_PARSER_RMC_getPosition(gpsLatLongOutOfRange2, &pos));
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

void test_gps_getLongOutOfRangeE(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_BITS_HIGH(ERROR_LONG_OUT_OF_RANGE,
                          GPS_PARSER_RMC_getPosition(gpsLatLongOutOfRange1, &pos));
}

void test_gps_getLongOutOfRangeW(void) {
    gps_rmc_position_t pos = {0};
    TEST_ASSERT_BITS_HIGH(ERROR_LONG_OUT_OF_RANGE,
                          GPS_PARSER_RMC_getPosition(gpsLatLongOutOfRange2, &pos));
}

void test_gps_getHour(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(gpsValidMsg1, &time));
    TEST_ASSERT_EQUAL_UINT8(expectedFrame1.time.hours, time.hours);
}

void test_gps_getHourOutOfRange(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,242448.900,A,";
    TEST_ASSERT_EQUAL(ERROR_HH_OUT_OF_RANGE, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getHourInvalidDigits(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,1X2448.900,A,";
    TEST_ASSERT_EQUAL(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getTimeNullOutput(void) {
    TEST_ASSERT_EQUAL(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(gpsValidMsg1, NULL));
}
