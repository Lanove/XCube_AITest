/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "rng.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <main_cxx.h>
#include "network.h"
#include "network_data.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
	NORMAL,
	SAG,
	UNDERVOLTAGE,
	SWELL,
	OVERVOLTAGE
} Gangguan_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t ADC_Value;
/* Global handle to reference the instantiated C-model */
static ai_handle network = AI_HANDLE_NULL;

/* Global c-array to handle the activations buffer */
AI_ALIGNED(32)
static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

/* Array to store the data of the input tensor */
AI_ALIGNED(32)
static ai_float in_data[AI_NETWORK_IN_1_SIZE];
/* or static ai_u8 in_data[AI_NETWORK_IN_1_SIZE_BYTES]; */

/* c-array to store the data of the output tensor */
AI_ALIGNED(32)
static ai_float out_data[AI_NETWORK_OUT_1_SIZE];
/* static ai_u8 out_data[AI_NETWORK_OUT_1_SIZE_BYTES]; */

/* Array of pointer to manage the model's input/output tensors */
static ai_buffer *ai_input;
static ai_buffer *ai_output;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/*
 * Bootstrap
 */
int aiInit(void) {
    ai_error err;

    /* Create and initialize the c-model */
    const ai_handle acts[] = { activations };
    err = ai_network_create_and_init(&network, acts, NULL);
    if (err.type != AI_ERROR_NONE) {
        printf("Error INIT AI Network!\r\n");
    };

    /* Reteive pointers to the model's input/output tensors */
    ai_input = ai_network_inputs_get(network, NULL);
    ai_output = ai_network_outputs_get(network, NULL);

    return 0;
}

/*
 * Run inference
 */
int aiRun(const void *in_data, void *out_data) {
    ai_i32 n_batch;
    ai_error err;

    /* 1 - Update IO handlers with the data payload */
    ai_input[0].data = AI_HANDLE_PTR(in_data);
    ai_output[0].data = AI_HANDLE_PTR(out_data);

    /* 2 - Perform the inference */
    n_batch = ai_network_run(network, &ai_input[0], &ai_output[0]);
    if (n_batch != 1) {
        err = ai_network_get_error(network);
        printf("Error Invoking Network!!\r\n");
        return 1;
    };

    return 0;
}

float RandomNumber(float min, float max){
    uint32_t randomNumber;
    HAL_RNG_GenerateRandomNumber(&hrng,&randomNumber);
    return min + (randomNumber / (float)UINT32_MAX) * (max - min);
}

int ProsesOutput(ai_float* input_data, unsigned int len){
	ai_float maxValue = input_data[0];
	int maxIndex = 0;
	for (int i = 1; i < len; i++) {
	    if (input_data[i] > maxValue) {
	        maxValue = input_data[i];
	        maxIndex = i;
	    }
	}
    return maxIndex;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE {
    HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
    return ch;
}

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADC_Value, 1);
    }
}
*/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    HAL_UART_Transmit(&huart1, (uint8_t*)&ADC_Value, sizeof(ADC_Value), HAL_MAX_DELAY);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_RNG_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */

    /* USER CODE BEGIN WHILE */
  char buffer[256];
  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
  HAL_Delay(500);
  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
  HAL_Delay(500);
  sprintf(buffer,"Hello World\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t*)&buffer, strlen(buffer), HAL_MAX_DELAY);
  aiInit();
  while (1)
  {
    /* USER CODE END WHILE */
	char namaGangguan[16];
    in_data[0] = RandomNumber(0,120);
    in_data[1] = RandomNumber(22,418);
    aiRun(in_data, out_data);
    Gangguan_t gangguan = ProsesOutput(out_data,5);
    switch(gangguan){
    case NORMAL:
    	sprintf(namaGangguan,"Normal");
    	break;
    case SAG:
    	sprintf(namaGangguan,"Sag");
    	break;
    case UNDERVOLTAGE:
    	sprintf(namaGangguan,"Undervoltage");
    	break;
    case SWELL:
    	sprintf(namaGangguan,"Swell");
    	break;
    case OVERVOLTAGE:
    	sprintf(namaGangguan,"Overvoltage");
    	break;
    }
    sprintf(buffer,"Invoked (%.2f,%.2f) = %s\r\n",in_data[0],in_data[1],namaGangguan);
    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
    HAL_UART_Transmit(&huart1, (uint8_t*)&buffer, strlen(buffer), HAL_MAX_DELAY);
    HAL_Delay(500);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
    while (1) {
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
