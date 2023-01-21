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
#include "crc.h"
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
typedef enum {
    NORMAL, SAG, UNDERVOLTAGE, SWELL, OVERVOLTAGE
} Gangguan_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define US_PER_VOLT 117120
#define US_PER_mA 2116500
#define US_PER_W 8136000
#define SAMPLE_COUNT 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
struct PowerLineData{
    float voltage_rms;
    float current_rms;
    float apparent_power;
    float real_power;
    Gangguan_t kondisi;
};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

struct PowerLineData pwr;

char buffer[256];
bool cfx_edgeStates[2];
uint32_t cfx_us[2];

uint32_t cfx_t1[2];
uint32_t cfx_t2[2];
uint32_t cfx_ovc[2];
bool cfx_readCurrent;

float voltage_rms;
float current_rms;
float apparent_power;
float real_power;
float t_float= 0.5;

TIM_HandleTypeDef *cfx_htim = &htim5;
uint32_t cfx_timch[2] = {TIM_CHANNEL_3, TIM_CHANNEL_4};
HAL_TIM_ActiveChannel cfx_activtimch[2] = {HAL_TIM_ACTIVE_CHANNEL_3, HAL_TIM_ACTIVE_CHANNEL_4};

static ai_handle network = AI_HANDLE_NULL;

AI_ALIGNED(32) static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
AI_ALIGNED(32) static ai_float in_data[AI_NETWORK_IN_1_SIZE];
AI_ALIGNED(32) static ai_float out_data[AI_NETWORK_OUT_1_SIZE];
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
    const ai_handle acts[] = {activations};
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

float RandomNumber(float min, float max) {
    uint32_t randomNumber;
    HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber);
    return min + (randomNumber / (float) UINT32_MAX) * (max - min);
}

Gangguan_t ProsesOutput(ai_float *input_data, unsigned int len) {
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

const char* gangguanToString(Gangguan_t g){
    if(g == NORMAL)
        return "Normal";
    else if(g==SAG)
        return "Sag";
    else if(g==SWELL)
        return "Swell";
    else if(g==UNDERVOLTAGE)
        return "Undervoltage";
    else if(g==OVERVOLTAGE)
        return "Overvoltage";
    return "Unknown";
}

void printUART(const char *str, size_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t *) str, len, HAL_MAX_DELAY);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    for (int i = 0; i < 2; i++) {
        if ((htim == cfx_htim && (htim->Channel == cfx_activtimch[i]))) {
            cfx_readCurrent = HAL_GPIO_ReadPin(SEL_GPIO_Port, SEL_Pin);
            if (cfx_edgeStates[i] == 0) {
                // Capture T1 & Reverse The ICU Edge Polarity
                cfx_t1[i] = HAL_TIM_ReadCapturedValue(htim, cfx_timch[i]);
                cfx_edgeStates[i] = 1;
                cfx_ovc[i] = 0;
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, cfx_timch[i], TIM_INPUTCHANNELPOLARITY_FALLING);
            } else if (cfx_edgeStates[i] == 1) {
                cfx_t2[i] = HAL_TIM_ReadCapturedValue(htim, cfx_timch[i]);
                cfx_us[i] = (cfx_t2[i] + (cfx_ovc[i] * 2000000)) - cfx_t1[i];
                cfx_edgeStates[i] = 0;
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, cfx_timch[i], TIM_INPUTCHANNELPOLARITY_RISING);
                if (htim->Channel == cfx_activtimch[0]) {
                    real_power = (float) US_PER_W / (float) cfx_us[i];
                    if (real_power > 1000)
                        real_power = 0;
                }
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim ==
        cfx_htim) { // Period of timer is elapsed, reset CFs Input Capture when it's overflowing for 2 times atleast
        for (int i = 0; i < 2; i++) {
            cfx_ovc[i]++;
            if (cfx_ovc[i] >= 2) {
                if (i == 0) {
                    real_power = 0.;
                    current_rms = 0.;
                }
                cfx_ovc[i] = 0;
                if (cfx_edgeStates[i] == 1) {
                    cfx_edgeStates[i] = 0;
                    cfx_us[i] = 0;
                    __HAL_TIM_SET_CAPTUREPOLARITY(htim, cfx_timch[i], TIM_INPUTCHANNELPOLARITY_RISING);
                }
            }
        }
    }
}
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
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_RNG_Init();
  MX_CRC_Init();
  MX_TIM5_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    HAL_TIM_Base_Start_IT(cfx_htim);
    HAL_TIM_IC_Start_IT(cfx_htim, cfx_timch[0]);
    HAL_TIM_IC_Start_IT(cfx_htim, cfx_timch[1]);
    HAL_TIM_Base_Start_IT(&htim3);
    aiInit();
    while (1) {
        static uint32_t lastTick;
        static Gangguan_t lastGangguan;
        char namaGangguan[16];

        HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
        if(cfx_readCurrent)
            current_rms = (float)US_PER_mA/(float)cfx_us[1];
        else
            voltage_rms = (float)US_PER_VOLT/(float)cfx_us[1];
        if(real_power == 0.)
            current_rms = 0.;
        apparent_power = voltage_rms * current_rms/1000.;
        t_float += ((float)HAL_GetTick() - (float)lastTick)/1000.;
        if(t_float >= 120.)
            t_float = 120.;
        in_data[0] = t_float;
        in_data[1] = voltage_rms;
        aiRun(in_data, out_data);
        Gangguan_t gangguan = ProsesOutput(out_data, 5);
        if(lastGangguan != gangguan &&
        !(lastGangguan == SWELL && gangguan == OVERVOLTAGE) &&
        !(lastGangguan == SAG && gangguan == UNDERVOLTAGE))
            t_float = 0.5;
        sprintf(buffer, "%.2fV %.2fmA %.2fVA %.2fW %.2fs %s\r\n", voltage_rms, current_rms, apparent_power, real_power,t_float,
                gangguanToString(gangguan));
        printUART(buffer, strlen(buffer));
        pwr.voltage_rms = voltage_rms;
        pwr.current_rms = current_rms;
        pwr.apparent_power = apparent_power;
        pwr.real_power = real_power;
        pwr.kondisi = gangguan;
        HAL_UART_Transmit(&huart3, (uint8_t *) &pwr, sizeof(struct PowerLineData), HAL_MAX_DELAY);
        bool copy_readCurrent = cfx_readCurrent; // prevent to be toggled from interrupt before switching SEL
        HAL_GPIO_TogglePin(SEL_GPIO_Port, SEL_Pin);
        lastTick = HAL_GetTick();
        lastGangguan = gangguan;
        if(copy_readCurrent)
            HAL_Delay(500);
        else
            HAL_Delay(2000);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
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
