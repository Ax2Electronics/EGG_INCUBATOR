#include "stm32f4xx_hal.h"
#include "tim.h"
#include "servo.h"

static void SystemClock_Config(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();

    /* Inicijalizacija periferija */
    TIM3_PWM_Init();
    Servo_Init();

    /* Potvrdi poziciju pri startu */
    Servo_SetAngle(TRAY_CENTER);
    HAL_Delay(3000u);
    Servo_SetAngle(TRAY_LEFT);
    HAL_Delay(3000u);
    Servo_SetAngle(TRAY_RIGHT);
    HAL_Delay(3000u);

    /* Pokreni prvo okretanje — levo */
    //Servo_StartMoveTo(TRAY_LEFT);
    //Servo_DemoSequence();
    /* Superloop */
    uint32_t now = 0u;

    while (1) {
        now = HAL_GetTick();

        /* Servo non-blocking update — mora biti svaki ciklus */
        Servo_Update(now);
    // Zadaj novo kretanje samo kada je servo slobodan
    if (!Servo_IsBusy()) {
        if (Servo_GetAngle() == TRAY_LEFT) {
            Servo_StartMoveTo(TRAY_RIGHT);
        } else {
            Servo_StartMoveTo(TRAY_LEFT);
        }
    }
        /* ------------------------------------------------
         * Ovde ce ici ostali taskovi:
         *
         * Task_ReadSensors(now);
         * Task_TemperatureControl(now);
         * Task_HumidityControl(now);
         * Task_SafetyCheck(now);
         * Task_EggTurning(now);
         * Task_UpdateDisplay(now);
         * Task_DataLogging(now);
         * Task_AlarmHandler(now);
         * ------------------------------------------------ */
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
