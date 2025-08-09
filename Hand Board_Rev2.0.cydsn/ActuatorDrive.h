/* ========================================
 *
 * Simpe driver for DRV8872-Q1 H-Bridge bidirectional driver for linear actuator
 * Name two output pins Pin_Actuator_I1 and Pin_Actuator_I2 in top design, for H-bridge In1 and In2
 * Author: Hayden Rundle

 * ========================================
*/
#pragma once

#include <project.h>
#include <stdint.h>

#define HIGH 1
#define LOW 0
#define MIN_PWM 20

typedef enum driveMode {
    COAST,
    REVERSE,
    FORWARDS,
    BRAKE
    
} driveMode;

void driveActuator(int16 pwm);
void stopActuator();
void setDriveMode(driveMode mode);


/* [] END OF FILE */
