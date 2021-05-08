/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "queue.h"
#include <csp/arch/csp_thread.h>
#include <csp/interfaces/csp_if_can.h>
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
//typedef uint32_t can_id_t;
//typedef struct {
//	/** 32 bit CAN identifier */
//	can_id_t id;
//	/** Data Length Code */
//	uint8_t dlc;
//	/**< Frame Data - 0 to 8 bytes */
//	union __attribute__((aligned(8))) {
//		uint8_t data[8];
//		uint16_t data16[4];
//		uint32_t data32[2];
//	};
//} can_frame_t;
//QueueHandle_t rxQueue;
typedef uint32_t can_id_t;

/** CAN Frame */
typedef struct {
	/** 32 bit CAN identifier */
	can_id_t id;
	/** Data Length Code */
	uint8_t dlc;
	/**< Frame Data - 0 to 8 bytes */
	union __attribute__((aligned(8))) {
		uint8_t data[8];
		uint16_t data16[4];
		uint32_t data32[2];
	};
} can_frame_t;

typedef enum {
	CAN_ERROR = 0,
	CAN_NO_ERROR = 1,
} can_error_t;


void startCAN();
void sendMessageCAN(uint32_t id,uint8_t* data, uint8_t length);
void can_irq(CAN_HandleTypeDef *pcan);
void canTask(void * pvParams);

int can_send(can_id_t id, uint8_t * data, uint8_t dlc);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
