#ifndef STM32F4XX_HAL_CONF_H
#define STM32F4XX_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED

#define HSE_VALUE    25000000U
#define HSI_VALUE    16000000U
#define HSE_STARTUP_TIMEOUT    100U
#define LSE_STARTUP_TIMEOUT    5000U
#define LSE_VALUE              32768U
#define EXTERNAL_CLOCK_VALUE   12288000U
#define VDD_VALUE              3300U
#define TICK_INT_PRIORITY      0x0FU
#define USE_RTOS               0U
#define USE_HAL_DRIVER

#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_pwr.h"

#endif /* STM32F4XX_HAL_CONF_H */