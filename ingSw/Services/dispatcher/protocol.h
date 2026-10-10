#ifndef __PROTOCOL_H
#define __PROTOCOL_H

/** \brief UART protocol message format and subscribers.
 **
 ** \addtogroup Protocol UART Protocol
 ** @{ */

/* === External file includes ============================================================== */

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

#define PROTOCOL_LENGTH_FIELD_LEN 2U
#define PROTOCOL_HEADER_LEN       5U // Includes PROTOCOL_LENGTH_FIELD_LEN
#define MSG_SIZE                  64U
#define PAYLOAD_MAX_SIZE          (MSG_SIZE - PROTOCOL_HEADER_LEN)
#define PROTOCOL_START_BYTE       '!'

/* === Data type declarations ============================================================== */

typedef enum {
    SUBSYSTEM_NONE = '0' - 1,
    SUBSYSTEM_COMM_TEST = '0',
    SUBSYSTEM_LED_RGB,
    SUBSYSTEM_LOGIC,
    SUBSYSTEM_PARSER,
    SUBSYSTEM_END
} subsystem_e;

typedef struct {
    uint8_t startByte;
    uint8_t subsystem;
    uint8_t cmd;
    uint8_t payload_size[PROTOCOL_LENGTH_FIELD_LEN];
    uint8_t payload[PAYLOAD_MAX_SIZE];
} protocol_msg_t;

typedef union {
    protocol_msg_t frame;
    uint8_t rawData[MSG_SIZE];
} protocolData_u;

typedef struct {
    subsystem_e subsystem;
    QueueHandle_t queue_sub;
} subscriber_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

/* === Internal function definitions ======================================================= */

static inline bool
PROTOCOL_payloadSizeToUint8(const uint8_t payload_size[PROTOCOL_LENGTH_FIELD_LEN], uint8_t *size) {
    if (payload_size == NULL || size == NULL || payload_size[0] < (uint8_t)'0' ||
        payload_size[0] > (uint8_t)'9' || payload_size[1] < (uint8_t)'0' ||
        payload_size[1] > (uint8_t)'9') {
        return false;
    }

    *size = (uint8_t)(((payload_size[0] - (uint8_t)'0') * 10U) + (payload_size[1] - (uint8_t)'0'));
    return true;
}

static inline bool PROTOCOL_isValidSubsystem(uint8_t subsystem) {
    return subsystem > (uint8_t)SUBSYSTEM_NONE && subsystem < (uint8_t)SUBSYSTEM_END;
}

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* __PROTOCOL_H */
