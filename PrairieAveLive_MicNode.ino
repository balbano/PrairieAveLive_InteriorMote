/***********************************************
Microphone Node, Arduino Fio, Electret mic, XBEE
************************************************/

#include <XBee.h>

// SET UP XBEE
XBee xbee = XBee();

uint8_t payload[] = {0};

XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0); // coord 0x0013a200, 0x40ace8df
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

// SET UP AUDIO
const int micPin = 1;
const uint8_t sampleFloor = 100; // Use mic_check sketch to determine value.
const uint8_t sampleCeiling = 190; // Use mic_check sketch to determine value.
const int sampleWindow = 40; // 25 FPS. (Panel runs at 12 FPS).
unsigned long prevMillis;

void setup() 
{
  Serial.begin(9600);
  xbee.setSerial(Serial);
  
  prevMillis = millis();
}

void loop() 
{
  uint8_t sampleMax = 0;
  while (millis() - prevMillis < sampleWindow) {
    uint8_t rawSample = getSample(micPin);
    if (rawSample > sampleMax) {
      sampleMax = rawSample;
    }
  }
  prevMillis = millis();
  
  uint8_t factoredSample = factorSample(sampleMax, sampleFloor, sampleCeiling);
  
  payload[0] = factoredSample;
  
  xbee.send(zbTx);
}

uint8_t getSample(int pin) {
  int sample = analogRead(pin);
  sample = map(sample, 0, 1023, 0, 255);
  return uint8_t(constrain(sample, 0, 255));
}

uint8_t factorSample(uint8_t rawSample, uint8_t thresholdLow, uint8_t thresholdHigh) {
  int factoredSample = map(rawSample, thresholdLow, thresholdHigh, 0, 255);
  return uint8_t(constrain(factoredSample, 0, 255));
}
