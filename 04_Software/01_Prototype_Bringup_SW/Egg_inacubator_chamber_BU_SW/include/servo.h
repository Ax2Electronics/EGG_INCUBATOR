#ifndef SERVO_H
#define SERVO_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ----------------------------------------------------------
 *  Servo pulse width granice (u mikrosekundama = TIM tikovima)
 * ---------------------------------------------------------- */
#define SERVO_MIN_US    500u    /*   0° */
#define SERVO_MID_US   1500u    /*  90° — tray horizontalno */
#define SERVO_MAX_US   2500u    /* 180° */

/* ----------------------------------------------------------
 *  Ugaone pozicije traya
 * ---------------------------------------------------------- */
#define TRAY_LEFT    45u    /* Tray -45° od horizontale */
#define TRAY_CENTER  90u    /* Tray horizontalno        */
#define TRAY_RIGHT  135u    /* Tray +45° od horizontale */

/* ----------------------------------------------------------
 *  Brzina okretanja
 *  400ms po stepenu → 90 stepeni × 400ms ≈ 36 sekundi
 * ---------------------------------------------------------- */
#define SERVO_STEP_DELAY_MS  200u
#define SERVO_STEP_DDEG 5u

/* ----------------------------------------------------------
 *  Public API
 * ---------------------------------------------------------- */

/* Inicijalizacija — pozovi jednom u main() */
void    Servo_Init(void);

/* Direktno postavljanje ugla — trenutno, bez rampe */
void    Servo_SetAngle(uint8_t deg);

/* Trenutni ugao */
uint8_t Servo_GetAngle(void);

/* Non-blocking pokretanje ka ciljnom uglu */
void    Servo_StartMoveTo(uint8_t targetDeg);

/* Non-blocking update — pozivaj svaki ciklus main loop-a */
/* Vraca 1 kada je kretanje zavrseno, 0 dok je u toku     */
uint8_t Servo_Update(uint32_t now);

/* Provera da li je servo trenutno u pokretu */
uint8_t Servo_IsBusy(void);

/* Precice za tray pozicije */
void    Tray_TiltLeft(void);
void    Tray_TiltRight(void);
void    Tray_Center(void);

/* Demo sekvenca za testiranje — blokira main loop */
void    Servo_DemoSequence(void);

#endif /* SERVO_H */