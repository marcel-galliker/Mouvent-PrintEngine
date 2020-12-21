#ifndef DENSIO_H_
#define DENSIO_H_

#include <type_defs.h>
#include "sa_tcp_ip.h"

#include "densio_def.h"

void densio_tick(void);
SDensioStatus* densio_get_densio_status(void);
void densio_trigger(SetupAssist_OutTriggerCmd* triggerCommand);

#endif /* DENSIO_H_ */