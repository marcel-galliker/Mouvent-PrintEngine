#pragma once
#include "rx_def.h"

void iq500_init();
void iq500_end();

void iq500_start_printing();
void iq500_stop_printing();

void iq500_abort(); // to be called when printing is aborted
void iq500_new_job(); // to be called from rx_print_ctrl when a new job is started (loading, buffering)
void iq500_print_done(SPrintQueueItem* item); // to be called from_rx_print_ctrl after a page is printed
void iq500_job_done(); // to be called from_rx_print_ctrl when it detects a job end
