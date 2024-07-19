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
//CY_ISR_PROTO(ISR_CAN);

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

uint32_t pwm_set = 0;
uint16_t on_time = 0;
uint16_t off_time = 0;

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

    //CAN_Start();

    /* Set CAN interrupt handler to local routine */
    // CyIntSetVector(CAN_ISR_NUMBER, ISR_CAN);

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    Print("Start\r\n");
    
    for(;;)
    {   
        err = 0;
        switch(GetState()) {
            case(UNINIT):
                Print("UNINIT: State to CHECK_CAN\r\n");
                SetStateTo(CHECK_CAN);
                break;
            case(CHECK_CAN):
                if (!PollAndReceiveCANPacket(&can_receive)) {
                    Print("CHECK_CAN: CAN Received\r\n");
                    CAN_time_LED = 0;
                    err = ProcessCAN(&can_receive, &can_send);
                    PrintInt(err);
                }
                
                break;
            case DO_PWM_MODE:
                Print("DO_PWM_MODE: Getting PWM from packet\r\n");
                pwm_set = GetPWMFromPacket(&can_receive);
                SetMotorPWM(pwm_set);
                Print("DO_PWM_MODE: PWM Set\r\n");
                Print("DO_PWM_MODE: State to CHECK_CAN\r\n");
                SetStateTo(CHECK_CAN);
                break;
            case DO_SECONDARY_MODE:
                // mode 1 tasks
                id = GetPCAID(&can_receive);
                //on_time = GetUIntPCAOnTimeFromPacket(&can_receive);
                //off_time = GetUIntPCAOffTimeFromPacket(&can_receive);
                on_time = (can_receive.data[2] << 8) | can_receive.data[3];
                off_time = (can_receive.data[4] << 8) | can_receive.data[5];
                pwm_set = off_time - on_time;
                Print("\r\n");
                PrintInt(on_time);
                Print("\r\n");
                PrintInt(off_time);
                Print("\r\n");
                PrintInt(pwm_set);
                Print("\r\n");
                
                if (id == LASER_PCA_ID) {
                    Print("DO_SECONDARY_MODE: Laser PWM Set\r\n");
                    PWM_Laser_WriteCompare(pwm_set);
                } else if (id == LINEAR_PCA_ID) {
                    Print("DO_SECONDARY_MODE: Linear Actuator PWM Set\r\n");
                    if (pwm_set > PWM_MAX / 2)
                        PWM_Actuator_WriteCompare(PWM_MAX);
                    else
                        PWM_Actuator_WriteCompare(0);
                } else {
                    Print("DO_SECONDARY_MODE: ERROR_INVALID_ID\r\n");
                    err = ERROR_INVALID_ID;   
                }
                Print("DO_SECONDARY_MODE: State to CHECK_CAN\r\n");
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
    Timer_Period_Reset_Start();
    
    sprintf(txData, "DG: %x\r\nSerialAddress: %d\r\n", 0x4, address);
    Print(txData);
    
    PWM_Laser_Init();
    PWM_Laser_Start();
    PWM_Actuator_Init();
    PWM_Actuator_Start();
    StartMotorPWM();
    
    isr_Period_Reset_StartEx(Period_Reset_Handler);
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

/*
static uint32 ReadSwSwitch(void)
{
    uint32 heldDown;
    uint32 sw1Status;
    uint32 sw2Status;

    sw1Status = 0u;  /* Switch is not active 
    sw2Status = 0u;  /* Switch is not active 
    heldDown = 0u;  /* Reset debounce counter 

    /* Wait for debounce period before determining whether the switch is pressed 
    while (Switch1_Read() == SWITCH_PRESSED)
    {
        /* Count debounce period 
        CyDelay(SWITCH_DEBOUNCE_UNIT);
        ++heldDown;

        if (heldDown > SWITCH_DEBOUNCE_PERIOD)
        {
            sw1Status = 1u; /* Switch is pressed 
            break;
        }
    }
    
    while (Switch2_Read() == SWITCH_PRESSED)
    {
        /* Count debounce period
        CyDelay(SWITCH_DEBOUNCE_UNIT);
        ++heldDown;

        if (heldDown > SWITCH_DEBOUNCE_PERIOD)
        {
            sw2Status = 1u; /* Switch is pressed
            break;
        }
    }

    return (sw1Status);
    return (sw2Status);
}
*/



/* [] END OF FILE */
