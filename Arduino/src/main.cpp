#include <SimpleTimer.h>
#include <SPI.h>
#include <mcp_can.h>
#include <Wire.h>

MCP_CAN CAN(9);
SimpleTimer timer;

byte len = 0;
byte buf[8];
unsigned long rxId;

const int sensorPin = 3;
const long interval = 300;  // ms


const float wheelDiameter = 0.067;
const float circumference = 3.14 * wheelDiameter;
const int pulsesPerRevolution = 10;

unsigned long pulseCount = 0;
unsigned long previousMillisLeft = 0;
unsigned long previousMillisRight = 0;

const int leftBlinkerPin = 5;
const int rightBlinkerPin = 6;
const int BeamPin = 11;
const int rearLightPin = 7;
const int rearFogLightPin = 8;
const int frontFogLightPin = 10;
const int parkingLightPin = 15;
const int buzzer_pin = 12;

const int pins[8] = {
  leftBlinkerPin, rightBlinkerPin, rearLightPin,
  rearFogLightPin, frontFogLightPin, BeamPin,
  parkingLightPin, buzzer_pin
};

// Light states
bool leftBlinker = false;
bool rightBlinker = false;
bool highBeam = false;
bool lowBeam = false;
bool rearLights = false;
bool rearfogLight = false;
bool frontfogLight = false;
bool buzz = false;

void countPulse() {
  pulseCount++;
}

void sendCANMessage() {
  float rps = (float)pulseCount / pulsesPerRevolution / (interval / 1000.0);
  float rpm = rps * 60;
  pulseCount = 0;

  Serial.print("RPM: ");
  Serial.println(rpm);

  int speed = (int)rpm;

  byte data[8];
  data[0] = (speed >> 24) & 0xFF;
  data[1] = (speed >> 16) & 0xFF;
  data[2] = (speed >> 8) & 0xFF;
  data[3] = speed & 0xFF;
  data[4] = data[5] = data[6] = data[7] = 0;

  CAN.sendMsgBuf(0x1, 0, 4, data);
}

void blinkLeft() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLeft >= interval) {
    previousMillisLeft = currentMillis;
    digitalWrite(leftBlinkerPin, !digitalRead(leftBlinkerPin));
  }
}

void blinkRight() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisRight >= interval) {
    previousMillisRight = currentMillis;
    digitalWrite(rightBlinkerPin, !digitalRead(rightBlinkerPin));
  }
}

void printBits(byte data) {
		for (int b = 7; b >= 0; b--) {
				Serial.print(bitRead(data, b));
		}
    Serial.println();

}

void processCANMessage(unsigned long id, byte* data, byte length) {
  if (length < 1) return;
  bool state = (data[0] == 1);

  for(int i = 0; i < length; i++) {
    printBits(data[i]);
  }
  Serial.println();
  Serial.print("ID:");
  Serial.print(id);
  Serial.println();


  switch (id) {
    case 0x700: // Direction Indicator Left
      leftBlinker = state;
      break;
    case 0x701: // Direction Indicator Right
      rightBlinker = state;
      break;
    case 0x702: // Beam Low
      lowBeam = state;
      break;
    case 0x703: // Beam High
      highBeam = state;
      break;
    case 0x704: // Fog Front
      frontfogLight = state;
      break;
    case 0x705: // Fog Rear
      rearfogLight = state;
      break;
    case 0x706: // Hazard
      leftBlinker = state;
      rightBlinker = state;
      break;
    case 0x707: // Parking Light
      rearLights = state;
      break;
  }
}



void checkCAN() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    if (CAN.readMsgBuf(&rxId, &len, buf) == CAN_OK) {
      processCANMessage(rxId, buf, len);
    }
  }
}

void checkLoop() {
  // Blinkers
  if (leftBlinker) blinkLeft();
  else digitalWrite(leftBlinkerPin, LOW);

  if (rightBlinker) blinkRight();
  else digitalWrite(rightBlinkerPin, LOW);

  // Rear fog light
  digitalWrite(rearFogLightPin, rearfogLight ? HIGH : LOW);

  // Front fog light
  digitalWrite(frontFogLightPin, frontfogLight ? HIGH : LOW);

  // Rear lights (Parking lights)
  digitalWrite(rearLightPin, rearLights ? HIGH : LOW);
  digitalWrite(parkingLightPin, rearLights ? HIGH : LOW);

  // Beams (for simplicity, HIGH if either is ON)
  digitalWrite(BeamPin, (lowBeam || highBeam) ? HIGH : LOW);

  // Buzzer
  digitalWrite(buzzer_pin, buzz ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);
  Serial.println("CAN receiver test");

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  CAN.setMode(MCP_NORMAL);

  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), countPulse, FALLING);
  timer.setInterval(interval, sendCANMessage);

  for (int i = 0; i < 8; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {
  timer.run();
  checkCAN();
  checkLoop();
}
