/* === Header includes ===================================================================== */
#include "gps_parser.h"
#include "gps_parser_rmc.h"
#include <string.h>

/* === Internal data type declarations ===================================================== */

/* === Internal function declarations ====================================================== */

static void _processByte(gps_parser_t *parser, char byte);

/* === Internal variable definitions ======================================================= */

/* === Internal function definitions ======================================================= */
static int _hex(char byte) {
    if (byte >= '0' && byte <= '9') {
        return byte - '0';
    }
    if (byte >= 'A' && byte <= 'F') {
        return byte - 'A' + 10;
    }
    if (byte >= 'a' && byte <= 'f') {
        return byte - 'a' + 10;
    }
    return -1;
}

static bool _hasValidChecksum(const char *frame, size_t length) {
    /* Locate *HH safely before computing the XOR between $ and *. */
    if (length < 13 || frame[0] != '$' || frame[length - 2] != '\r' || frame[length - 1] != '\n' ||
        frame[length - 5] != '*') {
        return false;
    }
    int high = _hex(frame[length - 4]);
    int low = _hex(frame[length - 3]);
    if (high < 0 || low < 0) {
        return false;
    }
    unsigned char checksum = 0;
    for (size_t i = 1; i < length - 5; ++i) {
        unsigned char byte = (unsigned char)frame[i];
        if (byte < 0x20 || byte > 0x7e || byte == '*') {
            return false;
        }
        checksum ^= byte;
    }
    return checksum == (unsigned char)((high << 4) | low);
}

static void _dispatch(gps_parser_t *parser) {
    char *frame = parser->frame;
    size_t length = parser->length;
    if (!_hasValidChecksum(frame, length)) {
        return;
    }

    /* NMEA identifier: two-character talker followed by three-character type. */
    if (frame[6] != ',') {
        return;
    }
    char sentence[6];
    memcpy(sentence, frame + 1, sizeof(sentence) - 1);
    sentence[sizeof(sentence) - 1] = '\0';
    for (size_t i = 0; i < sizeof(sentence) - 1; ++i) {
        if (sentence[i] < 'A' || sentence[i] > 'Z') {
            return;
        }
    }

    if (strcmp(sentence + 2, "RMC") != 0) {
        return;
    }

    /* Decoder expects the body without $, CR or LF. */
    frame[length - 2] = '\0';
    gps_rmc_data_t data = {0};
    if (GPS_PARSER_RMC_processMsg(frame + 1, &data) == ERROR_OK && parser->callback) {
        parser->callback(&data, parser->user);
    }
}

static void _processByte(gps_parser_t *parser, char byte) {
    switch (parser->state) {
    case GPS_PARSER_IDLE:
        break;

    case GPS_PARSER_FI:
        if (byte == '$') {
            parser->state = GPS_PARSER_FE;
            parser->length = 1;
            parser->frame[0] = byte;
        }
        break;

    case GPS_PARSER_FE:
        /* START takes priority even when 82 bytes are already captured. */
        if (byte == '$') {
            parser->length = 1;
            parser->frame[0] = byte;
        } else if (parser->length == GPS_PARSER_FRAME_MAX) {
            parser->length = 0;
            parser->state = GPS_PARSER_FI;
            break;
        } else {
            parser->frame[parser->length] = byte;
            if (byte == '\n' && parser->frame[parser->length - 1] == '\r') {
                parser->length++;
                parser->frame[parser->length] = '\0';
                _dispatch(parser);
                parser->length = 0;
                parser->state = GPS_PARSER_FI;
            } else {
                parser->length++;
            }
        }
        break;
    }
}

/* === External function definitions ======================================================= */
void GPS_PARSER_init(gps_parser_t *parser, gps_parser_callback_t callback, void *user) {
    if (!parser) {
        return;
    }
    ring_buffer_init(&parser->ring, parser->storage, sizeof(parser->storage));
    parser->length = 0;
    parser->state = GPS_PARSER_IDLE;
    parser->callback = callback;
    parser->user = user;
}

void GPS_PARSER_rxReady(gps_parser_t *parser) {
    if (!parser) {
        return;
    }
    switch (parser->state) {
    case GPS_PARSER_IDLE:
        parser->state = GPS_PARSER_FI;
        break;

    case GPS_PARSER_FI:
    case GPS_PARSER_FE:
        break;
    }
}

void GPS_PARSER_consume(gps_parser_t *parser, const char *bytes, size_t length) {
    if (!parser || !bytes || parser->state == GPS_PARSER_IDLE) {
        return;
    }
    while (length) {
        size_t count = length < GPS_PARSER_RING_SIZE - 1 ? length : GPS_PARSER_RING_SIZE - 1;
        ring_buffer_queue_arr(&parser->ring, bytes, count);
        char byte;
        while (ring_buffer_dequeue(&parser->ring, &byte)) {
            _processByte(parser, byte);
        }
        bytes += count;
        length -= count;
    }
}

/* === End of documentation ================================================================ */
