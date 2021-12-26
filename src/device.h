#pragma once

#define SONOFF_BASIC 1
#define SONOFF_DUALR2 2

// GPIO HW PIN ASSIGNMENT

#if DEVICE == SONOFF_BASIC

// SONOFF BASIC
#define PIN_RELAY_COOL 12 // D6 GPIO12
#define PIN_RELAY_HEAT 0 // unused
#define PIN_TEMP 14 // D5 GPIO14
#define PIN_LED_WIFI 13 // D7 GPIO13
#undef HEATING_ENABLED
#define SWITCH_ON HIGH
#define SWITCH_OFF LOW
#define LED_ON LOW
#define LED_OFF HIGH

#elif DEVICE == SONOFF_DUALR2

// SONOFF DUAL R2
#define PIN_RELAY_COOL 5 // GPIO5
#define PIN_RELAY_HEAT 12 // GPIO12
#define PIN_TEMP 9 // GPIO9 - Button 1
#define PIN_LED_WIFI 13 // D7 GPIO13
#define HEATING_ENABLED 1
#define SWITCH_ON HIGH
#define SWITCH_OFF LOW
#define LED_ON LOW
#define LED_OFF HIGH

#else

// NodeMCU (Development)

#define PIN_RELAY_COOL 12 // D6 GPIO12
#define PIN_RELAY_HEAT 0 // unused
#define PIN_TEMP 14 // D5 GPIO14
#define PIN_LED_WIFI 13 // D7 GPIO13
#undef HEATING_ENABLED
#define SWITCH_ON LOW
#define SWITCH_OFF HIGH
#define LED_ON LOW
#define LED_OFF HIGH

#endif
