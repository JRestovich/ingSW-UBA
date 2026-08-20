#ifndef ACTIVE_OBJECT_H_
#define ACTIVE_OBJECT_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/********************** typedef **********************************************/
typedef struct
{
    TaskHandle_t h_task;
    QueueHandle_t h_queue;
    char queue_txt[15];
    char task_txt[15];
} active_object_t;

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* ACTIVE_OBJECT_H_ */

/********************** end of file ******************************************/
