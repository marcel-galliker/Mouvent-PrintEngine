
#include <stdio.h>
#include <stdarg.h>
#include "rx_robot_tcpip.h"
#include "rx_trace.h"
#include <network.h>

static int	_Enabled = 0;

//--- TrEnable ----------------------------------------
void TrEnable(int enable)
{
	_Enabled = enable;
}

//--- TrPrintf --------------------------------------------
void TrPrintf(int trace, const char *format, ...)
{
	va_list ap;
	int len=0;

	if (_Enabled && trace)
	{
		SRobotTraceMsg msg;
		msg.header.msgId  = CMD_TRACE;

//		len = sprintf(msg.message, "%d.%03d: ", now/1000, now%1000);
		va_start(ap, format);
		len += vsnprintf(msg.message+len, sizeof(msg.message)-len-1, format, ap);
		va_end(ap);
		msg.header.msgLen = sizeof(msg.header)+len+1;
		network_send_ctrl_msg(&msg, msg.header.msgLen);
	}
} // end TrPrintf
