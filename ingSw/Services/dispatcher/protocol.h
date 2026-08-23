/* ******************************************************************************
  * @file           : uartcomm.h
  * @brief          : Header for pwm.c file.
  *                   This file contains the common defines of the application.
  *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "queue.h"

/* Private includes ----------------------------------------------------------*/
#define MSG_SIZE 64
#define PAYLOAD_MAX_SIZE 60

typedef enum {
    SUBSYSTEM_LED_RGB,
    SUBSYSTEM_LOGIC,
    SUBSYSTEM_PARSER
} subsystem_e;

typedef struct
{
    char startByte;
    uint8_t subsystem;
    uint8_t cmd;
    uint8_t msg_size;
	uint8_t	payload[PAYLOAD_MAX_SIZE];
} protocol_msg_t;

typedef union
{
    protocol_msg_t msg;
    uint8_t rawData[MSG_SIZE];    
} protocolData_u;

typedef struct
{
    subsystem_e     subsystem;
	QueueHandle_t	queue_sub;
} subscriber_t;


#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */
