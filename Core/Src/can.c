/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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

/* Includes ------------------------------------------------------------------*/
#include "can.h"


/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

CAN_HandleTypeDef hcan2;

/* CAN2 init function */
void MX_CAN2_Init(void)
{

  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 108;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }



}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN3_CLK_ENABLE();
    __HAL_RCC_CAN2_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB13     ------> CAN2_TX
    PB5     ------> CAN2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN3_CLK_DISABLE();
    __HAL_RCC_CAN2_CLK_DISABLE();
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN2 GPIO Configuration
    PB13     ------> CAN2_TX
    PB5     ------> CAN2_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_5);

  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void startCAN(){

	  //For now, setup the filter to accept all messages.
	  CAN_FilterTypeDef filterConfig;
	  filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;//Choose mask mode.Accept when RxID & mask == ID.
	  filterConfig.FilterBank  = 0; //The filter number. We can have 14 filters, 0-13.
	  filterConfig.FilterScale = CAN_FILTERSCALE_32BIT; //Select one 32 bit filter, not 2 16 bit filters.
	  filterConfig.FilterFIFOAssignment = CAN_FilterFIFO0; // Assign messages from this filter to FIFO 0;
	  filterConfig.FilterMaskIdHigh = 0xFFFF; //MSB of filter mask. Choose 0xFFFF so that we accept all messages.
	  filterConfig.FilterMaskIdLow = 0xFFFF; //LSB of filter mask. Choose 0xFFFF so that we accept all messages.
	  filterConfig.FilterActivation = CAN_FILTER_ENABLE;

	  HAL_StatusTypeDef stat = HAL_CAN_ConfigFilter(&hcan2, &filterConfig);

	  //Set interrupt for rx.
	 stat =  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
	 HAL_NVIC_SetPriority(CAN2_RX0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY-1,0);

	  //Start the can peripheral
	  stat = HAL_CAN_Start(&hcan2);

	  rxQueue = xQueueCreate(10,sizeof(can_frame_t));//Create queue for 10 can messages;
}

void sendMessageCAN(uint32_t id,uint8_t* data, uint8_t length){

	CAN_TxHeaderTypeDef header;
	header.ExtId = id;
	header.IDE = CAN_ID_EXT;
	header.RTR = CAN_RTR_DATA;
	header.DLC = length;

	uint32_t mailbox;

	HAL_CAN_AddTxMessage(&hcan2, &header, data, &mailbox);

	//We might want to remove this later...
	//There could be a better way to check if the message is sent, like the callback etc.
	while(HAL_CAN_IsTxMessagePending(&hcan2, mailbox)){
		//Just wait for message to be sent.
	}

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){

	uint8_t rxData[8];
	CAN_RxHeaderTypeDef header;

	HAL_CAN_GetRxMessage(hcan, 0, &header, rxData);

	can_frame_t frame;
	frame.id=header.ExtId;
	frame.dlc = header.DLC;

	for(int i=0; i<frame.dlc;i++){
		frame.data[i] = rxData[i];
	}

	xQueueSendToBackFromISR(rxQueue,&frame,NULL);

}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/