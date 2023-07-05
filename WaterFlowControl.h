#ifndef WATER_FLOW_CONTROL_H
#define WATER_FLOW_CONTROL_H

#include "stdint.h"
#include "Hardware.h"
#include "TaskManager.h"

void init_water_flow_control(void);

void check_water_flow(void);

float get_water_flow(void);

uint32_t get_air_flow_disable_delay(void);

void water_flow_senor1_interrupt_handler(void);

void task_manager_interrupt_handler(void);

#endif
