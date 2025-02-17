#include <SimpleTimer.h>
#include <SPI.h>          
#include <mcp_can.h>
#include <Wire.h>


MCP_CAN CAN(9);
SimpleTimer timer;
byte data[1] = {20};
byte len = 0;
byte buf[8];
unsigned long rxId;


const int sensorPin = 3;  
const long interval = 300;  

const float wheelDiameter = 0.067; 
float circumference = 3.14 * wheelDiameter;
float alpha = 0.5; // Smoothing factor (0 < alpha <= 1)
//float smoothedSpeed = 0;

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

void countPulse() {
	pulseCount++;
}

void sendCANMessage() {
	float rps = (float)pulseCount / pulsesPerRevolution / (interval / 1000.0);
	//smoothedSpeed = alpha * rps + (1 - alpha) * smoothedSpeed;
	float rpm = rps * 60; // Convert to RPM
	pulseCount = 0;

	Serial.print("RPM: ");
	Serial.println(rpm);

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
	 
	if(CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) 
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




// int reading = 0;

// void setup() {
//   Wire.begin();   
//   pinMode(leftBlinkerPin, OUTPUT);
//   pinMode(rightBlinkerPin, OUTPUT);
//   pinMode(frontLight1, OUTPUT);
//   pinMode(frontLight2, OUTPUT);
//   pinMode(readFogLight, OUTPUT);  
//   pinMode(buzzer_pin, OUTPUT); 
//   Serial.begin(9600);
// }





// void controlFrontLights(bool highBeam, bool mediumBeam, bool lowBeam) {
//   if (highBeam) {
//     analogWrite(frontLight1, 255);
//     analogWrite(frontLight2, 255);
//   } else if (mediumBeam) {
//     analogWrite(frontLight1, 128);
//     analogWrite(frontLight2, 128);
//   } else if (lowBeam) {
//     analogWrite(frontLight1, 64);
//     analogWrite(frontLight2, 64);
//   } else {
//     analogWrite(frontLight1, 0);
//     analogWrite(frontLight2, 0);
//   }
// }

// void controlRearLights(bool rearLights) {
//   if (rearLights) {
//     analogWrite(rearLight1, 255);
//     analogWrite(rearLight2, 255);
//   } else {
//     analogWrite(rearLight1, 0);
//     analogWrite(rearLight2, 0);
//   }
// }

// void checkDistance(bool buzz) {
//   Wire.beginTransmission(112);
//   Wire.write(byte(0x00));
//   Wire.write(byte(0x51)); 
//   Wire.endTransmission();
//   delay(70); 

//   Wire.beginTransmission(112);
//   Wire.write(0x02);
//   Wire.write(0x24);
//   Wire.endTransmission();
	
//   Wire.beginTransmission(112);
//   Wire.write(0x02); 
//   Wire.endTransmission();

//   Wire.requestFrom(112, 2); 
//   if (2 <= Wire.available()) {
//     reading = Wire.read();
//     reading = reading << 8;
//     reading |= Wire.read();
//     Serial.println(reading);

//     if (reading < 5 && buzz) {
//       digitalWrite(buzzer_pin, HIGH);
//     } else {
//       digitalWrite(buzzer_pin, LOW);
//     }
//   }
// }

// void loop() {
//   bool leftBlinker = true;
//   bool rightBlinker = false;
//   bool highBeam = true;
//   bool mediumBeam = false;
//   bool lowBeam = false;
//   bool rearLights = true;
//   bool fogLight = true;
//   bool buzz = false;


	
//   controlFrontLights(highBeam, mediumBeam, lowBeam);
//   controlRearLights(rearLights);
	
//   if (fogLight) digitalWrite(readFogLight, HIGH);

//   checkDistance(buzz);
// }