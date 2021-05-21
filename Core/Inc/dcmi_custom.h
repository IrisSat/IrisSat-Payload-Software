/*
 * dcmi_custom.h
 *
 *  Created on: May 21, 2021
 *      Author: howarth3-INS
 */

#ifndef INC_DCMI_CUSTOM_H_
#define INC_DCMI_CUSTOM_H_

#include "dcmi.h"
HAL_StatusTypeDef HAL_DCMI_Start_DMA_multi(DCMI_HandleTypeDef *hdcmi, uint32_t DCMI_Mode, uint32_t buffer0, uint32_t buffer1, uint32_t bufferSize, uint32_t imageSize);
uint8_t bufferSwitched();
uint32_t* getFreeBuff();
void doneBufferHandling();
uint8_t getDcmiCustomError();

#endif /* INC_DCMI_CUSTOM_H_ */
