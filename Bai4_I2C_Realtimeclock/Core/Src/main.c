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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
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
uint8_t count_led_debug = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void displayTime();
void updateTime();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t ds3231_state = 0;
uint8_t ds3231_state_increase_number = 0;
uint8_t blink = 0;
uint32_t ds3231_increase_number = 0;




uint8_t ds3231_state_increase_number2 = 0;

uint8_t blink2 = 0;
uint8_t timersec = 0;
uint8_t timerhour = 0;
uint8_t timerminute = 0;
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
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 lcd_Clear(BLACK);
 updateTime();
  while (1)
  {
	  while(!flag_timer2);
	  flag_timer2 = 0;
	  button_Scan();
	  if (button_count[0] == 1) {
		  lcd_Clear(BLACK);
		  ds3231_state++;
		  if (ds3231_state >= 3) {
			  ds3231_state = 0;
		  }
	  }
	  if (button_count[12] == 1) {
		  ds3231_state_increase_number++;
		  if (ds3231_state_increase_number >= 7) {
			  ds3231_state_increase_number = 0;
		  }
	  }
	  if (ds3231_state == 0) {

		  ds3231_ReadTime();
		  displayTime();
		  //lcd_ShowStr(20, 0, "ALRM!!!!!!!", WHITE, RED, 24, 0);
		  if (ds3231_sec==timersec && ds3231_min == timerminute && ds3231_hours == timerhour) lcd_ShowStr(20, 0, "ALRM!!!!!!!", WHITE, RED, 24, 0);
	  } else if (ds3231_state == 1) {
		  if (button_count[3] == 1) {



			  ds3231_buffer[ds3231_state_increase_number]++;
			  //ds3231_Write(ds3231_state_increase_number, ds3231_buffer[ds3231_state_increase_number]);


			  /*switch (ds3231_state_increase_number){
				case 0:
					ds3231_Write(ADDRESS_SEC, ds3231_buffer[ds3231_state_increase_number]);
				break;
				case 1:
					ds3231_Write(ADDRESS_MIN, ds3231_buffer[ds3231_state_increase_number]);
				break;
				case 2:
					ds3231_Write(ADDRESS_HOUR, ds3231_buffer[ds3231_state_increase_number]);
				break;
				case 3:
					ds3231_Write(ADDRESS_DAY, ds3231_buffer[ds3231_state_increase_number]);
				  break;
				case 4:
					ds3231_Write(ADDRESS_DATE, ds3231_buffer[ds3231_state_increase_number]);
				break;
				case 5:
					ds3231_Write(ADDRESS_MONTH, ds3231_buffer[ds3231_state_increase_number]);
				break;
				case 6:
					ds3231_Write(ADDRESS_YEAR, ds3231_buffer[ds3231_state_increase_number]);
				break;
				default:
					break;
			  }*/


			  	  ds3231_Write(ADDRESS_SEC, BCD2DEC(ds3231_buffer[0]));
			  	ds3231_Write(ADDRESS_MIN, BCD2DEC(ds3231_buffer[1]));
			  	ds3231_Write(ADDRESS_HOUR, BCD2DEC(ds3231_buffer[2]));
			  	ds3231_Write(ADDRESS_DAY, BCD2DEC(ds3231_buffer[3]));
			  	ds3231_Write(ADDRESS_DATE, BCD2DEC(ds3231_buffer[4]));
			  	ds3231_Write(ADDRESS_MONTH, BCD2DEC(ds3231_buffer[5]));
			  	ds3231_Write(ADDRESS_YEAR, BCD2DEC(ds3231_buffer[6]));
		  }
			ds3231_sec = BCD2DEC(ds3231_buffer[0]);
			ds3231_min = BCD2DEC(ds3231_buffer[1]);
			ds3231_hours = BCD2DEC(ds3231_buffer[2]);
			ds3231_day = BCD2DEC(ds3231_buffer[3]);
			ds3231_date = BCD2DEC(ds3231_buffer[4]);
			ds3231_month = BCD2DEC(ds3231_buffer[5]);
			ds3231_year = BCD2DEC(ds3231_buffer[6]);
		  displayTime1();
		  lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);
	  } else if (ds3231_state == 2) {


		  if (button_count[12] == 1) {
			  ds3231_state_increase_number2++;
		  		  if (ds3231_state_increase_number2 >= 3) {
		  			ds3231_state_increase_number2 = 0;
		  		  }
		  	  }

		  if (button_count[3] == 1) {
			  if (ds3231_state_increase_number2==0){
				  timersec = (timersec+1)%60;
			  }
			  else if (ds3231_state_increase_number2==1){
			  				  timerminute = (timerminute+1)%60;
			  			  }
			  else if (ds3231_state_increase_number2==2){
			  				  timerhour = (timerhour+1)%60;
			  			  }
		  }

		  displayTime2();
		  lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);//lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);


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
	  HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	  HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	  timer_init();
	  led7_init();
	  button_init();
	  lcd_init();
	  ds3231_init();
	  setTimer2(50);
	  setTimer3(250);
}

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_7seg(){
	led7_SetDigit(0, 0, 0);
	led7_SetDigit(5, 1, 0);
	led7_SetDigit(4, 2, 0);
	led7_SetDigit(7, 3, 0);
}
void test_button(){
	for(int i = 0; i < 16; i++){
		if(button_count[i] == 1){
			led7_SetDigit(i/10, 2, 0);
			led7_SetDigit(i%10, 3, 0);
		}
	}
}
//int year = 25; int month = 11, int date = 5, int hour = 9, int min = 10, int sec = 11
void updateTime(){


	ds3231_buffer[0] = 25;
	ds3231_buffer[1] = 11;
	ds3231_buffer[2] = 5;
	ds3231_buffer[3] = 4;
	ds3231_buffer[4] = 9;
	ds3231_buffer[5] = 10;
	ds3231_buffer[6] = 11;

	ds3231_Write(ADDRESS_YEAR, 25);
	ds3231_Write(ADDRESS_MONTH, 11);
	ds3231_Write(ADDRESS_DATE, 05);
	ds3231_Write(ADDRESS_DAY, 4);
	ds3231_Write(ADDRESS_HOUR, 9);
	ds3231_Write(ADDRESS_MIN, 10);
	ds3231_Write(ADDRESS_SEC, 11);
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
    if (button_count[7] == 1)
        return 1;
    else
        return 0;
}
void displayTime(){
	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);
}

