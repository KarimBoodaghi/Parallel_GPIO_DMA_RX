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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "RingBuffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define LEN 10
#define TXLEN (LEN * 9) + 2
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t buff[LEN]; /*** GPIO DMA (Parallel Input) Buffer ***/
uint8_t txBuff[TXLEN]; /*** UART Buffer ***/
uint8_t transferComplete = 0;

char Serial_TX_Data[16];

RingBuffer Ring_Buffer_1;
uint16_t Ring_Buffer_1_Length = 0, Ring_Buffer_1_FreeSpace = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);
static void HalfTransferComplete(DMA_HandleTypeDef *DmaHandle);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	RingBuffer_Init(&Ring_Buffer_1);
	
	
	/*** Attach DMA callback functions ***/
	htim1.hdma[TIM_DMA_ID_CC2]->XferHalfCpltCallback = HalfTransferComplete;
	htim1.hdma[TIM_DMA_ID_CC2]->XferCpltCallback = TransferComplete;
	htim1.hdma[TIM_DMA_ID_CC2]->XferErrorCallback = TransferError;
 
	/*** Start DMA in interrupt mode, specify source and destination ***/
	HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_CC2], (uint32_t) &GPIOF->IDR, (uint32_t) buff, LEN);
 
	/*** Enable timer to trigger DMA transfer - CC2DE bit ***/
	__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC2);
 
	/*** Enable timer input capture ***/
	HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_2);
	
	sprintf(Serial_TX_Data, "%s", "Start\r\n");
	HAL_UART_Transmit(&huart6, (uint8_t*)Serial_TX_Data, strlen(Serial_TX_Data), 0xFFFF);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		Ring_Buffer_1_Length = RingBuffer_GetDataLength(&Ring_Buffer_1);
		Ring_Buffer_1_FreeSpace = RingBuffer_GetFreeSpace(&Ring_Buffer_1);
		if (transferComplete)
		{
			transferComplete = 0;
 
//			txBuff[0] = '#';
//			txBuff[1] = '\n';
//			for (int i = 0; i < LEN; i++)
//			{
//				for (int k = 0; k < 8; k++)
//				{
//					uint32_t val = buff[i] & (1u << k);
//					txBuff[(i * 9) + k + 2] = val == 0 ? '0' : '1';
//				}
//				txBuff[(i * 9) + 10] = '\n';
//			}
 

			RingBuffer_Read(&Ring_Buffer_1, txBuff, 10);
			HAL_UART_Transmit_IT(&huart6, txBuff, 10);
		}
		//HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
		HAL_Delay(200);
    /* USER CODE END WHILE */

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
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void HalfTransferComplete(DMA_HandleTypeDef *DmaHandle)
{
	//HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
//	sprintf(Serial_TX_Data, "%s", "DMA HLF\r\n");
//	HAL_UART_Transmit(&huart6, (uint8_t*)Serial_TX_Data, strlen(Serial_TX_Data), 0xFFFF);
}
 
static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
	//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
//	sprintf(Serial_TX_Data, "%s", "DMA CMPLT\r\n");
//	HAL_UART_Transmit(&huart6, (uint8_t*)Serial_TX_Data, strlen(Serial_TX_Data), 0xFFFF);
 
	transferComplete = 1;
}
 
static void TransferError(DMA_HandleTypeDef *DmaHandle)
{
	//HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	sprintf(Serial_TX_Data, "%s", "DMA Error\r\n");
	HAL_UART_Transmit(&huart6, (uint8_t*)Serial_TX_Data, strlen(Serial_TX_Data), 0xFFFF);
}
 
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	/*** Enable again DMA for a new transfer ***/
	//HAL_DMA_Start_IT(htim1.hdma[TIM_DMA_ID_CC2], (uint32_t) &GPIOF->IDR, (uint32_t) buff, LEN);
}
/* USER CODE END 4 */

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
