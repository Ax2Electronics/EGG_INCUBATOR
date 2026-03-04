#ifndef TIM_H
#define TIM_H

#include "stm32f4xx_hal.h"

/* ----------------------------------------------------------
 *  TIM3 — PWM za servo
 *
 *  Pin     : PA6 (TIM3_CH1, AF02)
 *  Clock   : APB1 Timer clock = 84MHz
 *  Prescaler = 83   → Timer clock = 1MHz (1 tik = 1us)
 *  Period    = 19999 → 20ms period = 50Hz PWM
 * ---------------------------------------------------------- */
extern TIM_HandleTypeDef htim3;

void TIM3_PWM_Init(void);

#endif /* TIM_H */