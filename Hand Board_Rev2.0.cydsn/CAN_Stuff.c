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

#include <stdio.h>
#include "main.h"
#include "CAN_Stuff.h"
#include "FSM_Stuff.h"
#include "HindsightCAN/CANLibrary.h"

extern char txData[TX_DATA_SIZE];
extern uint8 address;

//Reads from CAN FIFO and changes the state and mode accordingly
int ProcessCAN(CANPacket* receivedPacket, CANPacket* packetToSend) {
    uint16_t packageID = GetPacketID(receivedPacket);
    uint8_t sender_DG = GetSenderDeviceGroupCode(receivedPacket);
    uint8_t sender_SN = GetSenderDeviceSerialNumber(receivedPacket);
    int32_t data = 0xFF;
    int err = 0;
    
    
    switch(packageID){
        // Board-specific packets
        case(ID_MOTOR_UNIT_MODE_SEL):
            Print("Package ID: ID_MOTOR_UNIT_MODE_SEL\r\n");
            Print("Modes not supported in PY2024 Hand Board\r\n");
            break;
            
        case(ID_MOTOR_UNIT_SET_PERIPHERALS):
            Print("Package ID: ID_MOTOR_UNIT_SET_PERIPHERALS\r\n");
            SetStateTo(DO_SECONDARY_HAND_MODE);
            break;
            
        case(ID_MOTOR_UNIT_PWM_DIR_SET):
            Print("Package ID: ID_MOTOR_UNIT_PWM_DIR_SET\r\n");
            SetStateTo(DO_PWM_MODE);
            break;
            
        // Common Packets
        case(ID_ESTOP):
            Print("\r\n\r\nSTOP\r\n\r\n");
            // stop all movement
            GotoUninitState();
            err = ESTOP_ERR_GENERAL;
            break;
        
        case(ID_TELEMETRY_PULL):            
            switch(DecodeTelemetryType(receivedPacket))
            {
                // USE CONSTANTS FOR CASES
                case(0):
                    data = 105;
                    break;
                default:
                    err = ERROR_INVALID_TTC;
                    break;
            }
            
            // Assemble and send packet
            AssembleTelemetryReportPacket(packetToSend, sender_DG, sender_SN, receivedPacket->data[3], data);
            
            if (err == 0)
                SendCANPacket(packetToSend);
            
            break;
            
        default: //recieved Packet with non-valid ID
            // could be due to corruption, don't uninit
            return ERROR_INVALID_PACKET;
    }
    
    return err;
}

void PrintCanPacket(CANPacket* packet){
    sprintf(txData, "ID %X DLC %X DATA", packet->id, packet->dlc);
    Print(txData);
    for(int i = 0; i < packet->dlc; i++ ) {
        sprintf(txData," %02X", packet->data[i]);
        Print(txData);
    }
    Print("\r\n");
}

int SendLimitAlert(uint8 status) {
    CANPacket can_send;
    AssembleLimitSwitchAlertPacket(&can_send, DEVICE_GROUP_JETSON, 
    DEVICE_SERIAL_JETSON, status);
    return SendCANPacket(&can_send);
}



/* [] END OF FILE */