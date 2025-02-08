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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "project.h"
#include "main.h"
#include "cyapicallbacks.h"
#include "CAN_Stuff.h"
#include "FSM_Stuff.h"
#include "HindsightCAN/CANLibrary.h"
#include "MotorDrive.h"
#include "ActuatorDrive.h"

// LED stuff
volatile uint8_t CAN_time_LED = 0;
volatile uint8_t ERROR_time_LED = 0;

#define SWITCH_DEBOUNCE_UNIT   (1u)

#define SWITCH_DEBOUNCE_PERIOD (10u)

#define SWITCH_RELEASED        (0u)
#define SWITCH_PRESSED         (1u)

// #define TX_DATA_SIZE           (25u)
#define ADC_CHANNEL_NUMBER_0   (0u)


static uint32 ReadSwSwitch(void);

uint8 switchState = SWITCH_RELEASED;

volatile uint8 isrFlag = 0u;

// UART stuff
char txData[TX_DATA_SIZE];

// CAN stuff
CANPacket can_receive;
CANPacket can_send;
uint8 address = 0;
uint16_t id = 0;

const uint8_t* data;

CY_ISR_PROTO(Period_Reset_Handler);
CY_ISR(Period_Reset_Handler) {
    CAN_time_LED++;
    ERROR_time_LED++;

    if (ERROR_time_LED >= 3) {
        LED_ERR_Write(LED_OFF);
    }
    if (CAN_time_LED >= 3) {
        LED_CAN_Write(LED_OFF);
    }
}

CY_ISR(Button_1_Handler) {
    LED_DBG_Write(!LED_DBG_Read());
}

int16_t pwm_set = 0;

int main(void)
{ 
    Initialize();
    int err;
    
    int16 output;
    uint16 resMilliVolts;
    char8 txData[TX_DATA_SIZE];

    DBG_UART_Start();
    ADC_Pot_Start();

    /* Start ADC conversion */
    ADC_Pot_StartConvert();
    
    Print("Start\r\n");
    
    for(;;)
    {   
        err = 0;
        switch(GetState()) {
            case(UNINIT):
                Print("UNINIT: State to CHECK_CAN\r\n");
                SetStateTo(CHECK_CAN);
                StopMotorPWM();
                PWM_Laser1_Sleep();
                PWM_Laser2_Sleep();
                break;
            case(CHECK_CAN):
                if (!PollAndReceiveCANPacket(&can_receive)) {
                    Print("CHECK_CAN: CAN Received\r\n");
                    CAN_time_LED = 0;
                    //LED_CAN_Write(LED_ON);
                    err = ProcessCAN(&can_receive, &can_send);
                    PrintCanPacket(&can_receive);
                }
                
                break;
            case DO_PWM_MODE:
                StartMotorPWM();
                Print("DO_PWM_MODE: Getting PWM from packet\r\n");
                pwm_set = GetPWMFromPacket(&can_receive);
                err = SetMotorPWM(pwm_set/32);
                Print("DO_PWM_MODE: PWM Set\r\n");
                Print("DO_PWM_MODE: State to CHECK_CAN\r\n");
                SetStateTo(CHECK_CAN);
                break;
            case DO_SECONDARY_HAND_MODE:
                PWM_Laser1_Wakeup();
                PWM_Laser2_Wakeup();
                id = GetPeripheralID(&can_receive);
                pwm_set = GetPeripheralData(&can_receive);
                
                if (id == LASER_PERIPH_ID) {
                    Print("DO_SECONDARY_HAND_MODE: Laser PWM Set\r\n");
                    PWM_Laser1_WriteCompare(pwm_set);
                    PWM_Laser1_WriteCompare(pwm_set);
                } else if (id == LINEAR_PERIPH_ID) {
                    Print("DO_SECONDARY_HAND_MODE: Linear Actuator PWM Set\r\n");
                     driveActuator(pwm_set);
                } else {
                    Print("DO_SECONDARY_HAND_MODE: ERROR_INVALID_ID\r\n");
                    err = ERROR_INVALID_ID;   
                }
                Print("DO_SECONDARY_HAND_MODE: State to CHECK_CAN\r\n");
                SetStateTo(CHECK_CAN);
                break;
            default:
                err = ERROR_INVALID_STATE;
                SetStateTo(UNINIT);
                break;
        }    
    
        if (err) DisplayErrorCode(err);
        
        if (DBG_UART_SpiUartGetRxBufferSize()) {
            DebugPrint(DBG_UART_UartGetByte());
        }
    }
}

void Initialize(void) {
    CyGlobalIntEnable; /* Enable global interrupts. LED arrays need this first */
    
    address = getSerialAddress();
    
    DBG_UART_Start();
    sprintf(txData, "Dip Addr: %x \r\n", address);
    Print(txData);
    
    LED_DBG_Write(LED_OFF);
    LED_CAN_Write(LED_OFF);
    LED_ERR_Write(LED_OFF);
    
    InitCAN(0x4, (int)address);
    
    sprintf(txData, "DG: %x\r\nSerialAddress: %d\r\n", 0x4, address);
    Print(txData);
    
    PWM_Laser1_Init();
    PWM_Laser1_Start();
    PWM_Laser2_Init();
    PWM_Laser2_Start();
    PWM_Motor_Init();
    PWM_Motor_Start();
    
    Timer_Period_Reset_Start();
     
    isr_Period_Reset_StartEx(Period_Reset_Handler);
    isr_Drive_StartEx(Drive_Handler);
}

void DebugPrint(char input) {
    switch(input) {
        case 'f':
            sprintf(txData, "Mode: %x State:%x \r\n", GetMode(), GetState());
            break;
        case 'x':
            sprintf(txData, "bruh\r\n");
            break;
        default:
            sprintf(txData, "what\r\n");
            break;
    }
    Print(txData);
}

int getSerialAddress() {
    int address = 0;
    
    if (address == 0)
        address = DEVICE_SERIAL_MOTOR_HAND;

    return address;
}

void DisplayErrorCode(uint8_t code) {    
    ERROR_time_LED = 0;
    LED_ERR_Write(LED_ON);
    
    sprintf(txData, "Error %X\r\n", code);
    Print(txData);

    switch(code)
    {
        case ERROR_INVALID_TTC:
            Print("Cannot Send That Data Type!\n\r");
            break;
        default:
            //some error
            break;
    }
}

/* [] END OF FILE */
