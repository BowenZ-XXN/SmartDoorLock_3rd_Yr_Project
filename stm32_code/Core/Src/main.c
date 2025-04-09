/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "menu.h"
#include "as608.h"
#include "delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
uint32_t previousMillis = 0;
uint32_t currentMillis = 0;
uint8_t keyPressed = 0;
char inputBuffer[16];  // 保存按键输入
uint8_t inputIndex = 0;  // 输入索引
char DoorPin[] = "5525"; // Predefined system PIN
uint8_t flag = 0;
uint8_t res = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RX_dat = 0;
uint8_t BT_Flag;
uint8_t facial_flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) // interrupt function
{
    if ((flag == 0) && (huart->Instance == USART2)) // make sure the message comes from UART2
    {
        if (RX_dat == 'o') // when the board receives 'o'
        {
            BT_Flag = 1;
        }
    }


    if ((flag == 13) && (huart->Instance == USART1)) // make sure the message comes from UART1
    {
        if (RX_dat == 'B') // when the board receives 'B'
        {
            facial_flag = 1;
        }
        HAL_UART_Receive_IT(&huart1, &RX_dat, 1); // enable interrupt again
    }
    HAL_UART_Receive_IT(&huart2, &RX_dat, 1); // enable interrupt again
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
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();
  as608_init();
  HAL_UART_Receive_IT(&huart2,&RX_dat,1);//turn on the interrupt of uart2
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      switch (flag) {
          case 0: // Initial menu
              flag = initial_menu();
              break;

          case 1: // Menu to check Door PIN
        	  flag = menu_pin();
              break;

          case 11: // Menu to check Door PIN
        	  flag = unlock_method();
              break;

          case 12: // Menu to check Door PIN
        	  flag = menu_fingerprint_check();
              break;

          case 13:
        	  HAL_UART_Receive_IT(&huart1,&RX_dat,1);//turn on the interrupt of uart2
              facial_flag = 0;
        	  flag = menu_facial_check();
        	  break;

          case 2: // Menu to check System PIN
        	  flag = menu_settings_check();
        	  break;

          case 3: // Menu to choose to edit system settings
        	  flag = menu_settings();
        	  break;

          case 31: // To change the Door PIN
        	  flag = set_doorPIN();
        	  break;

          case 32:
        	  flag = menu_modify_FR();
        	  break;

          case 33:
              flag = menu_record_FR();
              break;

          case 34:
              flag = menu_delete_FR();
              break;

          case 35:
        	  flag = menu_modify_Facial();
			  break;

          default:
              flag = 0; // In case unexpected value occurs, go back to main menu
              break;
      	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  currentMillis = HAL_GetTick();
  if (currentMillis - previousMillis > 200) {
    // Configure GPIO pins : PA4 PA5 PA6 PA7 to GPIO_INPUT
    GPIO_InitStructPrivate.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructPrivate);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
    if(GPIO_Pin == GPIO_PIN_4 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4))
    {
      keyPressed = 68; // ASCII value of D
    }
    else if(GPIO_Pin == GPIO_PIN_5 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
      keyPressed = 67; // ASCII value of C
    }
    else if(GPIO_Pin == GPIO_PIN_6 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6))
    {
      keyPressed = 66; // ASCII value of B
    }
    else if(GPIO_Pin == GPIO_PIN_7 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7))
    {
      keyPressed = 65; // ASCII value of A
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
    if(GPIO_Pin == GPIO_PIN_4 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4))
    {
      keyPressed = 35; // ASCII value of #
    }
    else if(GPIO_Pin == GPIO_PIN_5 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
      keyPressed = 57; // ASCII value of 9
    }
    else if(GPIO_Pin == GPIO_PIN_6 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6))
    {
      keyPressed = 54; // ASCII value of 6
    }
    else if(GPIO_Pin == GPIO_PIN_7 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7))
    {
      keyPressed = 51; // ASCII value of 3
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
    if(GPIO_Pin == GPIO_PIN_4 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4))
    {
      keyPressed = 48; // ASCII value of 0
    }
    else if(GPIO_Pin == GPIO_PIN_5 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
      keyPressed = 56; // ASCII value of 8
    }
    else if(GPIO_Pin == GPIO_PIN_6 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6))
    {
      keyPressed = 53; // ASCII value of 5
    }
    else if(GPIO_Pin == GPIO_PIN_7 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7))
    {
      keyPressed = 50; // ASCII value of 2
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
    if(GPIO_Pin == GPIO_PIN_4 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4))
    {
      keyPressed = 42; // ASCII value of *
    }
    else if(GPIO_Pin == GPIO_PIN_5 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
      keyPressed = 55; // ASCII value of 7
    }
    else if(GPIO_Pin == GPIO_PIN_6 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6))
    {
      keyPressed = 52; // ASCII value of 4
    }
    else if(GPIO_Pin == GPIO_PIN_7 && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7))
    {
      keyPressed = 49; // ASCII value of 1
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
    // Configure GPIO pins : PA4 PA5 PA6 PA7 back to EXTI
    GPIO_InitStructPrivate.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStructPrivate.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructPrivate);
    previousMillis = currentMillis;
  }
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
