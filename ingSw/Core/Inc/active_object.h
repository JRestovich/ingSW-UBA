#ifndef ACTIVE_OBJECT_H_
#define ACTIVE_OBJECT_H_

/** \file active_object.h
 ** \brief Task and queue handles for active objects.
 **
 ** \addtogroup ActiveObject
 ** @{ */

/* === External file includes ============================================================== */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* === C++ linkage ========================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* === Definitions and macros ============================================================== */

/* === Data type declarations ============================================================== */

typedef struct
{
    TaskHandle_t h_task;
    QueueHandle_t h_queue;
    char queue_txt[15];
    char task_txt[15];
} active_object_t;

/* === External variable declarations ====================================================== */

/* === External function declarations ====================================================== */

/* === End of documentation ================================================================ */

#ifdef __cplusplus
}
#endif

/** @} End of the module definition for Doxygen */

#endif /* ACTIVE_OBJECT_H_ */
