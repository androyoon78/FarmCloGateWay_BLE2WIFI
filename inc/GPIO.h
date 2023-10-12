#ifndef _GPIO_H_
#define _GPIO_H_

#define _CONFIG_DEBUG_MSG_
void CalibrationAllSensors();
void FactoryResetOfSensorValue();

/* GPIO */
/* LED Pin */
int led_pin = 25;   // W_LED
int reset_pin = 32; // RESET (i)
int en_env_pin = 4;  // EN_ENV
int en_ch2_pin = 2;  // EN_CH2
int ce_pin = 23;       // CE

/* -- DAMOA Sensor */
const  int soilPin = 34;   // analog input Pin to receive soil sensor value
const  int soilPin_1 = 34;   // analog input Pin to receive soil sensor value --> GP2  (out2-15)
const  int soilPin_2 = 39;   // analog input Pin to receive soil sensor value --> GP3  (out1-30)
const int addressPin = 5;

// OUT1 --> GP3 --> 39 (15cm)
// OUT2 --> GP2 --> 34 (30cm)
const  int soilPin_15 = 39;
const  int soilPin_30 = 34;

// TEMP OFF
const int tempOffPin = 21;
const  int ecPin = 39;      // analog input Pin to receive ec sensor value
int batPin = 35;   // Analog input Pin to receive battery value
int tempPin = 36;   // anlaog input Pin to receive temperature sensor value
void FactoryResetOfSensorValue();
#endif
