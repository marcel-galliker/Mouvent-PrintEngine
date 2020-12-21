#include "sa_tcp_ip.h"
#include "comm.h"
#include "error.h"
#include "ethernet.h"
#include "power_step_def.h"
#include "densio_def.h"
#include "densio.h"
#include "power_step.h"
#include "atmel_start_pins.h"

#include <string.h>

#define MAX_ANSWER_MESSAGE_SIZE			(256)
#define MAX_ANSWER_MESSAGE_BUFFER_SIZE	(3 * MAX_ANSWER_MESSAGE_SIZE)

// Prototypes
static INT32 handle_status_commands(UINT8* message);
static INT32 handle_motor_commands(UINT8* message);
static INT32 handle_densio_commands(UINT8* message);

INT32 handle_command_message(UINT8* message)
{	
	SMsgHdr* messageHeader = (SMsgHdr*)message;
	
	switch(messageHeader->msgId & COMMAND_TYPE_MASK)
	{
		case STATUS_COMMAND_MASK:
		handle_status_commands(message);
		break;
		
		case MOTOR_COMMAND_MASK:
		handle_motor_commands(message);
		break;
		
		case DENSIO_COMMAND_MASK:
		handle_densio_commands(message);
		break;
		
		default:
		// Handle unknown command type error
		break;
	}
	
	return REPLY_OK;
}

INT32 send_answer_message(UINT32 length, UINT8* data)
{
	static UINT32 bytesWritten;
	static UINT8 outputBuffer[MAX_ANSWER_MESSAGE_BUFFER_SIZE];
	
	if(length > MAX_ANSWER_MESSAGE_SIZE)
		return REPLY_ERROR;
		
	comm_encode(data,
				length,
				outputBuffer,
				sizeof(outputBuffer),
				&bytesWritten);
				
	ethernet_send(outputBuffer, bytesWritten);
	
	return REPLY_OK;
}

static INT32 handle_status_commands(UINT8* message)
{
	SStatusMessage statusMessage;
	SMsgHdr* messageHeader = (SMsgHdr*)message;
	
	SMotorStatus *motorStatus = ps_get_motor_status();
	//SDensioStatus *densioStatus = densio_get_densio_status();
	
	
	switch(messageHeader->msgId)
	{
		case CMD_STATUS_GET:
		statusMessage.header.msgId = messageHeader->msgId;
		statusMessage.header.msgId = messageHeader->msgLen = sizeof(statusMessage);
		
		statusMessage.powerStepStatus = motorStatus->motorStatus;
		statusMessage.motorPosition = motorStatus->motorPosition;
		statusMessage.motorVoltage = motorStatus->motorVoltage;
		statusMessage.motorMoveCurrent = motorStatus->motorMoveCurrent;
		statusMessage.motorHoldCurrent = motorStatus->motorHoldCurrent;
		statusMessage.refDone = motorStatus->refDone;
		statusMessage.moving = motorStatus->moving;
		
		statusMessage.inputs = (gpio_get_pin_level(Input2) << IN_STOP_LEFT) |
								(gpio_get_pin_level(Input1) << IN_STOP_RIGHT);
		
		send_answer_message(sizeof(SStatusMessage), (UINT8 *)&statusMessage);
		break;
		
		default:
		// Handle unknown command
		break;
		
	}
	
	return REPLY_OK;
}

static INT32 handle_motor_commands(UINT8* message)
{
	SMsgHdr* messageHeader = (SMsgHdr*)message;
	
	switch(messageHeader->msgId)
	{
		case CMD_MOTOR_REFERENCE:
		move_reference();
		break;
				
		case CMD_MOTOR_MOVE:
		if(messageHeader->msgLen != sizeof(SMotorMoveCommand))
			return REPLY_ERROR;
			
		SMotorMoveCommand* motorMoveCommand = (SMotorMoveCommand*)message;
		move_relative(motorMoveCommand);
		break;
		
		case CMD_MOTOR_STOP:
		stop_motor();
		break;
		
		default:
		// Handle unknown command
		break;
	}
	
	return REPLY_OK;
}

static INT32 handle_densio_commands(UINT8* message)
{
	SMsgHdr* messageHeader = (SMsgHdr*)message;
	
	switch(messageHeader->msgId)
	{
		case CMD_OUT_TRIGGER:
		if(messageHeader->msgLen != sizeof(SOutTriggerCmd))
			return REPLY_ERROR;
		
		SOutTriggerCmd* triggerCommand = (SOutTriggerCmd*)message;
		densio_trigger(triggerCommand);
		break;
		
		default:
		// Handle unknown command
		break;
	}
	
	return REPLY_OK;
}