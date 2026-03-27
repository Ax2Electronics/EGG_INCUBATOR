#include "stepper.h"

/* --- CONFIG --- */
#define STEPS_PER_REV      3200.0f   // 200*16 microstep
#define STEP_ANGLE         (360.0f / STEPS_PER_REV)
#define STEP_INTERVAL_MS   2         // brzina (podesi)

/* --- GPIO (fizički pinovi za STM32F401RE / Nucleo za DRV8462) --- */
/* STEP  -> PB0 */
#define STEP_PORT GPIOB
#define STEP_PIN  GPIO_PIN_0

/* DIR   -> PB1 */
#define DIR_PORT  GPIOB
#define DIR_PIN   GPIO_PIN_1

/* CS    -> PB6 (D10) */
#define DRV_CS_PORT GPIOB
#define DRV_CS_PIN  GPIO_PIN_6

/* SPI1: SCK=PA5 (D13), MISO=PA6 (D12), MOSI=PA7 (D11) */
#define SPI_SCK_PORT GPIOA
#define SPI_SCK_PIN  GPIO_PIN_5

/* --- STATE --- */
static int32_t target_steps = 0;
static int32_t current_steps = 0;
static uint32_t last_step_time = 0;
static bool busy = false;

static SPI_HandleTypeDef hspi1;

static void Stepper_SPI_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    // SPI1 SCK/MISO/MOSI - PA5/PA6/PA7
    gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);

    // DRV CS - PD10
    gpio.Pin = DRV_CS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DRV_CS_PORT, &gpio);

    // STEP and DIR outputs
    gpio.Pin = STEP_PIN | DIR_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(STEP_PORT, &gpio);

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;  // CPOL = 0
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;    // CPHA = 1: device propagates SDO on rising, captures SDI on falling
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        // SPI inicijalizacija nije uspela, dodaj ovde fallback/errhandler po potrebi
        while (1);
    }
}



static void DRV_Write(uint8_t addr, uint8_t data)
{
    uint16_t tx = ((addr & 0x7F) << 8) | data;
    uint16_t rx = 0;

    HAL_GPIO_WritePin(DRV_CS_PORT, DRV_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);

    // Ovde koristimo TransmitReceive da bi videli povratni bajt i proverili funkcionalnost linije
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx, (uint8_t*)&rx, 1, HAL_MAX_DELAY);

    HAL_Delay(1);
    HAL_GPIO_WritePin(DRV_CS_PORT, DRV_CS_PIN, GPIO_PIN_SET);

    // Optional: mozes sacuvati rx za debug
    (void)rx;
}
static void Stepper_SpiSelfTest(void)
{
    // Ovaj test izaziva vidljivu SCK aktivnost i potvrđuje da se prenosi
    for (uint8_t i = 0; i < 10; i++) {
        DRV_Write(0x10, 0x12); // bezopasan zapis u test registru
        HAL_Delay(5);
    }
}

static void DRV_Init(void)
{
    HAL_Delay(10);

    DRV_Write(0x10, 0x12);
    HAL_Delay(10);
    DRV_Write(0x0D, 0x05);
    HAL_Delay(10);  
    DRV_Write(0x0E, 0x96);
    HAL_Delay(10);
    DRV_Write(0x0F, 0xA0);
    HAL_Delay(10);
    DRV_Write(0x05, 0x05);
    HAL_Delay(10);
    DRV_Write(0x04, 0x88);
}

static void Stepper_Pulse(void)
{
    HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);
    __NOP(); __NOP(); __NOP();
    HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
}

/* --- API --- */

void Stepper_Init(void)
{
    current_steps = 0;
    target_steps = 0;
    last_step_time = HAL_GetTick();
    busy = false;

    Stepper_SPI_Init();

    HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DRV_CS_PORT, DRV_CS_PIN, GPIO_PIN_SET);

    // Prvi test - vidimo da li SCK blinka po uspešnom SPI inicijaliziju
    Stepper_SpiSelfTest();

    DRV_Init();



}

void Stepper_Home(void)
{
    current_steps = 0;
    target_steps = 0;
    busy = false;
}

float Stepper_GetAngle(void)
{
    return (float)current_steps * STEP_ANGLE;
}

void Stepper_MoveToAngle(float angle_deg)
{
    int32_t steps = (int32_t)(angle_deg / STEP_ANGLE);

    target_steps = steps;

    if (target_steps == current_steps) {
        busy = false;
        return;
    }

    if (target_steps > current_steps) {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
    }

    busy = true;
}

void Stepper_MoveByAngle(float delta_angle_deg)
{
    int32_t delta_steps = (int32_t)(delta_angle_deg / STEP_ANGLE);
    if (delta_steps == 0) return;

    target_steps = current_steps + delta_steps;

    if (delta_steps > 0) {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
    }

    busy = true;
}

bool Stepper_IsBusy(void)
{
    return busy;
}

void Stepper_Update(uint32_t now)
{
    if (!busy) return;

    if ((now - last_step_time) < STEP_INTERVAL_MS)
        return;

    last_step_time = now;

    if (current_steps == target_steps) {
        busy = false;
        return;
    }

    Stepper_Pulse();

    if (target_steps > current_steps)
        current_steps++;
    else
        current_steps--;

    if (current_steps == target_steps)
        busy = false;
}



