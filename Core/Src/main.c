/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

#include <csp/csp.h>
#include "csp/interfaces/csp_if_can.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
int test(int a, int b);
static void vTestCspServer(void * pvParameters);
static void vTestCspClient(void * pvParameters);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Un-comment one, to run the CSP client or server task.
//#define CLIENT
#define SERVER
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* USER CODE BEGIN 2 */
  HAL_CAN_MspInit(&hcan2);
  MX_CAN2_Init();
//  startCAN();

  BaseType_t status;
#ifdef SERVER
    status = xTaskCreate(vTestCspServer,
                         "Test CSP Server",
                         160,
                         NULL,
                         1,
                         NULL);

#endif

#ifdef CLIENT
    status = xTaskCreate(vTestCspClient,
                         "Test CSP Client",
                         160,
                         NULL,
                         1,
                         NULL);


#endif

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */


	  if (HAL_ADC_Start(&hadc1) != HAL_OK)
	  {
	      /* Start Conversation Error */
	      // Error_Handler();
	  }
	  if (HAL_ADC_PollForConversion(&hadc1, 500) != HAL_OK)
	  {
	      /* End Of Conversion flag not set on time */
	      // Error_Handler();
	      ADCValue=-1;
	  }
	  else
	  {
	      /* ADC conversion completed */
	      /*##-5- Get the converted value of regular channel ########################*/
	      ADCValue = HAL_ADC_GetValue(&hadc1);
	      double voltage = ((double)ADCValue/4096*3 -1.5)/1.09;
	      double resistance = ((30000-20000*voltage)/(30000+20000*voltage) )* 10000; //(10000*(0.5-voltage/3))/(0.5+voltage/3);
	      temperatures[count] =  1/(9.02e-4 + 2.49e-4*log(resistance)+2.01e-7*pow(log(resistance),3)) - 273.15;
	      count = (count+1)%60;
	  }
	  HAL_ADC_Stop(&hadc1);
	 HAL_Delay(600);
    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int test(int a, int b){

	return a+b;
}

static void vTestCspServer(void * pvParameters){

	struct csp_can_config can_conf;
	can_conf.bitrate=250000;
	can_conf.clock_speed=250000;
	can_conf.ifc = "CAN";

	/* Init buffer system with 5 packets of maximum 256 bytes each */
	int resp = csp_buffer_init(5, 256);//The 256 number is from the MTU of the CAN interface.

	/* Init CSP with address 0 */
	resp = csp_init(0);

	/* Init the CAN interface with hardware filtering */
	resp = csp_can_init(CSP_CAN_MASKED, &can_conf);

	/* Setup default route to CAN interface */
	resp = csp_rtable_set(CSP_DEFAULT_ROUTE,0, &csp_if_can,CSP_NODE_MAC);

	size_t freSpace = xPortGetFreeHeapSize();
	/* Start router task with 100 word stack, OS task priority 1 */
	resp = csp_route_start_task(100, 1);


	csp_conn_t * conn = NULL;
	csp_packet_t * packet= NULL;
	csp_socket_t * socket = csp_socket(0);
	csp_bind(socket, CSP_ANY);
	csp_listen(socket,4);

	while(1) {

			conn = csp_accept(socket, 1000);
			if(conn){
				packet = csp_read(conn,0);
				//prvUARTSend(&g_mss_uart0, packet->data, packet->length);
				//printf(“%S\r\n”, packet->data);
				csp_buffer_free(packet);
				csp_close(conn);
			}
	}
}
/*-----------------------------------------------------------*/
static void vTestCspClient(void * pvParameters){

	struct csp_can_config can_conf;
	can_conf.bitrate=250000;
	can_conf.clock_speed=250000;
	can_conf.ifc = "CAN";

	/* Init buffer system with 5 packets of maximum 256 bytes each */
	int res = csp_buffer_init(5, 256);//The 256 number is from the MTU of the CAN interface.

	/* Init CSP with address 1 */
	res = csp_init(1);

	/* Init the CAN interface with hardware filtering */
	res = csp_can_init(CSP_CAN_MASKED, &can_conf);

	/* Setup address 0 to route to CAN interface */
	res = csp_rtable_set(0,0, &csp_if_can,0);

	size_t freSpace = xPortGetFreeHeapSize();
	/* Start router task with 100 word stack, OS task priority 1 */
	res = csp_route_start_task(100, 1);


	while(1){
		csp_conn_t * conn;
		csp_packet_t * packet;
		conn = csp_connect(2,0,4,1000,0);	//Create a connection. This tells CSP where to send the data (address and destination port).
		packet = csp_buffer_get(sizeof("TEST0 World")); // Get a buffer large enough to fit our data. Max size is 256.
		sprintf(packet->data,"TEST0 World");
		packet->length=strlen("TEST0 World");
		csp_send(conn,packet,0);
		csp_close(conn);
		vTaskDelay(10000);
	}
}


/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
