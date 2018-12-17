#ifndef __THERMOMETER_H__
#define __THERMOMETER_H__

void early_setup_thermometer();
void setup_thermometer();
void loop_thermometer();

void load_background_image();
void thermometer_print_values(float temperature, float humidity, uint16_t pressure);

#endif // __THERMOMETER_H__
