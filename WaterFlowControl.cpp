#include "WaterFlowControl.h"

#define WATER_SENSOR1_POLL_TIMEOUT_MS  1000
#define WATER_FLOW_COUNT_PER_LITER  22

#define AIR_FLOW_DISABLE_DELAY_RANGE_MS_MIN  10000
#define AIR_FLOW_DISABLE_DELAY_RANGE_MS_MAX  60000

#define WATER_FLOW_SENSOR1_LED_BLINK_PERIOD_MS  1000

#define AIR_FLOW_LED_BLINK_TIMEOUT_MS 500

#define CHECK_FOR_WATER_FLOW_MISSING_DELAY_MS   4000
#define CHECK_FOR_WATER_FLOW_MISSING_TIMEOUT_MS 500

#define POLL_AIR_FLOW_DISABLE_REGULATION_TIMEOUT_MS 50

#define TASK_MANAGER_CAPACITY 10

typedef enum {
  WAIT_FOR_WATER_FLOW,
  WATER_FLOW_LESS_THAN_2_LITERS,
  WATER_FLOW_MORE_THAN_2_LITERS,
  WATER_FLOW_LESS_THAN_4_LITERS,
  WATER_FLOW_MORE_THAN_4_LITERS
} water_flow_sensor_state_t;

typedef enum {
  AIR_FLOW_DISABLED,
  AIR_FLOW_ENABLED,
  AIR_FLOW_WAITING_FOR_OFF
} air_flow_state_t;

typedef struct {
  water_flow_sensor_state_t water_flow_sensor1_state;
  water_flow_sensor_state_t water_flow_sensor2_state;
  air_flow_state_t air_flow_state;
  float water_flow;
  volatile uint8_t water_flow_senor1_counter;
  uint32_t air_flow_disable_delay;
} WaterFlowControl_t;

WaterFlowControl_t water_flow_control = {0};

void poll_water_flow_senor1(void * context);
void check_for_water_flow_missing(void * context);
void poll_air_flow_disable_regulation(void * context);

void init_water_flow_control(void) {
  water_flow_control.water_flow_sensor1_state = WAIT_FOR_WATER_FLOW;
  water_flow_control.water_flow_sensor2_state = WAIT_FOR_WATER_FLOW;
  water_flow_control.air_flow_state = AIR_FLOW_DISABLED;
  water_flow_control.air_flow_disable_delay = AIR_FLOW_DISABLE_DELAY_RANGE_MS_MIN;
  tm_create_task(poll_water_flow_senor1, NULL, TM_ROLL_MODE, 0, WATER_SENSOR1_POLL_TIMEOUT_MS);
  tm_create_task(poll_air_flow_disable_regulation, NULL, TM_ROLL_MODE, 0, POLL_AIR_FLOW_DISABLE_REGULATION_TIMEOUT_MS);
  tm_create_task(check_for_water_flow_missing, NULL, TM_ROLL_MODE, CHECK_FOR_WATER_FLOW_MISSING_DELAY_MS, CHECK_FOR_WATER_FLOW_MISSING_TIMEOUT_MS);
}

void poll_water_flow_senor1(void * context) {
  water_flow_control.water_flow = (float)water_flow_control.water_flow_senor1_counter / (float)WATER_FLOW_COUNT_PER_LITER;
  water_flow_control.water_flow_senor1_counter = 0;
}

uint32_t convert_value_range(uint32_t value,
        uint32_t range_from_min,
        uint32_t range_from_max,
        uint32_t range_to_min,
        uint32_t range_to_max) {
  return (((value - range_from_min) * (range_to_max - range_to_min)) / (range_from_max - range_from_min)) + range_to_min;;
}

void poll_air_flow_disable_regulation(void * context) {
  uint16_t adc_value = get_adc_value(AIR_FLOW_DISABLE_REGULATION_ADC_CHANNEL);
  water_flow_control.air_flow_disable_delay = convert_value_range((uint32_t)adc_value, ADC_RANGE_MIN, ADC_RANGE_MAX, AIR_FLOW_DISABLE_DELAY_RANGE_MS_MIN, AIR_FLOW_DISABLE_DELAY_RANGE_MS_MAX);
}

