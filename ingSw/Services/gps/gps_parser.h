#ifndef GPS_PARSER_H
#define GPS_PARSER_H

/* === External file includes ============================================================== */
#include "gps_types.h"
#include <stddef.h>

/* === C++ linkage ========================================================================= */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */
#define GPS_PARSER_FRAME_MAX 82

/* === Data type declarations ============================================================== */
typedef enum {
    GPS_PARSER_FI,
    GPS_PARSER_FE
} gps_parser_state_t;

typedef void (*gps_parser_callback_t)(const gps_rmc_data_t *data);

/** Caller-owned context. Treat fields as private; do not copy after init.
 * Serialize operations; no ISR concurrency or callback reentry on this context.
 * Callback data is borrowed and valid only during the synchronous callback.
 */
typedef struct {
    char frame[GPS_PARSER_FRAME_MAX + 1];
    size_t length;
    gps_parser_state_t state;
    gps_parser_callback_t callback;
} gps_parser_t;

/* === External function declarations ====================================================== */
/** Initialize in FI, ready to find the next sentence start. */
void GPS_PARSER_init(gps_parser_t *parser, gps_parser_callback_t callback);
/** Process a borrowed UART chunk directly, synchronously and in order.
 * Partial frames persist across calls; the caller owns reception buffering.
 * NULL context/input or an empty batch is a no-op.
 * Input must not alias the parser context.
 */
void GPS_PARSER_consume(gps_parser_t *parser, const char *bytes, size_t length);

/* === End of documentation ================================================================ */
#ifdef __cplusplus
}
#endif
#endif
