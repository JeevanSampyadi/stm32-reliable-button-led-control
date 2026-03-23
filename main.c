/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Reliable Button + LED Control
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
/* Private variables ---------------------------------------------------------*/
uint32_t lastDebounceTime = 0;
uint32_t pressStartTime = 0;
uint32_t lastBlinkTime = 0;
uint8_t pressCount = 0;
uint8_t blinkRemaining = 0;
uint8_t isPressed = 0;
uint8_t longPressActive = 0;
/* Timing constants */
#define DEBOUNCE_TIME 50
#define LONG_PRESS_TIME 800
#define BLINK_INTERVAL 200
#define FAST_BLINK 100
/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* ================= MAIN ================= */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  while (1)
  {
    uint32_t now = HAL_GetTick();
    GPIO_PinState btn = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    /* ---------- BUTTON PRESS DETECT ---------- */
    if(btn == GPIO_PIN_RESET && !isPressed)
    {
        if(now - lastDebounceTime > DEBOUNCE_TIME)
        {
            isPressed = 1;
            pressStartTime = now;
            longPressActive = 0;
        }
    }
    /* ---------- LONG PRESS ---------- */
    if(isPressed && btn == GPIO_PIN_RESET)
    {
        if((now - pressStartTime > LONG_PRESS_TIME) && !longPressActive)
        {
            longPressActive = 1;
        }
        if(longPressActive && now - lastBlinkTime > FAST_BLINK)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            lastBlinkTime = now;
        }
    }
    /* ---------- BUTTON RELEASE ---------- */
    if(btn == GPIO_PIN_SET && isPressed)
    {
        uint32_t duration = now - pressStartTime;
        if(duration < LONG_PRESS_TIME)
        {
            pressCount++;
            if(pressCount > 5)
                pressCount = 1;
            blinkRemaining = pressCount * 2;
            lastBlinkTime = now;
        }
        isPressed = 0;
        lastDebounceTime = now;
    }
    /* ---------- BLINK ENGINE ---------- */
    if(blinkRemaining > 0 && now - lastBlinkTime > BLINK_INTERVAL)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        blinkRemaining--;
        lastBlinkTime = now;
    }
  }
}
/* ================= EXTI CALLBACK ================= */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_13)
    {
        if(HAL_GetTick() - lastDebounceTime > DEBOUNCE_TIME)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
        lastDebounceTime = HAL_GetTick();
    }
}
/* ================= SYSTEM CLOCK ================= */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType =
  RCC_CLOCKTYPE_HCLK |
  RCC_CLOCKTYPE_SYSCLK |
  RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}
/* ================= GPIO INIT ================= */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* LED (PA5) */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* USER BUTTON (PC13) */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /* EXTI interrupt */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}
/* ================= ERROR HANDLER ================= */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
