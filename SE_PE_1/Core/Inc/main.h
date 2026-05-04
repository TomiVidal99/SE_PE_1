/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OUT_LED_DEBUG_Pin GPIO_PIN_13
#define OUT_LED_DEBUG_GPIO_Port GPIOC
#define BTN_MENU_Pin GPIO_PIN_0
#define BTN_MENU_GPIO_Port GPIOA
#define BTN_MENU_EXTI_IRQn EXTI0_IRQn
#define ADC_Pin GPIO_PIN_1
#define ADC_GPIO_Port GPIOA
#define GPIO330R_Pin GPIO_PIN_5
#define GPIO330R_GPIO_Port GPIOA
#define GPIO10K_Pin GPIO_PIN_6
#define GPIO10K_GPIO_Port GPIOA
#define GPIO1M_Pin GPIO_PIN_7
#define GPIO1M_GPIO_Port GPIOA
#define DEBUG_PIN_Pin GPIO_PIN_8
#define DEBUG_PIN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
