/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "sensor.h"
#include "buzzer.h"
#include "uart.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void test_Buzzer();
void test_Adc();
void test_Uart();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t timer_run = 1;
int hour = 15;
int minute = 20;

int buzzer_bool = 0;

float arr[10] = {-1};
int LIGHT_THREADSHOLD = 1000;
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
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_TIM13_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 lcd_Clear(BLACK);
 led7_SetDigit(hour/10,0,0);
 led7_SetDigit(hour%10,1,0);
 led7_SetDigit(minute/10,2,0);
 led7_SetDigit(minute%10,3,0);
  while (1)
  {
	  //while(!flag_timer2);
	  if (flag_timer3){
		  timer_run++;
		  flag_timer3 = 0;

		  if (timer_run>=60){
			  timer_run=0;
			  minute++;
		  }
		  if (minute>=60){
			  minute=0;
			  hour++;
		  }
		  if (hour>=24){
			  hour = 0;
		  }
//		  uart_Rs232SendString("A");
		  led7_SetDigit(hour/10,0,0);
		   led7_SetDigit(hour%10,1,0);
		   led7_SetDigit(minute/10,2,0);
		   led7_SetDigit(minute%10,3,0);



		   char temp[100];
		   char temp2[100];
		   int nhietdo = 123;
		   float humi = 3.14f;

		   uart_Rs232SendString("---------------\n");
		   float b = sensor_GetVoltage() * sensor_GetCurrent();
		   sprintf(temp, "Power:%f\n", b);
		   uart_Rs232SendString(temp);

		   b = sensor_GetTemperature();
		   sprintf(temp, "Temp:%f\n", b);
		   uart_Rs232SendString(temp);

		   if (sensor_GetLight()<=LIGHT_THREADSHOLD) uart_Rs232SendString("Light: Weak\n");
		   else uart_Rs232SendString("Light: Strong\n");



		   int a = (sensor_GetPotentiometer() * 100) / 4095;
		   sprintf(temp, "Humi:%d\n", a);
		   //strcat(temp, temp);

		   uart_Rs232SendString(temp);


		   if (a>70){
		  buzzer_bool = 1-buzzer_bool;

		  if (buzzer_bool) buzzer_SetVolume(25);
		  //lcd_ShowStr(0, 0, "!!!!!!:", RED, BLACK, 16, 0);
		  else {buzzer_SetVolume(0);buzzer_SetVolume(0);}
		   }
		   else {buzzer_bool=0;buzzer_SetVolume(0);}

		   if (timer_run%15==0){
			   //lcd_ShowIntNum(0, 160, a, 4, RED, BLACK, 16);
			   for (int i = 0; i < 10-1; i++)
					{
						arr[i] = arr[i+1];
					}
					arr[9] = b;
		   }
	  }
	  if (!flag_timer2) continue;

	  flag_timer2 = 0;
	  button_Scan();
	  test_LedDebug();
	  test_Adc();
	  test_Buzzer();
	  test_Uart();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_init(){
	  timer_init();
	  button_init();
	  lcd_init();
	  sensor_init();
	  buzzer_init();
	  setTimer2(50);
	  setTimer3(1000);
}

uint8_t count_led_debug = 0;

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

uint8_t isButtonUp()
{
    if (button_count[3] == 1)
        return 1;
    else
        return 0;
}

uint8_t isButtonDown()
{
    if (button_count[7] == 1){
    	timer_run=0;
        return 1;
    }
    else
        return 0;
}

uint8_t isButtonRight()
{
    if (button_count[11] == 1)
        return 1;
    else
        return 0;
}

uint8_t count_adc = 0;
//

//

#define CHART_X0 10
#define CHART_Y0 180
#define CHART_W 200
#define CHART_H 125

// Example: arr[10] = { -1, 12, 16, -1, 22, 19, 25, -1, 30, 27 };

/*void drawChart(int *arr, int n)
{
    int prevValid = 0;
    int prevValue = -1;

    for (int i = 0; i < n; i++)
    {
        if (arr[i] == -1)
            continue; // skip invalid

        int x = CHART_X0 + (i * (CHART_W / (n - 1)));

        // Map arr[i] to Y coordinate (top = min value)
        // Example: arr range 0–100
        int y = CHART_Y0 + CHART_H - (arr[i] * CHART_H / 100);

        // Draw point
        lcd_DrawPoint(x, y, BLUE);

        // If previous was valid, draw line between them
        if (prevValue != -1)
        {
            lcd_DrawLine(prevValid, prevValue, x, y, BLUE);
        }

        prevValid = x;
        prevValue = y;
    }
}
int MAX_VALUE = 50;   // you can change this anytime

void drawChart(int *arr, int n)
{
    int prevX = -1;
    int prevY = -1;

    for (int i = 0; i < n; i++)
    {
        if (arr[i] < 0)     // -1 = invalid
            continue;

        int value = arr[i];

        // Clip to max (prevent going outside the chart)
        if (value > MAX_VALUE)
            value = MAX_VALUE;

        // X coordinate evenly spaced across width
        int x = CHART_X0 + (i * (CHART_W / (n - 1)));

        // Scale value to screen height (0 = bottom, max = top)
        int y = CHART_Y0 + CHART_H - (value * CHART_H / MAX_VALUE);

        // Draw point
        lcd_DrawPoint(x, y, BLUE);

        // Draw line only if previous point existed
        if (prevX != -1)
        {
            lcd_DrawLine(prevX, prevY, x, y, BLUE);
        }

        prevX = x;
        prevY = y;
    }
}*/
int MAX_VALUE = 200;   // you can change this anytime

void drawChart(float *arr, int n)
{
    int prevX = -1;
    int prevY = -1;

    for (int i = 0; i < n; i++)
    {
        if (arr[i] < 0.0f)     // negative = invalid
            continue;

        float value = arr[i];

        // Clip to max
        if (value > MAX_VALUE)
            value = MAX_VALUE;

        // X coordinate
        int x = CHART_X0 + (i * (CHART_W / (n - 1)));

        // Scale value to screen height
        int y = CHART_Y0 + CHART_H - (int)(value * CHART_H / MAX_VALUE);

        // Draw point
        lcd_DrawPoint(x, y, BLUE);

        // Draw line if previous point exists
        if (prevX != -1)
            lcd_DrawLine(prevX, prevY, x, y, BLUE);

        prevX = x;
        prevY = y;
    }
}


//int arr[10] = {-1};
//int  arr[10] = { -1, 12, 16, -1, 22, 19, 25, -1, 30, 27 };
//float arr[10] = {-1};
void test_Adc(){
	count_adc = (count_adc + 1)%20;
	if(count_adc == 0){
		sensor_Read();
		int a = (sensor_GetPotentiometer() * 100) / 4095;


		float b = sensor_GetVoltage() * sensor_GetCurrent();
//		for (int i = 0; i < 10-1; i++)
//		{
//			arr[i] = arr[i+1];
//		}
//		arr[9] = b;

		lcd_ShowStr(10, 100, "Power:", RED, BLACK, 16, 0);
		//lcd_ShowFloatNum(130, 100,sensor_GetVoltage(), 4, RED, BLACK, 16);
		lcd_ShowFloatNum(130, 100,b, 4, RED, BLACK, 16);

		lcd_ShowStr(10, 140, "Light:", RED, BLACK, 16, 0);

		if (sensor_GetLight()<=LIGHT_THREADSHOLD) lcd_ShowStr(130, 140, "Weak", RED, BLACK, 16, 0);
		else lcd_ShowStr(130, 140, "Strong", RED, BLACK, 16, 0);
		//lcd_ShowIntNum(130, 140, sensor_GetLight(), 4, RED, BLACK, 16);//sensor_GetTemperature

		lcd_ShowStr(10, 120, "Temp:", RED, BLACK, 16, 0);
//		lcd_ShowFloatNum(130, 120,sensor_GetCurrent(), 4, RED, BLACK, 16);
		lcd_ShowFloatNum(130, 120,sensor_GetTemperature(), 4, RED, BLACK, 16);


		lcd_ShowStr(10, 160, "Humi:", RED, BLACK, 16, 0);//Potentiometer
		lcd_ShowIntNum(130, 160, a, 4, RED, BLACK, 16);

		//lcd_ShowStr(10, 160, "Potentiometer:", RED, BLACK, 16, 0);
		lcd_Fill(10, 180, 10+200, 180 + 125, WHITE);
		drawChart(arr,10);
		//lcd_DrawLine(0,0,50,50,RED);

		//lcd_ShowStr(10, 180, "Temperature:", RED, BLACK, 16, 0);
		//lcd_ShowFloatNum(130, 180,sensor_GetTemperature(), 4, RED, BLACK, 16);timer_run
		//lcd_Fill(130, 180, 130 + 40, 180 + 16, BLACK);
		//lcd_ShowIntNum(130, 180,timer_run, 4, RED, BLACK, 16);
	}
}

void test_Buzzer(){
	if(isButtonUp()){
		buzzer_SetVolume(50);
	}

	if(isButtonDown()){
		buzzer_SetVolume(0);
	}

	if(isButtonRight()){
		buzzer_SetVolume(25);
	}

}

void test_Uart() {
  if (button_count[12] == 1) {
    // Example: send character 'A'
    uart_Rs232SendString("A");

    // When message received
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
