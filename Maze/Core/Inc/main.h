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
#include "stm32l4xx_hal.h"

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
#define BEAM_BREAK_TWO_Pin GPIO_PIN_8
#define BEAM_BREAK_TWO_GPIO_Port GPIOF
#define BEAM_BREAK_TWO_EXTI_IRQn EXTI9_5_IRQn
#define RST1_Pin GPIO_PIN_0
#define RST1_GPIO_Port GPIOC
#define STP1_Pin GPIO_PIN_1
#define STP1_GPIO_Port GPIOC
#define SLP_Pin GPIO_PIN_3
#define SLP_GPIO_Port GPIOC
#define EN1_Pin GPIO_PIN_3
#define EN1_GPIO_Port GPIOA
#define DIR1_Pin GPIO_PIN_4
#define DIR1_GPIO_Port GPIOC
#define CS_Pin GPIO_PIN_12
#define CS_GPIO_Port GPIOF
#define BEAM_BREAK_Pin GPIO_PIN_0
#define BEAM_BREAK_GPIO_Port GPIOG
#define BEAM_BREAK_EXTI_IRQn EXTI0_IRQn
#define EN2_Pin GPIO_PIN_3
#define EN2_GPIO_Port GPIOD
#define STP2_Pin GPIO_PIN_4
#define STP2_GPIO_Port GPIOD
#define DIR2_Pin GPIO_PIN_7
#define DIR2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