void blink_water_flow_senor1_led(void * context) {
  uint32_t blink_timeout_ms = 0;
  blink_timeout_ms = ((uint32_t)WATER_FLOW_SENSOR1_LED_BLINK_PERIOD_MS - ((uint32_t)water_flow_control.water_flow * (uint32_t)250)) / (uint32_t)2;
  tm_set_timeout(blink_water_flow_senor1_led, blink_timeout_ms);
  toggle_water_flow_senor1_led();
}

void blink_air_flow_led(void * context) {
  toggle_air_flow_led();
}

void disable_air_flow(void * context) {
  disable_air_flow();
  disable_air_flow_led();
  tm_delete_task(blink_air_flow_led);
}

void check_for_water_flow_missing(void * context) {
  if (water_flow_control.water_flow_sensor1_state == WATER_FLOW_LESS_THAN_4_LITERS && water_flow_control.water_flow < 0.2) {
    disable_water_flow_senor1_led();
    water_flow_control.water_flow_sensor1_state = WAIT_FOR_WATER_FLOW;
    water_flow_control.water_flow_sensor2_state = WAIT_FOR_WATER_FLOW;
    tm_delete_task(blink_water_flow_senor1_led);
    tm_create_task(disable_air_flow, NULL, TM_ONE_SHOT_MODE, water_flow_control.air_flow_disable_delay, 0);
    tm_create_task(blink_air_flow_led, NULL, TM_ROLL_MODE, 0, AIR_FLOW_LED_BLINK_TIMEOUT_MS);
  }
}

void check_water_flow(void) {
  if (water_flow_control.water_flow_sensor1_state == WAIT_FOR_WATER_FLOW && water_flow_control.water_flow > 0.3) {
    enable_air_flow();
    enable_air_flow_led();
    water_flow_control.water_flow_sensor1_state = WATER_FLOW_LESS_THAN_4_LITERS;
    tm_delete_task(disable_air_flow);
    tm_delete_task(blink_air_flow_led);
    tm_create_task(blink_water_flow_senor1_led, NULL, TM_ROLL_MODE, 0, WATER_FLOW_SENSOR1_LED_BLINK_PERIOD_MS);
  }
  if (water_flow_control.water_flow_sensor1_state == WATER_FLOW_LESS_THAN_4_LITERS && water_flow_control.water_flow > 4) {
    tm_delete_task(blink_water_flow_senor1_led);
    enable_water_flow_senor1_led();
    enable_water_flow_senor2_valve();
    water_flow_control.water_flow_sensor1_state = WATER_FLOW_MORE_THAN_4_LITERS;
    water_flow_control.water_flow_sensor2_state = WATER_FLOW_LESS_THAN_2_LITERS;
  }
  if (water_flow_control.water_flow_sensor2_state == WATER_FLOW_LESS_THAN_2_LITERS && get_water_flow_senor2_state()) {
    enable_water_flow_senor2_led();
    water_flow_control.water_flow_sensor2_state = WATER_FLOW_MORE_THAN_2_LITERS;
  }
  if (water_flow_control.water_flow_sensor2_state == WATER_FLOW_MORE_THAN_2_LITERS && !get_water_flow_senor2_state()) {
    disable_water_flow_senor2_led();
    disable_water_flow_senor2_valve();
    water_flow_control.water_flow_sensor1_state = WATER_FLOW_LESS_THAN_4_LITERS;
    water_flow_control.water_flow_sensor2_state = WAIT_FOR_WATER_FLOW;
    tm_create_task(blink_water_flow_senor1_led, NULL, TM_ROLL_MODE, 0, WATER_FLOW_SENSOR1_LED_BLINK_PERIOD_MS);
    tm_set_delay(check_for_water_flow_missing, CHECK_FOR_WATER_FLOW_MISSING_DELAY_MS);
  }
}

float get_water_flow(void) {
  return water_flow_control.water_flow;
}

uint32_t get_air_flow_disable_delay(void) {
  return water_flow_control.air_flow_disable_delay;
}

void water_flow_senor1_interrupt_handler(void) {
  ++water_flow_control.water_flow_senor1_counter;
}

void task_manager_interrupt_handler(void) {
  tm_sheduler();
}
