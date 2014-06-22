/***********************************************
Microphone Node, Arduino Fio, Electret mic, XBEE
************************************************/

#include <XBee.h>
// #include <SoftwareSerial.h>
// All SoftwareSerial stuff commented out to speed up the sample rate.

// SET UP XBEE
// -----------

// create the XBee object
XBee xbee = XBee();

uint8_t payload[] = {0};

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0); // coord 0x0013a200, 0x40ace8df
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
// ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// SET UP AUDIO
// ------------

const int micPin = 1;
const uint8_t sampleFloor = 100; // Set for the room by checking the reading when it is quiet.
const uint8_t sampleCeiling = 190; // Set for the mic by checking the reading when blowing on it.
const int sampleWindow = 10;

// SET UP SOFT SERIAL
// ------------------

// SoftwareSerial softSerial(4, 5); // RX, TX

// SET UP FPS TRACKER
// ------------------
// unsigned long prevTime;

void setup() 
{
  Serial.begin(9600);
  xbee.setSerial(Serial);
  
  // softSerial.begin(4800);
  // softSerial.println("Booting up!");
  
  // prevTime = millis();
}

void loop() 
{
  uint8_t rawSample = getSample(sampleWindow, micPin);
  // softSerial.print("Raw sample: ");
  // softSerial.println(rawSample);
  
  uint8_t factoredSample = factorSample(rawSample, sampleFloor, sampleCeiling);
  // softSerial.print("Factored sample: ");
  // softSerial.println(factoredSample);

  payload[0] = factoredSample;
  // softSerial.print("Payload: ");
  // softSerial.println(payload[0]);
  
  xbee.send(zbTx);
  
  /*
  if (xbee.readPacket(500)) {
    // got a response!
    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        softSerial.println("Delivery success.");
      } else {
        softSerial.println("Delivery failure. The remote XBee did not receive our packet. Is it powered on?");
      }
    }
  } else if (xbee.getResponse().isError()) {
    softSerial.print("Error reading packet.  Error code: ");  
    softSerial.println(xbee.getResponse().getErrorCode());
  } else {
    softSerial.println("Local XBee did not provide a timely TX Status Response -- should not happen");
  }
  */
  
  /*
  float fps = 1000.0 / (millis() - prevTime);
  softSerial.print("FPS: ");
  softSerial.println(fps);
  prevTime = millis();
  */
  
  delay(10);
}

uint8_t getSample(unsigned long window, int pin) {
  unsigned long startMillis = millis();
  int sampleMax = 0;
  while (millis() - startMillis < window) {
    int sample = analogRead(pin);
    if (sample > sampleMax) {
      sampleMax = sample;
    }
  }
  sampleMax = map(sampleMax, 0, 1023, 0, 255);
  return uint8_t(sampleMax);
}

uint8_t factorSample(uint8_t rawSample, uint8_t thresholdLow, uint8_t thresholdHigh) {
  int factoredSample = map(rawSample, thresholdLow, thresholdHigh, 0, 255);
  return uint8_t(constrain(factoredSample, 0, 255));
}