void displayTime1(){
	if(flag_timer3) {
		flag_timer3 = 0;
		if (ds3231_state_increase_number == 0) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(150, 100, ds3231_sec, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
		}
		if (ds3231_state_increase_number == 1) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(110, 100, ds3231_min, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
		}
		if (ds3231_state_increase_number == 2) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(70, 100, ds3231_hours, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
		}
		if (ds3231_state_increase_number == 3) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(20, 130, ds3231_day, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
		}
		if (ds3231_state_increase_number == 4) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(70, 130, ds3231_date, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
		}
		if (ds3231_state_increase_number == 5) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(110, 130, ds3231_month, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
		}
		if (ds3231_state_increase_number == 6) {
			if (blink == 0) {
				blink = 1;
				lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
			} else {
				blink = 0;
				lcd_ShowIntNum(150, 130, ds3231_year, 2, BLACK, BLACK, 24);
			}
		} else {
			lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
		}
	}
}

void displayTime2(){
	if (flag_timer3) {
	    flag_timer3 = 0;


	    lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
	    	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
	    	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
	    	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	    	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	    	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	    	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);



	    	switch (ds3231_state_increase_number2) {
	    	        case 0: // Blink seconds
	    	            if (blink2 == 0) {
	    	            	blink2 = 1;
	    	                lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
	    	            } else {
	    	            	blink2 = 0;
	    	                lcd_ShowIntNum(150, 160, timersec, 2, BLACK, BLACK, 24);
	    	            }
	    	            // Show others normally
	    	            lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
	    	            lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
	    	            break;

	    	        case 1: // Blink minutes
	    	            if (blink2 == 0) {
	    	            	blink2 = 1;
	    	                lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
	    	            } else {
	    	            	blink2 = 0;
	    	                lcd_ShowIntNum(110, 160, timerminute, 2, BLACK, BLACK, 24);
	    	            }
	    	            lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
	    	            lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
	    	            break;

	    	        case 2: // Blink hours
	    	            if (blink2 == 0) {
	    	            	blink2 = 1;
	    	                lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
	    	            } else {
	    	            	blink2 = 0;
	    	                lcd_ShowIntNum(70, 160, timerhour, 2, BLACK, BLACK, 24);
	    	            }
	    	            lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
	    	            lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
	    	            break;

	    	        default: // No blinking (show all normally)
	    	            lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
	    	            lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
	    	            lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
	    	            break;
	    	    }
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
