#include <OneWire.h>
#include <DallasTemperature.h>
#include <assert.h>
#include "./util.h"

#define ONE_WIRE_BUS 10
#define RELAY_PIN 6
#define FAN_PIN 9

#define POLLING_INTERVAL 3000
#define XBEE_BUFFER_SIZE 100

#define INTERNAL_TEMP_INDEX 0
#define TOWER_TEMP_INDEX 1
#define COMPRESSOR_DELAY 240000 // Milliseconds

#define TARGET_TEMP 40.0

#define CONTROLLER_ADDRESS 0x40C1BC59

#define DEBUG
// #define FAKE_TEMPS

float fakeinternaltemp;
float faketowertemp;

#ifdef FAKE_TEMPS
  fakeinternaltemp = 72;
  faketowertemp = 72;
#endif

OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

int currentKeggeratorState;
long startTime;
long cycleEndTime;

void setup(void) {
  Serial.println("Starting Program");
  Serial.begin(9600);

  startTime = millis();
  cycleEndTime = millis();

  initializeRelay();
  initializeSensors();
  initializeFan();
  initMessaging();
}

void loop(void) { 
  sensors.requestTemperatures(); // Send the command to get temperatures

  long currentTime = millis();
  float internalTemp = readInternalTemp();
  float towerTemp = readTowerTemp();

  reportTemperature("internal", internalTemp);
  reportTemperature("tower", towerTemp);

  if ((internalTemp > 0) && (towerTemp > 0)) {
    Serial.printf("Internal temperature is: %3.2f\n", internalTemp);
    Serial.printf("Tower temperature is: %3.2f\n", towerTemp);

    long timeSinceCycleEnd = currentTime - cycleEndTime;
    long timeSinceStart = currentTime - startTime;

#ifdef DEBUG
      Serial.printf("DEBUG: Time since cycle end: %u\n", timeSinceCycleEnd / 1000);
      Serial.printf("DEBUG: Time since start: %u\n", timeSinceStart / 1000);
#endif

    if (internalTemp > TARGET_TEMP) {
      if ((timeSinceCycleEnd > COMPRESSOR_DELAY) && (timeSinceStart > COMPRESSOR_DELAY)) {
        setKeggeratorState(HIGH);
      } else {
        Serial.print("Delaying for compressor... ");
        Serial.print((COMPRESSOR_DELAY - timeSinceCycleEnd) / 1000);
        Serial.println(" seconds left");
      }
    } else {
      setKeggeratorState(LOW);
    }
  } else {
    Serial.println("Temperature reading error. Skipping cycle");
  }

  Serial.println();
  delay(POLLING_INTERVAL);
}

void setKeggeratorState(int newKeggeratorState) {
  if (currentKeggeratorState != newKeggeratorState) {
    currentKeggeratorState = newKeggeratorState;

    if (newKeggeratorState == HIGH) {
      reportEvent("power", "on");
      Serial.println("Turning keggerator ON");
      digitalWrite(RELAY_PIN, newKeggeratorState);
    } else if (newKeggeratorState == LOW) {

      reportEvent("power", "off");
      Serial.println("Turning keggerator OFF");
      digitalWrite(RELAY_PIN, newKeggeratorState);

      cycleEndTime = millis();
    } else {
      Serial.print(newKeggeratorState);
      Serial.println(" isn't a valid state");
      abort();
    }
  }
}

void initializeSensors() {
  Serial.println("Initializing Sensors");
  sensors.begin();
  Serial.printf("Found %i devices\n", sensors.getDeviceCount());
}

void initializeRelay() {
  Serial.println("Initializing Relay");
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void initializeFan() {
  Serial.println("Initializing Fan");
  pinMode(FAN_PIN, OUTPUT);
  setFanSpeed(100.0);
}

void setFanSpeed(float percent) {
  float speed = percent / 100.0 * 255.0;
  Serial.printf("Setting fan to %3.0f (%3.2f)\n", speed, percent);

  analogWrite(FAN_PIN, speed);
}

float readInternalTemp() {
#ifdef FAKE_TEMPS
    if (currentKeggeratorState == HIGH) {
      Serial.println("DEBUG: Cooling");

      fakeInternalTemp -= 0.1;
    } else {
      Serial.println("DEBUG: Warming");

      fakeInternalTemp += 0.1;
    }
    return fakeInternalTemp;
#else
    return sensors.getTempFByIndex(INTERNAL_TEMP_INDEX);
#endif
}

float readTowerTemp() {
  return sensors.getTempFByIndex(TOWER_TEMP_INDEX);
}

void reportTemperature(String name, float temp) {
  char buffer[XBEE_BUFFER_SIZE];
  sprintf(buffer, "{\"type\":\"metric\", \"name\":\"%s\", \"value\":\"%f\"}", name.c_str(), temp);
  sendMessage(buffer, CONTROLLER_ADDRESS);
}

void reportEvent(String name, String value) {
  char buffer[XBEE_BUFFER_SIZE];
  sprintf(buffer, "{\"type\":\"event\", \"name\":\"%s\", \"value\":\"%s\"}", name.c_str(), value.c_str());
  sendMessage(buffer, CONTROLLER_ADDRESS);
}
