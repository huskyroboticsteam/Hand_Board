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

// BLDC

#include <project.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "MotorDrive.h"
#include "CAN_Stuff.h"

int16 min_pwm = 20;
int32 PWM_value = 0;

volatile uint8  PWM_invalidate = 0;

Conversion conv = {};

uint8 limit1 = 0;
uint8 limit2 = 0;
uint8 bound_set1;
uint8 bound_set2;

int StartMotorPWM() {
    return 0;
}

void StopMotorPWM() {
    PWM_Motor_WriteCompare(0);
}

// Sends PWM and Direction to the motor driver
// Also checks limits and sets PWM variables
// Accepts values -1024 - 1024
// BLDC Edition!!!
int SetMotorPWM(int16 pwm) {
    int err = 0;
    PWM_invalidate = 0;
    
    if (pwm < 0) {
        if (limit1) {
            err = ERROR_LIMIT;
            pwm = 0;
        }
    } else if (pwm > 0) {
        if (limit2) {
            err = ERROR_LIMIT;
            pwm = 0;
        }
    }
    
    if (abs(pwm) < min_pwm) pwm = 0;
    
    PWM_value = pwm;
    PWM_Motor_WriteCompare(pwm/0.1024+30000);
    
    return err;
}

int UpdateConversion() {
    if (conv.min_set && conv.max_set && conv.mDegMax != conv.mDegMin) {
        conv.ratio = (double) (conv.tickMax-conv.tickMin)/(conv.mDegMax-conv.mDegMin);
        conv.ratio_set = 1;
    } else {
        return 1;
    }
    
    return 0;
}

Conversion GetConversion() { return conv; }

void SetConvRatio(float ratio) {
    conv.ratio = ratio;
    conv.ratio_set = 1;
}

int SetConvMin(int32 tickMin, int32 mDegMin) {
    conv.tickMin = tickMin;
    conv.mDegMin = mDegMin;
    conv.min_set = 1;
    
    return UpdateConversion();
}

void SetConvMax(int32 tickMax, int32 mDegMax) {
    conv.tickMax = tickMax;
    conv.mDegMax = mDegMax;
    conv.max_set = 1;
    UpdateConversion();
}

int32 GetCurrentPWM() { return PWM_value; }
uint8 GetLimitStatus() { return limit1 | (limit2 << 1); }  

CY_ISR(Drive_Handler) {
    if (Limit_1_Read() == 0) {
        if (limit1 == 0) {
            SetMotorPWM(0);
            SendLimitAlert(1);
        }
        limit1 = 1;
    } else limit1 = 0;
    
    if (Limit_2_Read() == 0) {
        if (limit2 == 0) {
            SetMotorPWM(0);
            SendLimitAlert(2);
        }
        limit2 = 1;
    } else limit2 = 0;
    
    if (PWM_invalidate == 20) SetMotorPWM(0);
    else PWM_invalidate++;
}

/* [] END OF FILE */
