#include "./messaging.h"

SoftwareSerial soft_serial(ZB_RX_PIN, ZB_TX_PIN);
XBee xbee = XBee();

void initMessaging() {
  pinMode(STATUS_LED, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);

  soft_serial.begin(9600);
  xbee.setSerial(soft_serial);
}

void flashLed(int pin, int times, int wait) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}

void sendMessage(char* buffer, uint32_t address) {
  ZBTxStatusResponse txStatus = ZBTxStatusResponse();
  XBeeAddress64 zbAddress = XBeeAddress64(0x0013A200, address);
  Serial.println(strlen(buffer));
  ZBTxRequest zbTx = ZBTxRequest(zbAddress, (uint8_t*)buffer, strlen(buffer));

  xbee.send(zbTx);

  Serial.println("Sending Message");

  if (xbee.readPacket(500)) {
    // got a response!
    Serial.println("Received response");

    // should be a znet tx status             
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        flashLed(STATUS_LED, 5, 50);
        Serial.println("Success");
      } 
      else {
        // the remote XBee did not receive our packet. is it powered on?
        flashLed(ERROR_LED, 3, 50);
        Serial.println("Error XBee did not receive");
      }
    }
  } 
  else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  } 
  else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(ERROR_LED, 2, 50);
    Serial.println("Error XBee with TX Status");
  }
  
}
