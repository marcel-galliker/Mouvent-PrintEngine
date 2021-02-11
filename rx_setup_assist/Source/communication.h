#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

INT32 handle_command_message(UINT8* message);
INT32 send_answer_message(UINT32 length, UINT8* data);

INT32 comm_send_status(void);

#endif /* COMMUNICATION_H_ */