#include <Arduino.h>
#include <XBee.h>
#include <SoftwareSerial.h>

#ifndef STATUS_LED
#define STATUS_LED 13
#endif

#ifndef ERROR_LED
#define ERROR_LED 13
#endif

#define ZB_RX_PIN 3 // Corresponds to TX on shield
#define ZB_TX_PIN 2 // Corresponds to RX on shield

void initMessaging();
void flashLed(int, int, int);
void sendMessage(char*, uint32_t);
