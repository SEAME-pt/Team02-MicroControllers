#include <SimpleTimer.h>
#include <SPI.h>          
#include <mcp_can.h>
#include <Wire.h>


MCP_CAN CAN(10);
SimpleTimer timer;
byte data[1] = {20};
byte len = 0;
byte buf[8];
unsigned long rxId;


const int sensorPin = 3;  
const long interval = 10;  

const float wheelDiameter = 0.067; 
float circumference = 3.14 * wheelDiameter;
float alpha = 0.5; // Smoothing factor (0 < alpha <= 1)
//float smoothedSpeed = 0;

const int pulsesPerRevolution = 10;
int pulseCount = 0;

unsigned long previousMillisLeft = 0;
unsigned long previousMillisRight = 0;

const int leftBlinkerPin = 5;
const int rightBlinkerPin = 6;
const int BeamPin = 11;
const int rearLightPin = 7;
const int rearFogLightPin = 8;
const int frontFogLightPin = 4;
const int parkingLightPin = 15;
const int buzzer_pin = 12;
const int pins[8] = {leftBlinkerPin, rightBlinkerPin, rearLightPin, rearFogLightPin, frontFogLightPin, BeamPin, parkingLightPin, buzzer_pin};

bool leftBlinker = false;
bool rightBlinker = false;
bool highBeam = false;
bool mediumBeam = false;
bool lowBeam = false;
bool rearLights = false;
bool rearfogLight = false;
bool frontfogLight = false;
bool buzz = false;

// Add these global variables
unsigned long lastPulseTime = 0;
unsigned long currentPulseTime = 0;
volatile unsigned long pulsePeriod = 0;
volatile boolean newPulseDetected = false;
float rpm = 0;

void countPulse() {
    currentPulseTime = micros();
    pulsePeriod = currentPulseTime - lastPulseTime;
	rpm = 60000000.0 / (pulsePeriod * pulsesPerRevolution);
    lastPulseTime = currentPulseTime;
	pulseCount++;
}


void sendCANMessage() {

    Serial.print("RPM: ");
    Serial.println(rpm);


	if (micros() - lastPulseTime > 100000) {
        rpm = 0;
    }
	int speed = (int) rpm;

	byte data[8];
	data[0] = (speed >> 24) & 0xFF; // Highest byte
	data[1] = (speed >> 16) & 0xFF; // High byte
	data[2] = (speed >> 8) & 0xFF;  // Low byte
	data[3] = speed & 0xFF;
	data[4] = 0 & 0xFF;
	data[5] = 0 & 0xFF;
	data[6] = 0 & 0xFF;
	data[7] = 0 & 0xFF;

	CAN.sendMsgBuf(0x1, 0, 4, data);
}

void printBits(byte data) {
		for (int b = 7; b >= 0; b--) {
				Serial.print(bitRead(data, b));
		}
}

void blinkLeft() {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillisLeft >= interval) {
        previousMillisLeft = currentMillis;
        int state = digitalRead(leftBlinkerPin);
        digitalWrite(leftBlinkerPin, !state);
    }
}

void blinkRight() {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillisRight >= interval) {
        previousMillisRight = currentMillis;
        int state = digitalRead(rightBlinkerPin);
        digitalWrite(rightBlinkerPin, !state);
    }
}

void byteToBitsArray(byte b)
{
	if ((b >> 0) & 0x01)
	{
		rightBlinker = true;
	}
	else
	{
		rightBlinker = false;
	}
		
	if ((b >> 1) & 0x01)
	{
		leftBlinker = true;
	}
	else
	{
		leftBlinker = false;
	}

	if ((b >> 5) & 0x01)
	{
		rearfogLight = true;
	}
	else
	{
		rearfogLight = false;
	}

	if ((b >> 2) & 0x01)
	{
		mediumBeam = true;
	}
	else
	{
		mediumBeam = false;
	}

	// for (int i = 0; i < 8; i++) {
	//       int bit = (b >> (7 - i)) & 0x01;
	//       digitalWrite(pins[i], bit);
	// }
}

void processCANMessage(unsigned long id, byte* data, byte length) {
		switch (id) {
				case 0x03: 
				 // Lights message
						Serial.print("Lights bits: ");
						printBits(data[0]);
						byteToBitsArray(data[0]);
						Serial.println();
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

void checkLoop()
{
    if (leftBlinker) {
		blinkLeft();
    } else {
        digitalWrite(leftBlinkerPin, LOW);
    }

    if (rightBlinker) {
		blinkRight();
    } else {
        digitalWrite(rightBlinkerPin, LOW);
    }

	if (rearfogLight)
		digitalWrite(rearFogLightPin, HIGH);
	else
		digitalWrite(rearFogLightPin, LOW);
	
	if (mediumBeam)
	{
		digitalWrite(rearLightPin, HIGH);
	}
	else
	{
		digitalWrite(rearLightPin, LOW);
	}
}

void setup()
{
	Serial.begin(9600);

	Serial.println("CAN transmitter test");
	 
	if(CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
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



void loop()
{
	timer.run();
	checkCAN();
	checkLoop();

}