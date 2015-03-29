#include <JeeLib.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RF24.h>

#define SENSOR_PIN 10
#define LED_PIN 13
#define SLEEP_PERIOD 30000
#define SENSOR_NAME "Aquarium"


OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);
RF24 radio(9, 8);

const uint64_t pipes[2] = { 0xF0F0F0F0E1, 0xF0F0F0F0D2 };

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

struct Message {
  char name[28];
  float temp;
};

Message message;

void setup(void) {
  pinMode(LED_PIN, OUTPUT);

  memcpy(message.name, SENSOR_NAME, sizeof(SENSOR_NAME));
  
  sensors.begin();
  sensors.setResolution(12);

  radio.begin();                          // Start up the radio
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setDataRate(RF24_1MBPS);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.enableDynamicPayloads();
}


void loop(void) { 
  digitalWrite(LED_PIN, LOW);

  sensors.requestTemperatures(); // Send the command to get temperatures
  message.temp = sensors.getTempFByIndex(0);

  radio.write(&message, sizeof(Message));

  Sleepy::loseSomeTime(SLEEP_PERIOD);
}
