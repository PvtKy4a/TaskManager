#include "Hardware.h"

#define WATER_FLOW_SENSOR1      2
#define WATER_FLOW_SENSOR1_LED  3

#define WATER_FLOW_SENSOR2_VALVE 4

#define WATER_FLOW_SENSOR2      5
#define WATER_FLOW_SENSOR2_LED  6

#define AIR_FLOW      7
#define AIR_FLOW_LED  8

#define ENABLE_LED    0
#define DISABLE_LED   1

#define ENABLE_AIR_FLOW   1
#define DISABLE_AIR_FLOW  0

#define ENABLE_WATER_FLOW_SENSOR_2_VALVE  1
#define DISABLE_WATER_FLOW_SENSOR_2_VALVE 0

#define TASK_MANAGER_TIMER  TIMER1_A

void (* task_manager_interrupt_handler)(void);

uint16_t get_adc_value(uint8_t adc_channel) {
  uint16_t adc_value = analogRead(adc_channel);
  return adc_value;
}

uint8_t get_water_flow_senor2_state(void) {
  uint8_t state = digitalRead(WATER_FLOW_SENSOR2);
  return state;
}

void enable_interrupts(void) {
  interrupts();
}

void disable_interrupts(void) {
  noInterrupts();
}

void init_water_flow_senor1_interrupt(void (* water_flow_senor1_interrupt_handler)(void)) {
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR1), water_flow_senor1_interrupt_handler, RISING);
}

void init_gpio(void) {
  pinMode(WATER_FLOW_SENSOR1, INPUT);
  pinMode(WATER_FLOW_SENSOR2, INPUT);
  pinMode(WATER_FLOW_SENSOR1_LED, OUTPUT);
  digitalWrite(WATER_FLOW_SENSOR1_LED, DISABLE_LED);
  pinMode(WATER_FLOW_SENSOR2_LED, OUTPUT);
  digitalWrite(WATER_FLOW_SENSOR2_LED, DISABLE_LED);
  pinMode(WATER_FLOW_SENSOR2_VALVE, OUTPUT);
  digitalWrite(WATER_FLOW_SENSOR2_VALVE, DISABLE_WATER_FLOW_SENSOR_2_VALVE);
  pinMode(AIR_FLOW, OUTPUT);
  digitalWrite(AIR_FLOW, DISABLE_AIR_FLOW);
  pinMode(AIR_FLOW_LED, OUTPUT);
  digitalWrite(AIR_FLOW_LED, DISABLE_LED);
}

void init_task_manager_timer(void (* task_manager_timer_interrupt_handler)(void)) {
  task_manager_interrupt_handler = task_manager_timer_interrupt_handler;
  Timer1.setPeriod(1000);
  Timer1.enableISR(CHANNEL_A);
}

ISR(TASK_MANAGER_TIMER) {
  task_manager_interrupt_handler();
}

void enable_water_flow_senor1_led(void) {
  digitalWrite(WATER_FLOW_SENSOR1_LED, ENABLE_LED);
}

void disable_water_flow_senor1_led(void) {
  digitalWrite(WATER_FLOW_SENSOR1_LED, DISABLE_LED);
}

void toggle_water_flow_senor1_led(void) {
  digitalWrite(WATER_FLOW_SENSOR1_LED, !digitalRead(WATER_FLOW_SENSOR1_LED));
}

void enable_water_flow_senor2_valve(void) {
  digitalWrite(WATER_FLOW_SENSOR2_VALVE, ENABLE_WATER_FLOW_SENSOR_2_VALVE);
}

void disable_water_flow_senor2_valve(void) {
  digitalWrite(WATER_FLOW_SENSOR2_VALVE, DISABLE_WATER_FLOW_SENSOR_2_VALVE);
}

void enable_water_flow_senor2_led(void) {
  digitalWrite(WATER_FLOW_SENSOR2_LED, ENABLE_LED);
}

void disable_water_flow_senor2_led(void) {
  digitalWrite(WATER_FLOW_SENSOR2_LED, DISABLE_LED);
}

void enable_air_flow(void) {
  digitalWrite(AIR_FLOW, ENABLE_AIR_FLOW);
}

void disable_air_flow(void) {
  digitalWrite(AIR_FLOW, DISABLE_AIR_FLOW);
}

void enable_air_flow_led(void) {
  digitalWrite(AIR_FLOW_LED, ENABLE_LED);
}

void disable_air_flow_led(void) {
  digitalWrite(AIR_FLOW_LED, DISABLE_LED);
}

void toggle_air_flow_led(void) {
  digitalWrite(AIR_FLOW_LED, !digitalRead(AIR_FLOW_LED));
}
