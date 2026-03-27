#ifndef STEPPER_H
#define STEPPER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

void Stepper_Init(void);
void Stepper_Update(uint32_t now);

void Stepper_MoveToAngle(float angle_deg);
void Stepper_MoveByAngle(float delta_angle_deg);
void Stepper_Home(void);
float Stepper_GetAngle(void);

bool Stepper_IsBusy(void);

#endif