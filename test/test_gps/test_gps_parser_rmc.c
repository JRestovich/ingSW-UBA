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

void test_gps_getMinutes(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(gpsValidMsg1, &time));
    TEST_ASSERT_EQUAL_UINT8(expectedFrame1.time.minutes, time.minutes);
}

void test_gps_getMinutesZero(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,190048.900,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT8(0, time.minutes);
}

void test_gps_getMinutesMax(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,195948.900,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT8(59, time.minutes);
}

void test_gps_getMinutesOutOfRange(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,196048.900,A,";
    TEST_ASSERT_EQUAL(ERROR_MM_OUT_OF_RANGE, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getMinutesInvalidDigits(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192X48.900,A,";
    TEST_ASSERT_EQUAL(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getMinutesShortMsg(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_getTime("GPRMC,192", &time));
}

void test_gps_getSeconds(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(gpsValidMsg1, &time));
    TEST_ASSERT_EQUAL_UINT8(expectedFrame1.time.seconds, time.seconds);
}

void test_gps_getSecondsZero(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192400.900,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT8(0, time.seconds);
}

void test_gps_getSecondsMax(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192459.900,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT8(59, time.seconds);
}

void test_gps_getSecondsOutOfRange(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192460.900,A,";
    TEST_ASSERT_EQUAL(ERROR_SS_OUT_OF_RANGE, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getSecondsInvalidDigits(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,19244X.900,A,";
    TEST_ASSERT_EQUAL(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getSecondsShortMsg(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_getTime("GPRMC,19244", &time));
}

void test_gps_getMiliseconds(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(gpsValidMsg1, &time));
    TEST_ASSERT_EQUAL_UINT16(expectedFrame1.time.miliseconds, time.miliseconds);
}

void test_gps_getMilisecondsZero(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448.000,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT16(0, time.miliseconds);
}

void test_gps_getMilisecondsMax(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448.999,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT16(999, time.miliseconds);
}

void test_gps_getMilisecondsLeadingZeros(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448.009,A,";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getTime(msg, &time));
    TEST_ASSERT_EQUAL_UINT16(9, time.miliseconds);
}

void test_gps_getMilisecondsInvalidDigits(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448.9X0,A,";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getMilisecondsMissingDecimalPoint(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448X900,A,";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getMilisecondsTooManyDigits(void) {
    gps_rmc_time_t time = {0};
    const char msg[] = "GPRMC,192448.1000,A,";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getTime(msg, &time));
}

void test_gps_getMilisecondsShortMsg(void) {
    gps_rmc_time_t time = {0};
    TEST_ASSERT_EQUAL(ERROR_VALIDATION_FAIL, GPS_PARSER_RMC_getTime("GPRMC,192448.90", &time));
}

void test_gps_getYear(void) {
    gps_rmc_date_t date = {0};
    GPS_PARSER_RMC_getDate(gpsValidMsg1, &date);
    TEST_ASSERT_EQUAL_UINT16(expectedFrame1.date.year, date.year);
}

void test_gps_getYearInvalidDigits(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,10102X,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getYearShortMsg(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,10102,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getYearNullInput(void) {
    gps_rmc_date_t date = {0};
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getDate(NULL, &date));
}

void test_gps_getMonth(void) {
    gps_rmc_date_t date = {0};
    GPS_PARSER_RMC_getDate(gpsValidMsg1, &date);
    TEST_ASSERT_EQUAL_UINT8(expectedFrame1.date.month, date.month);
}

void test_gps_getMonthZero(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,100026,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_MONTH_OUT_OF_RANGE, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getMonthOutOfRange(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101326,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_MONTH_OUT_OF_RANGE, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getMonthInvalidDigits(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101X26,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getMonthShortMsg(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101,000.0,W*75";
    TEST_ASSERT_BITS_HIGH(ERROR_INPUT_DATA, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getDay(void) {
    gps_rmc_date_t date = {0};
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(gpsValidMsg1, &date));
    TEST_ASSERT_EQUAL_UINT8(expectedFrame1.date.day, date.day);
}

void test_gps_getDayZero(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,001026,000.0,W*75";
    TEST_ASSERT_NOT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getDayMonth31Days(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,311026,000.0,W*75";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
    TEST_ASSERT_EQUAL_UINT8(31, date.day);
}

void test_gps_getDayMonth30Days(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,300426,000.0,W*75";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
    TEST_ASSERT_EQUAL_UINT8(30, date.day);
}

void test_gps_getDayInvalidDayForMonth(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,310426,000.0,W*75";
    TEST_ASSERT_NOT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getDayLeapYearFebruary(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,290224,000.0,W*75";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
    TEST_ASSERT_EQUAL_UINT8(29, date.day);
}

void test_gps_getDayNonLeapYearFebruary(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,290226,000.0,W*75";
    TEST_ASSERT_NOT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getDayInvalidDigits(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,X11026,000.0,W*75";
    TEST_ASSERT_NOT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
}

void test_gps_getYearMin(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101000,000.0,W*75";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
    TEST_ASSERT_EQUAL_UINT16(2000, date.year);
}

void test_gps_getYearMax(void) {
    gps_rmc_date_t date = {0};
    const char msg[] = "GPRMC,192448.900,A,3256.499,S,06039.935,W,755.8,100.4,101099,000.0,W*75";
    TEST_ASSERT_EQUAL(ERROR_OK, GPS_PARSER_RMC_getDate(msg, &date));
    TEST_ASSERT_EQUAL_UINT16(2099, date.year);
}
