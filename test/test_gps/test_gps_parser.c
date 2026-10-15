/* Pruebas mínimas de State Transition Testing (STT) para GPSParser.
 * FSM actual: FIND_START / FIND_END. Transiciones T1 a T6.
 * Este archivo reemplaza test/test_gps/test_gps_parser.c.
 */
#include "unity.h"
#include "gps_parser.h"
#include "gps_parser_rmc.h"
#include "gps_data/gps_test_data.h"
#include <stdio.h>
#include <string.h>

static gps_parser_t parser;
static unsigned decoder_calls;
static unsigned callback_calls;

/* Se conserva el wrapper de enlace que utiliza el proyecto para observar el despacho. */
gsp_error_t __wrap_GPS_PARSER_RMC_processMsg(const char *msg, gps_rmc_data_t *data) {
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_NOT_NULL(data);
    decoder_calls++;
    *data = expectedFrame1;
    return ERROR_OK;
}

static void on_rmc(const gps_rmc_data_t *data) {
    TEST_ASSERT_NOT_NULL(data);
    callback_calls++;
}

static void feed(const char *text) {
    GPS_PARSER_consume(&parser, text, strlen(text));
}

/* Construye una sentencia válida para ejercitar T5 sin probar el checksum en sí. */
static void feed_valid_rmc(void) {
    char frame[GPS_PARSER_FRAME_MAX + 2];
    const char *star = strchr(gpsValidMsg1, '*');
    TEST_ASSERT_NOT_NULL(star);
    size_t body_len = (size_t)(star - gpsValidMsg1);
    unsigned char checksum = 0;
    for (size_t i = 0; i < body_len; ++i) {
        checksum ^= (unsigned char)gpsValidMsg1[i];
    }
    int len =
        snprintf(frame, sizeof(frame), "$%.*s*%02X\r\n", (int)body_len, gpsValidMsg1, checksum);
    TEST_ASSERT_GREATER_THAN_INT(0, len);
    TEST_ASSERT_LESS_OR_EQUAL_INT((int)sizeof(frame) - 1, len);
    GPS_PARSER_consume(&parser, frame, (size_t)len);
}

void setUp(void) {
    decoder_calls = 0;
    callback_calls = 0;
    GPS_PARSER_init(&parser, on_rmc);
}

void tearDown(void) {
}

/* T1: FIND_START -- dato distinto de '$' --> FIND_START */
void test_STT_T1_noise_keeps_find_start(void) {
    feed("abc\r\n");
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_START, parser.state);
    TEST_ASSERT_EQUAL_UINT(0, parser.length);
    TEST_ASSERT_EQUAL_UINT(0, decoder_calls);
    TEST_ASSERT_EQUAL_UINT(0, callback_calls);
}

/* T2 + T3: FIND_START -- '$' --> FIND_END -- dato --> FIND_END */
void test_STT_T2_T3_start_then_data_stays_in_find_end(void) {
    feed("$A");
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_END, parser.state);
    TEST_ASSERT_EQUAL_UINT(2, parser.length);
    TEST_ASSERT_EQUAL_MEMORY("$A", parser.frame, 2);
    TEST_ASSERT_EQUAL_UINT(0, decoder_calls);
}

/* T2 + T3 + T4: un segundo '$' reemplaza la trama incompleta. */
void test_STT_T2_T4_new_start_resynchronizes(void) {
    feed("$ABC$");
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_END, parser.state);
    TEST_ASSERT_EQUAL_UINT(1, parser.length);
    TEST_ASSERT_EQUAL_CHAR('$', parser.frame[0]);
    TEST_ASSERT_EQUAL_UINT(0, decoder_calls);
}

/* T2 + T3 + T5: una trama completa vuelve a FIND_START. */
void test_STT_T2_T3_T5_complete_frame_returns_to_find_start(void) {
    feed_valid_rmc();
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_START, parser.state);
    TEST_ASSERT_EQUAL_UINT(0, parser.length);
    TEST_ASSERT_EQUAL_UINT(1, decoder_calls);
    TEST_ASSERT_EQUAL_UINT(1, callback_calls);
}

/* T2 + T3 + T6: byte 83 sin terminador provoca overflow y recuperación. */
void test_STT_T2_T3_T6_overflow_returns_to_find_start(void) {
    char bytes[GPS_PARSER_FRAME_MAX + 1];
    memset(bytes, 'x', sizeof(bytes));
    bytes[0] = '$';
    GPS_PARSER_consume(&parser, bytes, GPS_PARSER_FRAME_MAX);
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_END, parser.state);
    TEST_ASSERT_EQUAL_UINT(GPS_PARSER_FRAME_MAX, parser.length);
    GPS_PARSER_consume(&parser, bytes + GPS_PARSER_FRAME_MAX, 1);
    TEST_ASSERT_EQUAL(GPS_PARSER_FIND_START, parser.state);
    TEST_ASSERT_EQUAL_UINT(0, parser.length);
    TEST_ASSERT_EQUAL_UINT(0, decoder_calls);
}
