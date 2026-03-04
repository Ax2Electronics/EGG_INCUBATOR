/* ============================================================
 *  Incubator Project — Servo Module
 *  MCU    : STM32F401RE
 *  Timer  : TIM3 CH1 → PA6
 *  Servo  : DS3235
 *
 *  Interni ugao se cuva u desetinkama stepena (ddeg)
 *  0.1° rezolucija, public API koristi cele stepene
 *
 *  500us  =    0° =    0 ddeg
 *  1500us =   90° =  900 ddeg
 *  2500us =  180° = 1800 ddeg
 * ============================================================ */

#include "servo.h"
#include "tim.h"

/* ----------------------------------------------------------
 *  Interne promenljive
 *  Sve u desetinkama stepena (ddeg)
 *  Primer: 90.5° = 905 ddeg
 * ---------------------------------------------------------- */
static uint16_t s_currentDdeg = 900u;   /* Pocetna pozicija 90.0° */
static uint16_t s_targetDdeg  = 900u;
static uint32_t s_lastStepMs  = 0u;
static uint8_t  s_moving      = 0u;

/* ----------------------------------------------------------
 *  Interne pomocne funkcije
 * ---------------------------------------------------------- */

/**
 * @brief  Konvertuje desetine stepena u PWM pulse width (us)
 *
 *  Linearno mapiranje:
 *    0 ddeg (0°)   → 500us
 *  1800 ddeg (180°)→ 2500us
 *
 *  Formula: us = 500 + (ddeg * 2000) / 1800
 */
static uint32_t ddegToUs(uint16_t ddeg) {
    if (ddeg > 1800u) ddeg = 1800u;
    return SERVO_MIN_US + ((uint32_t)ddeg * 2000u) / 1800u;
}

/**
 * @brief  Postavi servo na poziciju u desetinkama stepena
 * @param  ddeg  Ugao u ddeg (0–1800)
 */
static void Servo_SetDdeg(uint16_t ddeg) {
    if (ddeg > 1800u) ddeg = 1800u;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ddegToUs(ddeg));
    s_currentDdeg = ddeg;
}

/* ----------------------------------------------------------
 *  Public API
 * ---------------------------------------------------------- */

/**
 * @brief  Inicijalizuj servo — postavi na centar i sacekaj
 */
void Servo_Init(void) {
    Servo_SetDdeg(900u);   /* 90.0° — tray horizontalno */
    HAL_Delay(500u);
}

/**
 * @brief  Direktno postavi servo na zadani ugao (bez rampe)
 * @param  deg  Ugao u stepenima (0–180)
 */
void Servo_SetAngle(uint8_t deg) {
    if (deg > 180u) deg = 180u;
    Servo_SetDdeg((uint16_t)deg * 10u);
}

/**
 * @brief  Vrati trenutni ugao serva u stepenima
 * @retval Ugao (0–180)
 */
uint8_t Servo_GetAngle(void) {
    return (uint8_t)(s_currentDdeg / 10u);
}

/**
 * @brief  Proveri da li je servo trenutno u pokretu
 * @retval 1 = u pokretu, 0 = slobodan
 */
uint8_t Servo_IsBusy(void) {
    return s_moving;
}

/**
 * @brief  Pokreni non-blocking kretanje ka ciljnom uglu
 * @param  targetDeg  Ciljni ugao u stepenima (0–180)
 */
void Servo_StartMoveTo(uint8_t targetDeg) {
    if (targetDeg > 180u) targetDeg = 180u;

    uint16_t targetDdeg = (uint16_t)targetDeg * 10u;

    /* Vec na ciljanoj poziciji — nema potrebe za kretanjem */
    if (targetDdeg == s_currentDdeg) return;

    s_targetDdeg = targetDdeg;
    s_moving     = 1u;
    s_lastStepMs = HAL_GetTick();
}

/**
 * @brief  Non-blocking servo update
 *         Pozivaj svaki ciklus main loop-a
 *
 *         Svaki poziv pomera servo za SERVO_STEP_DDEG
 *         samo ako je proslo SERVO_STEP_DELAY_MS od zadnjeg koraka.
 *         Ostatak vremena funkcija izlazi odmah bez blokiranja.
 *
 * @param  now  Trenutni HAL_GetTick() timestamp
 * @retval 1 = kretanje zavrseno ili servo miran
 *         0 = kretanje u toku
 */
