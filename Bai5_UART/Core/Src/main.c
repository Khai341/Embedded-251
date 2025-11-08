/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (Combined DS3231 + UART)
  ******************************************************************************
  * @attention
  *
  * This software is a combination of DS3231 clock with LCD display and UART
  * communication via USART1.
  *
  * Based on original code © 2023 STMicroelectronics, BSD 3-Clause License.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
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
#include "uart.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t count_led_debug = 0;
uint8_t ds3231_state = 0;
uint8_t ds3231_state_increase_number = 0;
uint8_t blink = 0;
uint32_t ds3231_increase_number = 0;

uint8_t ds3231_state_increase_number2 = 0;
uint8_t blink2 = 0;
uint8_t timersec = 0;
uint8_t timerhour = 0;
uint8_t timerminute = 0;

uint32_t mode3phase = 0;

extern uint8_t receive_buffer1;
extern uint8_t receive_buffer_string[100];
extern int head;
/////extern int tail;
extern int flag;

extern uint8_t ds3231_buffer[7];
extern uint16_t flag_timer3;

extern uint16_t flag_timer4;


int timeout = 0;
int retry = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void test_Uart();
void displayTime();
void displayTime1();
void displayTime2();
void updateTime();
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  system_init();
  lcd_Clear(BLACK);
  updateTime();

  while (1)
  {
    while (!flag_timer2);
    flag_timer2 = 0;
    checkUart();////
    button_Scan();
    test_LedDebug();

    /* UART test function */
    test_Uart();

    /* ==============================
     * DS3231 + LCD Display States
     * ============================== */
    if (button_count[0] == 1) {
      lcd_Clear(BLACK);
      ds3231_state++;
      if (ds3231_state >= 4) ds3231_state = 0;
      timeout = 0;
retry = 0;
    }

    if (button_count[12] == 1) {
      ds3231_state_increase_number++;
      if (ds3231_state_increase_number >= 7)
        ds3231_state_increase_number = 0;
    }

    if (ds3231_state == 0) {
      ds3231_ReadTime();
      displayTime();

      // Alarm check
      if (ds3231_sec == timersec && ds3231_min == timerminute && ds3231_hours == timerhour)
        lcd_ShowStr(20, 0, "ALRM!!!!!!!", WHITE, RED, 24, 0);

    } else if (ds3231_state == 1) {
      if (button_count[3] == 1) {
        ds3231_buffer[ds3231_state_increase_number]++;
        ds3231_Write(ADDRESS_SEC, BCD2DEC(ds3231_buffer[0]));
        ds3231_Write(ADDRESS_MIN, BCD2DEC(ds3231_buffer[1]));
        ds3231_Write(ADDRESS_HOUR, BCD2DEC(ds3231_buffer[2]));
        ds3231_Write(ADDRESS_DAY, BCD2DEC(ds3231_buffer[3]));
        ds3231_Write(ADDRESS_DATE, BCD2DEC(ds3231_buffer[4]));
        ds3231_Write(ADDRESS_MONTH, BCD2DEC(ds3231_buffer[5]));
        ds3231_Write(ADDRESS_YEAR, BCD2DEC(ds3231_buffer[6]));
      }

      ds3231_sec   = BCD2DEC(ds3231_buffer[0]);
      ds3231_min   = BCD2DEC(ds3231_buffer[1]);
      ds3231_hours = BCD2DEC(ds3231_buffer[2]);
      ds3231_day   = BCD2DEC(ds3231_buffer[3]);
      ds3231_date  = BCD2DEC(ds3231_buffer[4]);
      ds3231_month = BCD2DEC(ds3231_buffer[5]);
      ds3231_year  = BCD2DEC(ds3231_buffer[6]);

      displayTime1();
      lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);

    } else if (ds3231_state == 2) {
      if (button_count[12] == 1) {
        ds3231_state_increase_number2++;
        if (ds3231_state_increase_number2 >= 3)
          ds3231_state_increase_number2 = 0;
      }

      if (button_count[3] == 1) {
        if (ds3231_state_increase_number2 == 0)
          timersec = (timersec + 1) % 60;
        else if (ds3231_state_increase_number2 == 1)
          timerminute = (timerminute + 1) % 60;
        else if (ds3231_state_increase_number2 == 2)
          timerhour = (timerhour + 1) % 24;
      }

      displayTime2();
      lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);
    }
    else if (ds3231_state == 3) {
    	switch (mode3phase){
    	case 0:
    		lcd_ShowStr(40, 40, "Get hour", WHITE, RED, 24, 0);
    		break;
    	case 1:
    		lcd_ShowStr(40, 40, "Get minute", WHITE, RED, 24, 0);
    		    		break;
    	case 2:
    		lcd_ShowStr(40, 40, "Get second", WHITE, RED, 24, 0);
    		break;
    	default:
    		break;
    	}
    	/*lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
    	  lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
    	  lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);*/
    	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
    	  lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
    	  lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
    	  lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);
    }
  }
}

/* ===========================
 *  System & Peripheral Setup
 * =========================== */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    Error_Handler();
}

/* ===========================
 *  System Initialization
 * =========================== */
void system_init() {
  HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
  HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);

  timer_init();
  led7_init();
  button_init();
  lcd_init();
  ds3231_init();
  uart_init_rs232();

  setTimer2(50);
  setTimer3(250);


  setTimer4(250);
}

/* ===========================
 *  Helper Functions
 * =========================== */
void test_LedDebug() {
  count_led_debug = (count_led_debug + 1) % 20;
  if (count_led_debug == 0)
    HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
}

