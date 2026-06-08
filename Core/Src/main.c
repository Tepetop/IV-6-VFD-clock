/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "74hc595.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_DIGITS_COUNT   10u
#define TEST_LAMPS_COUNT    4u
#define TEST_SEGMENTS_OFF   0xFFu
#define TEST_GRIDS_OFF      0xFFu
#define TEST_STEP_DELAY_MS  200u
#define TEST_BLANK_DELAY_MS 50u

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static HC595_t ShiftReg595;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if 1
static const uint8_t kDigitSegments[TEST_DIGITS_COUNT] = {
  0x84u, /* 0 */
  0xBEu, /* 1 */
  0xC8u, /* 2 */
  0x98u, /* 3 */
  0xB2u, /* 4 */
  0x91u, /* 5 */
  0x81u, /* 6 */
  0xBCu, /* 7 */
  0x80u, /* 8 */
  0xB0u  /* 9 */
};
#else
static const uint8_t kDigitSegments[TEST_DIGITS_COUNT] = {
  0x7Bu, /* 0 */
  0x24u, /* 1 */
  0x5Du, /* 2 */
  0x6Du, /* 3 */
  0x2Eu, /* 4 */
  0x6Bu, /* 5 */
  0x7Bu, /* 6 */
  0x25u, /* 7 */
  0x7Fu, /* 8 */
  0x6Fu  /* 9 */
};
#endif

static const uint8_t kGridMasks[TEST_LAMPS_COUNT] = {
  0xFEu, /* Lamp 0 active-low */
  0xFDu, /* Lamp 1 active-low */
  0xFBu, /* Lamp 2 active-low */
  0xF7u  /* Lamp 3 active-low */
};

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
  /* USER CODE BEGIN 2 */
  HC595_Init(&ShiftReg595, SER_GPIO_Port, SER_Pin, SRCLK_Pin, SRCLEAR_Pin, RCLK_Pin, OE_Pin);
  HC595_SetShiftClear(&ShiftReg595, false);
  //HC595_WriteDisplayFrame(&ShiftReg595, TEST_SEGMENTS_OFF, TEST_GRIDS_OFF);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    for (uint8_t lamp = 0u; lamp < TEST_LAMPS_COUNT; lamp++) {
      for (uint8_t digit = 0u; digit < TEST_DIGITS_COUNT; digit++) {
        HC595_WriteDisplayFrame(&ShiftReg595, kDigitSegments[digit], kGridMasks[lamp]);
        HAL_Delay(TEST_STEP_DELAY_MS);
      }

      HC595_WriteDisplayFrame(&ShiftReg595, TEST_SEGMENTS_OFF, TEST_GRIDS_OFF);
      HAL_Delay(TEST_BLANK_DELAY_MS);
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