uint8_t Servo_Update(uint32_t now) {

    /* Servo miran — izadi odmah */
    if (!s_moving) return 1u;

    /* Jos nije vreme za sledeci korak */
    if ((now - s_lastStepMs) < SERVO_STEP_DELAY_MS) return 0u;
    s_lastStepMs = now;

    /* Pomeri jedan korak ka cilju */
    if (s_currentDdeg < s_targetDdeg) {
        /* Kretanje prema vecim uglovima */
        uint16_t next = s_currentDdeg + SERVO_STEP_DDEG;
        Servo_SetDdeg(next > s_targetDdeg ? s_targetDdeg : next);

    } else if (s_currentDdeg > s_targetDdeg) {
        /* Kretanje prema manjim uglovima — zastita od uint underflow */
        if (s_currentDdeg < SERVO_STEP_DDEG) {
            Servo_SetDdeg(0u);
        } else {
            uint16_t next = s_currentDdeg - SERVO_STEP_DDEG;
            Servo_SetDdeg(next < s_targetDdeg ? s_targetDdeg : next);
        }
    }

    /* Proveri da li smo stigli na cilj */
    if (s_currentDdeg == s_targetDdeg) {
        s_moving = 0u;
        return 1u;
    }

    return 0u;
}

/* ----------------------------------------------------------
 *  Precice za tray pozicije
 * ---------------------------------------------------------- */

/** @brief  Nakloni tray levo (-45° od horizontale) */
void Tray_TiltLeft(void) {
    Servo_StartMoveTo(TRAY_LEFT);
}

/** @brief  Nakloni tray desno (+45° od horizontale) */
void Tray_TiltRight(void) {
    Servo_StartMoveTo(TRAY_RIGHT);
}

/** @brief  Vrati tray u horizontalni polozaj */
void Tray_Center(void) {
    Servo_StartMoveTo(TRAY_CENTER);
}

/* ----------------------------------------------------------
 *  Demo sekvenca — SAMO ZA TESTIRANJE
 *  Blokira main loop — ukloni u produkciji
 * ---------------------------------------------------------- */
void Servo_DemoSequence(void) {

    /* Centar */
    Servo_SetAngle(TRAY_CENTER);
    HAL_Delay(2000u);

    /* Polako levo */
    for (int16_t d = (int16_t)(TRAY_CENTER * 10);
                 d >= (int16_t)(TRAY_LEFT * 10);
                 d -= (int16_t)SERVO_STEP_DDEG) {
        Servo_SetDdeg((uint16_t)d);
        HAL_Delay(SERVO_STEP_DELAY_MS);
    }
    HAL_Delay(2000u);

    /* Polako nazad na centar */
    for (int16_t d = (int16_t)(TRAY_LEFT * 10);
                 d <= (int16_t)(TRAY_CENTER * 10);
                 d += (int16_t)SERVO_STEP_DDEG) {
        Servo_SetDdeg((uint16_t)d);
        HAL_Delay(SERVO_STEP_DELAY_MS);
    }
    HAL_Delay(2000u);

    /* Polako desno */
    for (int16_t d = (int16_t)(TRAY_CENTER * 10);
                 d <= (int16_t)(TRAY_RIGHT * 10);
                 d += (int16_t)SERVO_STEP_DDEG) {
        Servo_SetDdeg((uint16_t)d);
        HAL_Delay(SERVO_STEP_DELAY_MS);
    }
    HAL_Delay(2000u);

    /* Polako nazad na centar */
    for (int16_t d = (int16_t)(TRAY_RIGHT * 10);
                 d >= (int16_t)(TRAY_CENTER * 10);
                 d -= (int16_t)SERVO_STEP_DDEG) {
        Servo_SetDdeg((uint16_t)d);
        HAL_Delay(SERVO_STEP_DELAY_MS);
    }
    HAL_Delay(1000u);
}