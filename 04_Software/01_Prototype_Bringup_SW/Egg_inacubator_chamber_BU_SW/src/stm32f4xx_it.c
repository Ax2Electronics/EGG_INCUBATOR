#include "stm32f4xx_hal.h"

/* SysTick — ovo je srce HAL_Delay() i HAL_GetTick() */
void SysTick_Handler(void) {
    HAL_IncTick();
}

/* Ostali fault handleri — korisni za debug */
void NMI_Handler(void) {
    while (1);
}

void HardFault_Handler(void) {
    /* Brzo trepćanje LED = HardFault */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIOA->MODER |= (1u << 10u);
    while (1) {
        GPIOA->ODR ^= (1u << 5u);
        for (volatile uint32_t i = 0; i < 50000u; i++);
    }
}

void MemManage_Handler(void) {
    while (1);
}

void BusFault_Handler(void) {
    while (1);
}

void UsageFault_Handler(void) {
    while (1);
}