void updateTime() {
  ds3231_Write(ADDRESS_YEAR, 25);
  ds3231_Write(ADDRESS_MONTH, 11);
  ds3231_Write(ADDRESS_DATE, 5);
  ds3231_Write(ADDRESS_DAY, 4);
  ds3231_Write(ADDRESS_HOUR, 9);
  ds3231_Write(ADDRESS_MIN, 10);
  ds3231_Write(ADDRESS_SEC, 11);
}

/* ===========================
 * UART Communication
 * =========================== */





void test_Uart() {
  if (button_count[12] == 1) {
    // Example: send character 'A'
    uart_Rs232SendString("A");

    // When message received
    if (flag == 1) {
      flag = 0;
      __disable_irq();
      uart_Rs232SendString(receive_buffer_string);
      __enable_irq();

      memset(receive_buffer_string, 0, sizeof(receive_buffer_string));
      head = 0;
    }
  }
}
//int timeout = 0;
//int retry = 0;
void checkUart(){
	if (flag_timer4){
		flag_timer4 = 0;
		timeout++;
	}

	if (flag == 1 && ds3231_state==3) {
		flag = 0;
		int tail = 0;
		int temp = 0;
		while (tail != 100 && receive_buffer_string[tail] != 0){
			//uart_Rs232SendString("A");
			temp = temp*10 + (receive_buffer_string[tail]-'0');
		tail++;
		}

		memset(receive_buffer_string, 0, sizeof(receive_buffer_string));
		      head = 0;
		      tail = 0;
		//uart_Rs232SendString(temp);
		// Convert integer to string
		char temp_str[12]; // enough for 32-bit int
		sprintf(temp_str, "%d", temp);


		// Send string
		//////uart_Rs232SendString(temp_str);
/*ds3231_Write(ADDRESS_SEC, BCD2DEC(ds3231_buffer[0]));
        ds3231_Write(ADDRESS_MIN, BCD2DEC(ds3231_buffer[1]));
        ds3231_Write(ADDRESS_HOUR, BCD2DEC(ds3231_buffer[2]));*/
switch (mode3phase){
    	case 0:
    		if (temp>=0 && temp<=23){
    			uart_Rs232SendString(temp_str);
    			retry=0;
    			ds3231_hours = temp;timeout = 0;ds3231_Write(ADDRESS_HOUR, temp);mode3phase = (mode3phase+1)%3;}
    		break;
    	case 1:
    		if (temp>=0 && temp<=59){
    			uart_Rs232SendString(temp_str);
    			retry=0;
    			ds3231_min = temp;timeout = 0;ds3231_Write(ADDRESS_MIN, temp);mode3phase = (mode3phase+1)%3;}
    		    		break;
    	case 2:
    		if (temp>=0 && temp<=59){
    			uart_Rs232SendString(temp_str);
    			ds3231_sec = temp;
    		ds3231_state = 0;
    		retry=0;
    		lcd_Clear(BLACK);timeout = 0;ds3231_Write(ADDRESS_SEC, temp);mode3phase = (mode3phase+1)%3;}
    		break;
    	default:
    		break;
    	}

//mode3phase = (mode3phase+1)%3;

	}

	if (ds3231_state==3 && timeout>=40){
		timeout = 0;
			if (retry>=2){
				mode3phase=0;
			ds3231_state = 0;
			retry=0;timeout=0;
			lcd_Clear(BLACK);}
			else{
				retry++;
				//lcd_ShowStr(60, 200, "Retry", WHITE, RED, 24, 0);
				//lcd_ShowIntNum(60, 230, retry, 1, RED, BLACK, 24);
			}
		}
	if (ds3231_state==3) {lcd_ShowIntNum(60, 230, retry, 1, RED, BLACK, 24);lcd_ShowStr(60, 200, "Retry", WHITE, RED, 24, 0);}
}

/* ===========================
 * Display functions
 * =========================== */
void displayTime() {
  lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
  lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
  lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
  lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);

  lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
  lcd_ShowIntNum(30, 30, ds3231_state, 1, RED, BLACK, 24);
}

/* (Keep displayTime1() and displayTime2() as in version 1) */
void displayTime1() {
    if (flag_timer3) {
        flag_timer3 = 0;

        // --- Seconds ---
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

        // --- Minutes ---
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

        // --- Hours ---
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

        // --- Day ---
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

        // --- Date ---
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

        // --- Month ---
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

        // --- Year ---
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


void displayTime2() {
    if (flag_timer3) {
        flag_timer3 = 0;

        // Display current DS3231 time and date
        lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
        lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
        lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
        lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
        lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
        lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
        lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);

        // Handle blinking for timer adjustment
        switch (ds3231_state_increase_number2) {
            case 0: // Blink seconds
                if (blink2 == 0) {
                    blink2 = 1;
                    lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
                } else {
                    blink2 = 0;
                    lcd_ShowIntNum(150, 160, timersec, 2, BLACK, BLACK, 24);
                }
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

            default: // No blinking
                lcd_ShowIntNum(70, 160, timerhour, 2, YELLOW, BLACK, 24);
                lcd_ShowIntNum(110, 160, timerminute, 2, YELLOW, BLACK, 24);
                lcd_ShowIntNum(150, 160, timersec, 2, YELLOW, BLACK, 24);
                break;
        }
    }
}

/* Copy them here if needed – omitted for brevity */

void Error_Handler(void)
{
  __disable_irq();
  while (1);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Optionally print debug info */
}
#endif
