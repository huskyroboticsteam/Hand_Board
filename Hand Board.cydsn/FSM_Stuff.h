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
#include <stdint.h>

//States in FSM
#define UNINIT          0x0
#define CHECK_CAN       0x1
#define DO_LASER_MODE   0x2
#define DO_LINEAR_MODE  0x3

//Operation modes
#define LASER_MODE      0x4
#define LINEAR_MODE     0x5

// FSM Errors (0x20-0x2F)
#define ERROR_ESTOP         0x20
#define ERROR_INVALID_STATE 0x21

void GotoUninitState();
void SetStateTo(uint8_t state);
void SetModeTo(uint8_t mode);
uint8_t GetState();
uint8_t GetMode();

/* [] END OF FILE */