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
const long interval = 25;  

const float wheelDiameter = 0.067; 
float circumference = 3.14 * wheelDiameter;
float alpha = 0.1; // Smoothing factor (0 < alpha <= 1) - Lower = more smoothing
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
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulsePeriod = 0;
volatile boolean newPulseDetected = false;
volatile int validPulseCount = 0;
float rpm = 0;

// Moving average filter for additional smoothing
const int FILTER_SIZE = 5;
float rpmHistory[FILTER_SIZE];
int filterIndex = 0;
bool filterFilled = false;

// Debouncing and filtering constants
const unsigned long MIN_PULSE_PERIOD = 5000; // Minimum 5ms between pulses (max 600 RPM)
const unsigned long MAX_PULSE_PERIOD = 1000000; // Maximum 1 second between pulses (min 3 RPM)
const unsigned long DEBOUNCE_TIME = 2000; // 2ms debounce time

void countPulse() {
    unsigned long currentTime = micros();
    unsigned long timeSinceLastPulse = currentTime - lastPulseTime;
    
    // Debounce: ignore pulses that are too close together
    if (timeSinceLastPulse < DEBOUNCE_TIME) {
        return;
    }
    

    if (timeSinceLastPulse >= MIN_PULSE_PERIOD && timeSinceLastPulse <= MAX_PULSE_PERIOD) {
        pulsePeriod = timeSinceLastPulse;
        newPulseDetected = true;
        validPulseCount++;
    }
    
    lastPulseTime = currentTime;
    pulseCount++;
}

float getSmoothedRPM(float newRpm) {
    // Add new value to circular buffer
    rpmHistory[filterIndex] = newRpm;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    
    if (!filterFilled && filterIndex == 0) {
        filterFilled = true;
    }
    
    // Calculate average
    float sum = 0;
    int count = filterFilled ? FILTER_SIZE : filterIndex;
    
    for (int i = 0; i < count; i++) {
        sum += rpmHistory[i];
    }
    
    return sum / count;
}

void sendCANMessage() {
    // Calculate RPM only if we have a new valid pulse
    if (newPulseDetected && pulsePeriod > 0) {
        // Disable interrupts briefly to safely read volatile variables
        noInterrupts();
        unsigned long currentPulsePeriod = pulsePeriod;
        newPulseDetected = false;
        interrupts();
        
        // Calculate instantaneous RPM
        float instantRpm = 60000000.0 / (currentPulsePeriod * pulsesPerRevolution);
        
        // Apply multiple levels of filtering
        // 1. Exponential moving average
        if (rpm == 0) {
            rpm = instantRpm; // First reading
        } else {
            rpm = alpha * instantRpm + (1.0 - alpha) * rpm;
        }
        
        // 2. Moving average filter for additional stability
        rpm = getSmoothedRPM(rpm);
    }
    
    // Check for timeout (no pulse for too long = stopped)
    if (micros() - lastPulseTime > 300000) { // 0.3 second timeout
        rpm = 0;
        validPulseCount = 0;
        // Reset filters
        filterIndex = 0;
        filterFilled = false;
    }
    
    // Ensure RPM is within reasonable bounds
    if (rpm < 0) rpm = 0;
    if (rpm > 600) rpm = 0; // Sanity check for your test bench

    Serial.print("RPM: ");
    Serial.print(rpm, 1); // Print with 1 decimal place
	
    int speed = (int)rpm;
	Serial.print(" Speed: ");
	Serial.print(speed);
	Serial.print(")");
    Serial.println();


	byte data[8];
	data[0] = (speed >> 24) & 0xFF; // Highest byte
	data[1] = (speed >> 16) & 0xFF; // High byte
	data[2] = (speed >> 8) & 0xFF;  // Low byte
	data[3] = speed & 0xFF;
	data[4] = 0 & 0xFF;
	data[5] = 0 & 0xFF;
	data[6] = 0 & 0xFF;
	data[7] = 0 & 0xFF;

	CAN.sendMsgBuf(0x1, 0, 8, data);
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

    // Initialize timing variables
    lastPulseTime = micros();
    rpm = 0;
    validPulseCount = 0;
    newPulseDetected = false;
    
    // Initialize filters
    filterIndex = 0;
    filterFilled = false;
    for (int i = 0; i < FILTER_SIZE; i++) {
        rpmHistory[i] = 0;
    }

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