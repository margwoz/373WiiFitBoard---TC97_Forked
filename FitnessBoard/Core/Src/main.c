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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hx711.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COEF1 6067.3
#define COEF2 5985.7
#define COEF3 6045.2
#define COEF4 5979.8
//Max amount of steps to normalize roll/pitch to. Increase for higher sense (careful not to over-rotate)
#define STEP_NORM 70.1 //MUST FIT IN int8_t [-128 to 127]
//Transmission CMDS (Outside of max normalized step boundary)
#define CMD_TARE 87
#define CMD_RDY 89
//Thresholds for state transition logic
#define PLAY_THRSH 10
#define MOVE_THRSH 5
#define TARE_THRSH 0.5
//Iteration counts for measurement samples and tare cycles
#define NUM_TARES 25
#define NUM_SAMPLES 1
//Dead zones for amplifier noise
#define LOWER_DZ 0.5
#define UPPER_DZ 500
//Tear delay in ms
#define TARE_DELAY 2000
#define PLAY_DELAY 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
//Cell Init
hx711_t cell1, cell2, cell3, cell4;
//State machine for tare, ready, and play states
typedef enum {
	TARE,
	RDY,
	PLAY
} state_t;
//Status print array
const char *state_names[] = { "TARE", "RDY ", "PLAY" };
//Empty tick for tare delay
uint32_t tare_timer_start = 0;
uint32_t play_timer_start = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_USART2_UART_Init();
	MX_USART6_UART_Init();
	/* USER CODE BEGIN 2 */
	//Cell1
	hx711_init(&cell1, clk1_GPIO_Port, clk1_Pin, cell1_GPIO_Port, cell1_Pin);
	hx711_coef_set(&cell1, COEF1); // read after calibration
	HAL_Delay(10);
	//Cell2
	hx711_init(&cell2, clk2_GPIO_Port, clk2_Pin, cell2_GPIO_Port, cell2_Pin);
	hx711_coef_set(&cell2, COEF2); // read after calibration
	HAL_Delay(10);
	//Cell3
	hx711_init(&cell3, clk3_GPIO_Port, clk3_Pin, cell3_GPIO_Port, cell3_Pin);
	hx711_coef_set(&cell3, COEF3); // read after calibration
	HAL_Delay(10);
	//Cell4
	hx711_init(&cell4, clk4_GPIO_Port, clk4_Pin, cell4_GPIO_Port, cell4_Pin);
	hx711_coef_set(&cell4, COEF4); // read after calibration
	HAL_Delay(10);

	//Initial tare before while()
	HAL_Delay(500);
	hx711_tare(&cell1, NUM_TARES);
	hx711_tare(&cell2, NUM_TARES);
	hx711_tare(&cell3, NUM_TARES);
	hx711_tare(&cell4, NUM_TARES);

	//Initial state
	state_t status = RDY;
	int8_t buffer[2] = { 0 };
	double weight1, weight2, weight3, weight4;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//Wait for weight threshold
		weight1 = hx711_weight(&cell1, NUM_SAMPLES);
		weight2 = hx711_weight(&cell2, NUM_SAMPLES);
		weight3 = hx711_weight(&cell3, NUM_SAMPLES);
		weight4 = hx711_weight(&cell4, NUM_SAMPLES);

		//HALF LB dead zone
		if (weight1 < LOWER_DZ || weight1 > UPPER_DZ) weight1 = 0;
		if (weight2 < LOWER_DZ || weight2 > UPPER_DZ) weight2 = 0;
		if (weight3 < LOWER_DZ || weight3 > UPPER_DZ) weight3 = 0;
		if (weight4 < LOWER_DZ || weight4 > UPPER_DZ) weight4 = 0;

		//Calculate sum (NO DIVIDE BY 0)
		float sum = weight1 + weight2 + weight3 + weight4;

		//Pitch and roll math (NORMALIZED)
		int8_t pitch = 0;
		int8_t roll = 0;
		if (sum > 0) {
			pitch = STEP_NORM * (((weight1 + weight4) - (weight2 + weight3)) / sum);
			roll = STEP_NORM * (((weight1 + weight2) - (weight3 + weight4)) / sum);
		}

		//Test print
		//printf("cell1: %.2f | cell2: %.2f | cell3: %.2f | cell4: %.2f <-> roll: %i | pitch %i <-> STATUS: %s\n",
		//		weight1, weight2, weight3, weight4, buffer[0], buffer[1], state_names[status]);

		//State machine for TARE, RDY, and PLAY states
		switch (status) {
		  case TARE:
			//TARE transmission
			buffer[0] = CMD_TARE;
			buffer[1] = CMD_TARE;
			//Transmit command, wait for tare
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			hx711_tare(&cell1, NUM_TARES);
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			hx711_tare(&cell2, NUM_TARES);
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			hx711_tare(&cell3, NUM_TARES);
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			hx711_tare(&cell4, NUM_TARES);
			//Update status
			status = RDY;
			break;

		  case RDY:
			//RDY transmission
			buffer[0] = CMD_RDY;
			buffer[1] = CMD_RDY;
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			//PLAY threshold
			if (sum > PLAY_THRSH) {
				if (play_timer_start == 0) {
					//start timer
					play_timer_start = HAL_GetTick();
				} else if (HAL_GetTick() - play_timer_start >= PLAY_DELAY) {
					status = PLAY;
					play_timer_start = 0;
				}
			} else {
				play_timer_start = 0;
			}
			break;

		  case PLAY:
			//Motor movement threshold
			if (sum > MOVE_THRSH && sum > 0) {
			  buffer[0] = pitch;
			  buffer[1] = roll;
			  //board not empty, reset tare delay
			  tare_timer_start = 0;
			} else {
			  buffer[0] = 0;
			  buffer[1] = 0;
			}
			//PLAY transmission
			HAL_UART_Transmit(&huart6, (uint8_t*) buffer, 2, 100);
			//TARE threshold
			if (sum <= TARE_THRSH) {
			  if (tare_timer_start == 0) {
				  //start timer
				  tare_timer_start = HAL_GetTick();
			  } else if (HAL_GetTick() - tare_timer_start >= TARE_DELAY) {
				  status = TARE;
				  tare_timer_start = 0;
			  }
			} else {
				tare_timer_start = 0;
			}
			break;
		}
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief USART6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART6_UART_Init(void) {

	/* USER CODE BEGIN USART6_Init 0 */

	/* USER CODE END USART6_Init 0 */

	/* USER CODE BEGIN USART6_Init 1 */

	/* USER CODE END USART6_Init 1 */
	huart6.Instance = USART6;
	huart6.Init.BaudRate = 115200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart6) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART6_Init 2 */

	/* USER CODE END USART6_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, clk3_Pin | clk2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, clk4_Pin | clk1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : clk3_Pin clk2_Pin */
	GPIO_InitStruct.Pin = clk3_Pin | clk2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : cell4_Pin */
	GPIO_InitStruct.Pin = cell4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(cell4_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : cell3_Pin */
	GPIO_InitStruct.Pin = cell3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(cell3_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : clk4_Pin clk1_Pin */
	GPIO_InitStruct.Pin = clk4_Pin | clk1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : cell1_Pin cell2_Pin */
	GPIO_InitStruct.Pin = cell1_Pin | cell2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
