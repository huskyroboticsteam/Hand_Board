/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#pragma once

#include <project.h>
#include <stdint.h>

#define FORWARD 1
#define BACKWARD 0

#define ERROR_PWM_NOT_ENABLED 0x30
#define ERROR_LIMIT 0x31

typedef struct conversion {
    uint8 min_set, max_set, ratio_set;
    int32 tickMin, tickMax;
    int32 mDegMin, mDegMax;
    double ratio;
} Conversion;

int StartMotorPWM();
void StopMotorPWM();
int SetMotorPWM(int16 pwm);
Conversion GetConversion();
uint8 GetConversionReady();
void SetConvRatio(float ratio);
int SetConvMin(int32 tickMin, int32 mDegMin);
void SetConvMax(int32 tickMax, int32 mDegMax);
void SetEncOffset(int32 tick_offset);
void SetEncDir(uint8 dir);
void SetUsingPot(uint8_t pot);
int32 GetPotValue();
int32 GetEncValue();
int32 GetPosition();
uint8 GetLimitStatus();
int32 GetCurrentMotorPWM();
void SetEncBound(uint8 lim_num, int32 value);

CY_ISR(Limit_Handler);
CY_ISR(Drive_Handler);

/* [] END OF FILE */
