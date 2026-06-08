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

static DS3231_t RtcSim;
static DS3231_DateTime RtcNow;
static DS3231_Alarm2 RtcAlarm2;

static uint32_t LastDisplayRefreshTick = 0u;
static uint32_t LastAlarm2Tick = 0u;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static bool App_IsLeapYear(uint16_t year_full);
static uint8_t App_DaysInMonth(uint8_t month, uint16_t year_full);
static void App_IncrementOneMinute(DS3231_DateTime *dt);
static void App_UpdateDisplayFromTime(IV6_t *display, const DS3231_DateTime *dt);
static void App_RtcSimulationInit(DS3231_t *hrtc, DS3231_DateTime *dt, DS3231_Alarm2 *alarm2);
static void App_RtcSimulationTask(DS3231_t *hrtc, DS3231_DateTime *dt, const DS3231_Alarm2 *alarm2);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static bool App_IsLeapYear(uint16_t year_full)
{
  if ((year_full % 4u) != 0u) {
    return false;
  }

  if ((year_full % 100u) != 0u) {
    return true;
  }

  return (year_full % 400u) == 0u;
}

static uint8_t App_DaysInMonth(uint8_t month, uint16_t year_full)
{
  static const uint8_t days_lut[12] = { 31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u };

  if (month < 1u || month > 12u) {
    return 31u;
  }

  if (month == 2u && App_IsLeapYear(year_full)) {
    return 29u;
  }

  return days_lut[month - 1u];
}

static void App_IncrementOneMinute(DS3231_DateTime *dt)
{
  if (dt == NULL) {
    return;
  }

  dt->seconds = 0u;
  dt->minutes++;
  if (dt->minutes < 60u) {
    return;
  }

  dt->minutes = 0u;
  dt->hours++;
  if (dt->hours < 24u) {
    return;
  }

  dt->hours = 0u;
  if (dt->day >= 1u && dt->day <= 7u) {
    dt->day = (uint8_t)((dt->day % 7u) + 1u);
  } else {
    dt->day = 1u;
  }

  uint16_t year_full = (uint16_t)(2000u + dt->year);
  uint8_t days_in_month = App_DaysInMonth(dt->month, year_full);

  dt->date++;
  if (dt->date <= days_in_month) {
    return;
  }

  dt->date = 1u;
  dt->month++;
  if (dt->month <= 12u) {
    return;
  }

  dt->month = 1u;
  year_full++;
  dt->year = (uint8_t)(year_full % 100u);
  dt->century = (year_full >= 2100u);
}

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

static void App_RtcSimulationInit(DS3231_t *hrtc, DS3231_DateTime *dt, DS3231_Alarm2 *alarm2)
{
  if (hrtc == NULL || dt == NULL || alarm2 == NULL) {
    return;
  }

  hrtc->hi2c = NULL;
  hrtc->i2c_addr = (uint16_t)(DS3231_I2C_ADDR << 1);
  hrtc->sqw_port = NULL;
  hrtc->sqw_pin = 0u;
  hrtc->hour_format = DS3231_FORMAT_24H;
  hrtc->initialized = true;
  hrtc->DS3231_IRQ_Alarm = DS3231_IRQ_NONE;
  hrtc->DS3231_IRQ_Flag = 0u;
  hrtc->oscilator_stopped = 0u;

  dt->seconds = RTC_SIM_START_SECOND;
  dt->minutes = RTC_SIM_START_MINUTE;
  dt->hours = RTC_SIM_START_HOUR;
  dt->ampm = DS3231_AM;
  dt->format = DS3231_FORMAT_24H;
  dt->day = 1u;
  dt->date = 1u;
  dt->month = 1u;
  dt->year = 26u;
  dt->century = false;

  alarm2->minutes = 0u;
  alarm2->hours = 0u;
  alarm2->ampm = DS3231_AM;
  alarm2->format = DS3231_FORMAT_24H;
  alarm2->day_date = 1u;
  alarm2->mode = DS3231_ALM2_EVERY_MINUTE;
}

static void App_RtcSimulationTask(DS3231_t *hrtc, DS3231_DateTime *dt, const DS3231_Alarm2 *alarm2)
{
  if (hrtc == NULL || dt == NULL || alarm2 == NULL) {
    return;
  }

  if (!hrtc->initialized || alarm2->mode != DS3231_ALM2_EVERY_MINUTE) {
    return;
  }

  uint32_t now = HAL_GetTick();
  while ((uint32_t)(now - LastAlarm2Tick) >= RTC_ALARM2_PERIOD_MS) {
    LastAlarm2Tick += RTC_ALARM2_PERIOD_MS;
    App_IncrementOneMinute(dt);
    hrtc->DS3231_IRQ_Alarm = DS3231_IRQ_ALARM2;
    hrtc->DS3231_IRQ_Flag = 1u;
  }
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
  /* USER CODE BEGIN 2 */
  HC595_Init(&ShiftReg595, SER_GPIO_Port, SER_Pin, SRCLK_Pin, SRCLEAR_Pin, RCLK_Pin, OE_Pin);
  HC595_SetOutputEnable(&ShiftReg595, true);
  HC595_SetShiftClear(&ShiftReg595, false);

  IV6_Init(&VfdDisplay, &ShiftReg595);

  App_RtcSimulationInit(&RtcSim, &RtcNow, &RtcAlarm2);
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

    App_RtcSimulationTask(&RtcSim, &RtcNow, &RtcAlarm2);

    if (RtcSim.DS3231_IRQ_Flag != 0u) {
      if ((RtcSim.DS3231_IRQ_Alarm & DS3231_IRQ_ALARM2) != 0u) {
        App_UpdateDisplayFromTime(&VfdDisplay, &RtcNow);
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
