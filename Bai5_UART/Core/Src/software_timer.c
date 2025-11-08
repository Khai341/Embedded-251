/*
 * software_timer.c
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#include "software_timer.h"

#define TIMER_CYCLE_2 1
#define TIMER_CYCLE_3 1
#define TIMER_CYCLE_4 1

uint16_t flag_timer2 = 0;
uint16_t timer2_counter = 0;
uint16_t timer2_MUL = 0;

uint16_t flag_timer3 = 0;
uint16_t timer3_counter = 0;
uint16_t timer3_MUL = 0;

uint16_t flag_timer4 = 0;
uint16_t timer4_counter = 0;
uint16_t timer4_MUL = 0;

void timer_init(){
	HAL_TIM_Base_Start_IT(&htim2);
}

void setTimer2(uint16_t duration){
	timer2_MUL = duration/TIMER_CYCLE_2;
	timer2_counter = timer2_MUL;
	flag_timer2 = 0;
}

void setTimer3(uint16_t duration){
	timer3_MUL = duration/TIMER_CYCLE_3;
	timer3_counter = timer3_MUL;
	flag_timer3 = 0;
}

void setTimer4(uint16_t duration){
	timer4_MUL = duration/TIMER_CYCLE_4;
	timer4_counter = timer4_MUL;
	flag_timer4 = 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		if(timer2_counter > 0){
			timer2_counter--;
			if(timer2_counter == 0) {
				flag_timer2 = 1;
				timer2_counter = timer2_MUL;
			}
		}
		if(timer3_counter > 0){
			timer3_counter--;
			if(timer3_counter == 0) {
				flag_timer3 = 1;
				timer3_counter = timer3_MUL;
			}
		}

		if(timer4_counter > 0){
					timer4_counter--;
					if(timer4_counter == 0) {
						flag_timer4 = 1;
						timer4_counter = timer4_MUL;
					}
				}
		led7_Scan();
	}
}

