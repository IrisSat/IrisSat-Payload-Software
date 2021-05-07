/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dcmi.h"
#include "dma.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile int a = 0;
int count = 0;
static uint32_t img_size = 50000;
uint32_t jpeg_buffer[50000] = {0};
//static uint8_t *memory_location = jpeg_buffer;
HAL_DCMI_StateTypeDef retval;
uint32_t errval;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
    if (count == 0){
    	osDelay(50);
    	CameraSoftReset();
    	osDelay(50);
    	CameraSensorInit();
        count = 1;
        osDelay(50);
        StartSensorInJpegMode(600,480);
        osDelay(50);
        jpeg_buffer[1] = 1;
        jpeg_buffer[2] = 1;
        jpeg_buffer[3] = 1;
        jpeg_buffer[4] = 1;
        jpeg_buffer[5] = 1;
        jpeg_buffer[6] = 1;
        jpeg_buffer[7] = 1;
        jpeg_buffer[8] = 1;
        jpeg_buffer[9] = 1;
        jpeg_buffer[10] = 1;

    	__HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_FRAME);
    	__HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_VSYNC);
    	__HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_ERR);
    	retval = HAL_DCMI_GetState(&hdcmi);
    	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) jpeg_buffer, img_size);
    	DoCapture();
    	retval = HAL_DCMI_GetState(&hdcmi);

    	osDelay(50);
    	errval = HAL_DCMI_GetError(&hdcmi);
    	osDelay(50);
    	while(1){
    		if (a == 1){
    			break;
    		}
    		osDelay(50);
    	}
    	int ff_count =0;
    	int counter = 0;
    	retval = HAL_DCMI_GetState(&hdcmi);
    	//for(int i=0; i<img_size; i++){

    		//if( (((jpeg_buffer[i]>>24)&0x000000FF) == 0xFF) || (jpeg_buffer[i]>>16)&0x000000FF == 0xFF|| (jpeg_buffer[i]>>8)&0x000000FF == 0xFF||(jpeg_buffer[i]>>0)&0x000000FF == 0xFF ){
//
  //  			ff_count++;
    //		}
    	//	counter++;
    	//}

    }


  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	//HAL_DCMI_Stop(hdcmi);
	a = 1;

}
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	//HAL_DCMI_Stop(hdcmi);
	a = 1;

}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
