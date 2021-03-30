#ifndef INCLUDES_STATUS_MANAGER_H_
#define INCLUDES_STATUS_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

bool status_manager_start(void);
bool status_manager_is_initalized(void);
void status_manager_handle_message(void* message);
void status_manager_send_status(void);

#endif /* INCLUDES_STATUS_MANAGER_H_ */
