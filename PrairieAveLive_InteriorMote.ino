/* 
  Prairie Avenue Live - Interior Mote
  ===================================
  
  The seven microphone nodes are placed throughout the Arts Incubator.
  Each node transmits volume data from the mic to the microcontroller
  running the LED panel.
  
  Terminology
  -----------
  
  - Mote: Arduino Fio and XBee collects sound levels from mic and transmits to controller.
    - There are multiple interior motes with one mic each and one exterior mote with multiple mics.
  - Mic: the electret mics used by the motes.
  - Controller: Teensy 3.1 and XBee that receive the sound data from the motes and control the LEDs.
  - Node: the origin point (on the LED array) of the visualization for a particular mic.
  
  Parts
  -----
  
  - Arduino Fio: https://www.sparkfun.com/products/10116
  - Electret mic with auto gain: http://www.adafruit.com/products/1713
  - XBee Series 2: http://www.adafruit.com/products/968

*/

#include <XBee.h>
#include <SoftwareSerial.h>

// AUDIO
const int micPin = 1;
uint8_t sampleMax;
const uint8_t sampleFloor = 100; // Use mic_check sketch to determine value.
const uint8_t sampleCeiling = 190; // Use mic_check sketch to determine value.

// XBEE
XBee xbee = XBee();
uint8_t payload[] = {0};
XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0); // coord 0x0013a200, 0x40ace8df
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

XBeeResponse response = XBeeResponse();

// TIMERS
const int timeBetweenTransmissions = 40; // 25 FPS. (Panel runs at 12 FPS).
unsigned long prevTransmissionTime;

const int timeBetweenReports = 1000; // Report once per second.
unsigned long prevReportTime;

// REPORTING
SoftwareSerial softSerial(4, 5); // RX, TX
int numberOfTransmissions;

void setup() 
{
  Serial.begin(57600);
  xbee.setSerial(Serial);
  
  softSerial.begin(57600);
  
  // Initialize the timers and counters;
  prevTransmissionTime = millis();
  prevReportTime = millis();
  numberOfTransmissions = 0;
  sampleMax = 0;
}

void loop() 
{
  uint8_t rawSample = getSample(micPin);
  if (rawSample > sampleMax) {
    sampleMax = rawSample;
  }
  
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    //prevTransmissionTime = millis();
    uint8_t factoredSample = factorSample(sampleMax, sampleFloor, sampleCeiling);
    payload[0] = factoredSample;
    xbee.send(zbTx);
    numberOfTransmissions++;
    sampleMax = 0;
  }
  
  if (millis() - prevReportTime > timeBetweenReports) {
    prevReportTime = millis();
    softSerial.println(numberOfTransmissions);
    numberOfTransmissions = 0;
  }
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
