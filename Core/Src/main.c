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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "74hc595.h"
#include "iv6.h"
#include "ds3231.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISPLAY_REFRESH_PERIOD_MS 2u
#define RTC_ALARM2_PERIOD_MS      2000u

#define RTC_SIM_START_HOUR        12u
#define RTC_SIM_START_MINUTE      00u
#define RTC_SIM_START_SECOND      0u

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static HC595_t ShiftReg595;
static IV6_t VfdDisplay;

static DS3231_t Rtc;
static DS3231_DateTime RtcNow;
static DS3231_Alarm2 RtcAlarm2;

static uint32_t LastDisplayRefreshTick = 0u;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static bool App_IsLeapYear(uint16_t year_full);
static uint8_t App_DaysInMonth(uint8_t month, uint16_t year_full);
static void App_IncrementOneMinute(DS3231_DateTime *dt);
static void App_UpdateDisplayFromTime(IV6_t *display, const DS3231_DateTime *dt);
static void App_RtcHardwareInit(DS3231_t *hrtc);
static void App_RtcAlarm2Callback(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void App_UpdateDisplayFromTime(IV6_t *display, const DS3231_DateTime *dt)
{
  if (display == NULL || dt == NULL) {
    return;
  }

  uint8_t digits[IV6_DIGITS_COUNT] = {
    (uint8_t)(dt->minutes % 10u),
    (uint8_t)(dt->minutes / 10u),
    (uint8_t)(dt->hours % 10u),
    (uint8_t)(dt->hours / 10u)
  };

  IV6_SetDigits(display, digits);
  IV6_SetDot(display, 2, true);
}

static void App_RtcHardwareInit(DS3231_t *hrtc)
{
  if (hrtc == NULL) {
    return;
  }

  /* Inicjalizacja z użyciem realnego sprzętu I2C1 */
  if (DS3231_Init(hrtc, &hi2c1, DS3231_SQW_PORT, DS3231_SQW_PIN, DS3231_I2C_ADDR, DS3231_FORMAT_24H) != DS3231_OK) {
    return;
  }

  /* Odczytaj aktualny czas */
  DS3231_GetDateTime(hrtc, &RtcNow);

  /* Konfiguracja Alarmu 2: 1 minuta */
  RtcAlarm2.minutes = 0u;
  RtcAlarm2.hours = 0u;
  RtcAlarm2.ampm = DS3231_AM;
  RtcAlarm2.format = DS3231_FORMAT_24H;
  RtcAlarm2.day_date = 0u;
  RtcAlarm2.mode = DS3231_ALM2_EVERY_MINUTE;

  if (DS3231_SetAlarm2(hrtc, &RtcAlarm2) != DS3231_OK) {
    /* Błąd ustawienia alarmu */
  }

  /* Włącz przerwanie alarmu 2 */
  DS3231_EnableAlarm2Interrupt(hrtc);
}

static void App_RtcAlarm2Callback(void)
{
  /* Aktualizacja wyświetlacza po odebraniu przerwania alarmu 2 */
  App_UpdateDisplayFromTime(&VfdDisplay, &RtcNow);
}

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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HC595_Init(&ShiftReg595, SER_GPIO_Port, SER_Pin, SRCLK_Pin, SRCLEAR_Pin, RCLK_Pin, OE_Pin);
  HC595_SetOutputEnable(&ShiftReg595, true);
  HC595_SetShiftClear(&ShiftReg595, false);

  IV6_Init(&VfdDisplay, &ShiftReg595);

  App_RtcHardwareInit(&RtcSim);
  LastDisplayRefreshTick = HAL_GetTick();
  LastAlarm2Tick = HAL_GetTick();

  App_UpdateDisplayFromTime(&VfdDisplay, &RtcNow);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - LastDisplayRefreshTick) >= DISPLAY_REFRESH_PERIOD_MS) {
      LastDisplayRefreshTick = now;
      IV6_RefreshStep(&VfdDisplay);
    }

    if (RtcSim.DS3231_IRQ_Flag != 0u) {
      if ((RtcSim.DS3231_IRQ_Alarm & DS3231_IRQ_ALARM2) != 0u) {
        App_RtcAlarm2Callback();
      }

      RtcSim.DS3231_IRQ_Alarm = DS3231_IRQ_NONE;
      RtcSim.DS3231_IRQ_Flag = 0u;
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
