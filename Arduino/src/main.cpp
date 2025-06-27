#include <SimpleTimer.h>
#include <SPI.h>
#include <mcp_can.h>
#include <Wire.h>

//leftblinker 4 
//rightblinker 8 
// rearfog 7 
// beam 5  / 6 
MCP_CAN CAN(10);
SimpleTimer timer;

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

const int leftBlinkerPin = 4;
const int rightBlinkerPin = 8;
const int rightBeamPin = 5;
const int leftBeamPin = 6;
const int rearFogLightPin = 7;

// Light states
bool leftBlinker = false;
bool rightBlinker = false;
bool highBeam = false;
bool lowBeam = false;
bool rearfogLight = false;
bool parkingLight = false;

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
  data[0] = (speed >> 24) & 0xFF;
  data[1] = (speed >> 16) & 0xFF;
  data[2] = (speed >> 8) & 0xFF;
  data[3] = speed & 0xFF;
  data[4] = data[5] = data[6] = data[7] = 0;

	CAN.sendMsgBuf(0x1, 0, 8, data);
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
    case 0x705: // Fog Rear
      rearfogLight = state;
      break;
    case 0x707: // Parking Light
      parkingLight = state;
      break;
  }
}

void checkCAN() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    if (CAN.readMsgBuf(&rxId, &len, buf) == CAN_OK) {
      processCANMessage(rxId, buf, len);
      sleep(10);
    }
  }
}

void checkLoop() {

  if (leftBlinker) blinkLeft();
  else digitalWrite(leftBlinkerPin, LOW);

  if (rightBlinker) blinkRight();
  else digitalWrite(rightBlinkerPin, LOW);

  digitalWrite(rearFogLightPin, rearfogLight ? HIGH : LOW);

  if(parkingLight) {
    analogWrite(leftBeamPin, 64);
    analogWrite(rightBeamPin, 64); 
  } else if(highBeam) {
    analogWrite(leftBeamPin, 255);
    analogWrite(rightBeamPin, 255);
  } else if (lowBeam) {
    analogWrite(leftBeamPin, 128);
    analogWrite(rightBeamPin, 128);
  }
  else {
    analogWrite(leftBeamPin, 0);
    analogWrite(rightBeamPin, 0);
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

  pinMode(leftBlinkerPin, OUTPUT);
  pinMode(rightBlinkerPin, OUTPUT);
  pinMode(leftBeamPin, OUTPUT);
  pinMode(rightBeamPin, OUTPUT);
  pinMode(rearFogLightPin, OUTPUT);

}

void loop()
{
	timer.run();
	checkCAN();
	checkLoop();

}
