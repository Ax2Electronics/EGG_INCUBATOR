#include "stm32f4xx_hal.h"
//#include "tim.h"
//#include "servo.h"
#include "stepper.h"

static void SystemClock_Config(void);

#define ANGLE_SWING  38.0f

int main(void) {
    HAL_Init();
    SystemClock_Config();

    /* Inicijalizacija periferija */
    //TIM3_PWM_Init();
    //Servo_Init();
    Stepper_Init();

    /* Servo proof-of-life (opciono) */
    //Servo_SetAngle(TRAY_CENTER);
    //HAL_Delay(1000u);

    /* Home stepper */
    Stepper_Home();

    /* Stepper self-test: pomeri 38° levo-desno 3 puta */
    for (int i = 0; i < 3; i++) {
        Stepper_MoveToAngle(ANGLE_SWING);
        while (Stepper_IsBusy()) {
            Stepper_Update(HAL_GetTick());
        }
        HAL_Delay(200);

        Stepper_MoveToAngle(-ANGLE_SWING);
        while (Stepper_IsBusy()) {
            Stepper_Update(HAL_GetTick());
        }
        HAL_Delay(200);
    }
    Stepper_Home();
    HAL_Delay(500);

    /* Superloop */
    uint32_t now = 0u;
    uint8_t state = 0u; // 0: +38, 1: -38

    while (1) {
        now = HAL_GetTick();

        Stepper_Update(now);

        if (!Stepper_IsBusy()) {
            if (state == 0u) {
                Stepper_MoveToAngle(ANGLE_SWING);
                state = 1u;
            } else {
                Stepper_MoveToAngle(-ANGLE_SWING);
                state = 0u;
            }
            // mala pauza izmedju pozicija (opciono):
            HAL_Delay(200u);
        }

        /* ... ostali taskovi ... */
    }
}


static void SystemClock_Config(void) {
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState            = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState        = RCC_PLL_ON;
    osc.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM            = 16;
    osc.PLL.PLLN            = 336;
    osc.PLL.PLLP            = RCC_PLLP_DIV4;
    osc.PLL.PLLQ            = 7;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                         RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

