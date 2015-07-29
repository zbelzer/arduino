#include <OneWire.h>
#include <DallasTemperature.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include "./messaging.h"

#define XBEE_BUFFER_SIZE 100
#define CONTROLLER_ADDRESS 0x40C1BC59

void setup() {
  Serial.begin(9600);
}


void loop() {
  initMessaging();

  reportTemperature("internal", 123.45);
  reportTemperature("tower", 45.67);
  reportEvent("test", "yes");

  delay(5000);
}

void reportTemperature(String name, float temp) {
  char buffer[XBEE_BUFFER_SIZE];
  sprintf(buffer, "{\"kind\":\"metric\", \"name\":\"%s\", \"value\":%f}", name.c_str(), temp);
  sendMessage(buffer, CONTROLLER_ADDRESS);
}

void reportEvent(String name, String value) {
  char buffer[XBEE_BUFFER_SIZE];
  sprintf(buffer, "{\"kind\":\"event\", \"name\":\"%s\", \"value\":\"%s\"}", name.c_str(), value.c_str());
  sendMessage(buffer, CONTROLLER_ADDRESS);
}
