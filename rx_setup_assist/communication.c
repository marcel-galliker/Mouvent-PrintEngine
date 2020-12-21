#include <type_defs.h>
#include <stdio.h>
#include "comm.h"
#include "communication.h"
#include "error.h"
#include "ethernet.h"
#include "power_step_def.h"
#include "densio_def.h"
#include "densio.h"
#include "power_step.h"
#include "atmel_start_pins.h"
#include "sa_tcp_ip.h"

#include <string.h>

#define MAX_ANSWER_MESSAGE_SIZE			(256)
#define MAX_ANSWER_MESSAGE_BUFFER_SIZE	(3 * MAX_ANSWER_MESSAGE_SIZE)

// Prototypes
static INT32 handle_status_commands(UINT8* message);
static INT32 handle_motor_commands(UINT8* message);
static INT32 handle_densio_commands(UINT8* message);

INT32 handle_command_message(UINT8* message)
{	
	SSAMsgHdr* messageHeader = (SSAMsgHdr*)message;
	
	switch(messageHeader->msgId & COMMAND_TYPE_MASK)
	{
		case STATUS_COMMAND_MASK:	handle_status_commands(message); break;
		
		case MOTOR_COMMAND_MASK:	handle_motor_commands(message);	 break;
		
		case DENSIO_COMMAND_MASK:	handle_densio_commands(message); break;
		
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

//--- handle_status_commands ---------------------------------------
INT32 handle_status_commands(UINT8* message)
{
	SSAMsgHdr* messageHeader = (SSAMsgHdr*)message;
	
	switch(messageHeader->msgId)
	{
		case CMD_STATUS_GET: comm_send_status(); break;
		default: break;
	}
	return REPLY_OK;
}

//--- comm_send_status -------------------------------------
INT32 comm_send_status(void)
{
	SSetupAssist_StatusMsg msg;	
	
	msg.hdr.msgLen	= sizeof(msg);
	msg.hdr.msgId	= CMD_STATUS_GET;
		
	memcpy(&msg.motor, ps_get_motor_status(), sizeof(SMotorStatus));
	msg.inputs			= (gpio_get_pin_level(Input2) << IN_STOP_LEFT) 
									| (gpio_get_pin_level(Input1) << IN_STOP_RIGHT);
		
	//	send_answer_message(sizeof(statusMessage), (UINT8 *)&statusMessage);
	ethernet_send(&msg, sizeof(msg));
	
	return REPLY_OK;
}

static INT32 handle_motor_commands(UINT8* message)
{
	SSAMsgHdr* messageHeader = (SSAMsgHdr*)message;
	
	switch(messageHeader->msgId)
	{
		case CMD_MOTOR_REFERENCE:
		move_reference();
		break;
				
		case CMD_MOTOR_MOVE:
		if(messageHeader->msgLen != sizeof(SetupAssist_MoveCmd))
			return REPLY_ERROR;
			
		SetupAssist_MoveCmd* motorMoveCommand = (SetupAssist_MoveCmd*)message;
		move_absolute(motorMoveCommand, 2);
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
	SSAMsgHdr* messageHeader = (SSAMsgHdr*)message;
	
	switch(messageHeader->msgId)
	{
		case CMD_OUT_TRIGGER:
		if(messageHeader->msgLen != sizeof(SetupAssist_OutTriggerCmd))
			return REPLY_ERROR;
		
		SetupAssist_OutTriggerCmd* triggerCommand = (SetupAssist_OutTriggerCmd*)message;
		densio_trigger(triggerCommand);
		break;
		
		default:
		// Handle unknown command
		break;
	}
	
	return REPLY_OK;
}