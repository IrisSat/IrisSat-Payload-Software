/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
#define ADC_CHANNEL_BOARD	1
#define ADC_CHANNEL_PLATE	0
#define ADC_CHANNEL_POWER	2
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */

//Reads all three channels and returns the results. Results are 12 bit numbers, 0->0V, 4096->3V.
//results[0] = sample plate temp, results[1] = board temp, results[2] = power good
void adcReadAllChannels_int(uint16_t* results);

//Reads one channel of the ADC.Results are 12 bit numbers, 0->0V, 4096->3V.
uint16_t adcReadChannel_int(uint8_t channel);

//Same as adcReadAllChannels_int, except results are as float and scaled for reference voltage.
void adcReadAllChannels_volts(float* results);

//Same as adcReadChannel_int, except result is as float and scaled for reference voltage.
float adcReadChannel_volts(uint8_t channel);

//Converts a voltage to temperature in Celcius.Takes the parameters A,B,C.
float getThermistorTemp(float voltage, float A,float B, float C);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
