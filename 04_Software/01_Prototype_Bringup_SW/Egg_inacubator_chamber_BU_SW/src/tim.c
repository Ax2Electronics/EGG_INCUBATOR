#include "tim.h"

TIM_HandleTypeDef htim3;

void TIM3_PWM_Init(void) {

    /* 1. Ukljuci clock za GPIOA i TIM3 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* 2. Konfiguriši PA6 kao AF02 (TIM3_CH1) */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin       = GPIO_PIN_6;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF2_TIM3;      /* TIM3_CH1 = AF02 na PA6 */
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 3. Konfiguriši TIM3 bazu */
    htim3.Instance               = TIM3;
    htim3.Init.Prescaler         = 83;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 19999;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim3);

    /* 4. Konfiguriši CH1 kao PWM Mode 1 */
    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 1500;               /* Pocetna pozicija — 90° */
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &oc, TIM_CHANNEL_1);

    /* 5. Pokreni PWM output */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}