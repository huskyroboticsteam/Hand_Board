/* ========================================
 *
 * Simple driver for DRV8872-Q1 H-Bridge bidirectional driver for linear actuator
 * Name two output pins Pin_Actuator_I1 and Pin_Actuator_I2 in top design, for H-bridge In1 and In2
 * Author: Hayden Rundle

 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ActuatorDrive.h"
#include "CAN_Stuff.h"
#include "FSM_Stuff.h"
#include "main.h"





void driveActuator(int16_t pwm) {
    if(abs(pwm) < MIN_PWM) {
        setDriveMode(COAST);
    }
    else if(pwm < 0) {
        setDriveMode(REVERSE);
    }
    else if (pwm > 0) {
        setDriveMode(FORWARD);
    }
}

void StopActuator() {
   setDriveMode(BRAKE);
}


void setDriveMode(driveMode mode) {
    switch(mode){
        case COAST:
            Pin_Actuator_I1_Write(LOW);
            Pin_Actuator_I2_Write(LOW);
            break;
        case REVERSE:
            Pin_Actuator_I1_Write(LOW);
            Pin_Actuator_I2_Write(HIGH);
            break;
        case FORWARD:
            Pin_Actuator_I1_Write(HIGH);
            Pin_Actuator_I2_Write(LOW);
            break;
        case BRAKE:
            Pin_Actuator_I1_Write(HIGH);
            Pin_Actuator_I2_Write(HIGH);
            break;
        default: 
            Pin_Actuator_I1_Write(LOW);
            Pin_Actuator_I2_Write(LOW);
            break;
    }
}

// TODO: Set up error pin reading 



/* [] END OF FILE */
