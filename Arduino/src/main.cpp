#include <SimpleTimer.h>
#include <SPI.h>          
#include <mcp_can.h>


MCP_CAN CAN(9);
SimpleTimer timer;
byte data[1] = {20};
byte len = 0;
byte buf[8];
unsigned long rxId;


const int sensorPin = 3;  
const long interval = 100;  

const float wheelDiameter = 0.067; 
float circumference = 3.14 * wheelDiameter;


const int pulsesPerRevolution = 25;
unsigned long pulseCount = 0;

void countPulse() {
  pulseCount++;
}

void sendCANMessage() {
  float rps = (float)pulseCount / pulsesPerRevolution / (interval / 1000.0);
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

  // for (int i = 0; i < 8; i++)
  // {
  //   Serial.print(data[i]);
  //   Serial.print(", ");
  // }
  // Serial.println("");

  CAN.sendMsgBuf(0x1, 0, 4, data);
}

void printBits(byte data) {
    for (int b = 7; b >= 0; b--) {
        Serial.print(bitRead(data, b));
    }
}

void processCANMessage(unsigned long id, byte* data, byte length) {
    switch (id) {
        case 0x03:  // Lights message
            Serial.print("Lights bits: ");
            printBits(data[0]);
            Serial.println();
            break;
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
}

void loop()
{
  timer.run();

  if(CAN.checkReceive() == CAN_MSGAVAIL) {
        // Read message with proper parameters
        if(CAN.readMsgBuf(&rxId, &len, buf) == CAN_OK) {
            // Serial.print("Received ID: 0x");
            // Serial.println(rxId, HEX);
            
            processCANMessage(rxId, buf, len);
        }
    }
}