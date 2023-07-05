#ifndef HARDWARE_H
#define HARDWARE_H

#include "stdint.h"
#include "Arduino.h"
#include <GyverTimers.h>

#define ADC_RANGE_MIN 0
#define ADC_RANGE_MAX 1023

#define AIR_FLOW_DISABLE_REGULATION_ADC_CHANNEL A0

uint8_t get_water_flow_senor2_state(void);

uint16_t get_adc_value(uint8_t adc_channel);

void enable_interrupts(void);
void disable_interrupts(void);

void init_water_flow_senor1_interrupt(void (* water_flow_senor1_interrupt_handler)(void));

void init_gpio(void);

void init_task_manager_timer(void (* task_manager_timer_interrupt_handler)(void));

void enable_water_flow_senor1_led(void);
void disable_water_flow_senor1_led(void);
void toggle_water_flow_senor1_led(void);

void enable_water_flow_senor2_valve(void);
void disable_water_flow_senor2_valve(void);

void enable_water_flow_senor2_led(void);
void disable_water_flow_senor2_led(void);

void enable_air_flow(void);
void disable_air_flow(void);

void enable_air_flow_led(void);
void disable_air_flow_led(void);
void toggle_air_flow_led(void);

#endif
