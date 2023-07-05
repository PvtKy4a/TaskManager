#include "WaterFlowControl.h"
#include "TaskManager.h"

#include <LiquidCrystal_I2C.h>            //для отладки
LiquidCrystal_I2C lcd(0x3f, 20, 4);       //

void setup() {

  lcd.init();                                               //для отладки
  lcd.backlight();                                          //

  init_gpio();

  tm_init_task_manager(10, disable_interrupts, enable_interrupts);
  tm_create_task(lcd_update, NULL, TM_ROLL_MODE, 0, 50);       //для отладки

  init_water_flow_control();

  init_water_flow_senor1_interrupt(water_flow_senor1_interrupt_handler);

  init_task_manager_timer(task_manager_interrupt_handler);

}

void loop() {

  tm_task_manager();

  check_water_flow();

}

void lcd_update(void * context) {                           //для отладки
  lcd.setCursor(0, 0);                                      //
  lcd.print(get_water_flow());                              //
  lcd.setCursor(0, 1);                                      //
  lcd.print(get_air_flow_disable_delay());                  //
}
