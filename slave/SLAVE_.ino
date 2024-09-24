#include <Wire.h> 

#define rainPin 2
#define speedPin 3
#define SLAVE_ADDR1 0x03


int prevBTN1 = 1, newBTN1 = 0;
uint16_t RainCounter;

int prevBTN2 = 1, newBTN2 = 0;
uint16_t WindCounter;

void setup() 
{ 
  Wire.begin(SLAVE_ADDR1);                // join i2c bus with address #2 
  Wire.onRequest(requestEvent); // register event
  Serial.begin(9600);           // start serial for output 
} 

void getRain() {
  newBTN1 = digitalRead(rainPin);
  if (!prevBTN1 && newBTN1) {
    RainCounter++;
    Serial.print("Rain Counter: ");
    Serial.println(RainCounter);
  }
  prevBTN1 = digitalRead(rainPin);
  delay(5);
}

void getSpeed() {
  newBTN2 = digitalRead(speedPin);
  if (!prevBTN2 && newBTN2) {
    WindCounter++;
    Serial.print("Speed Counter: ");
    Serial.println(WindCounter);
  }
  prevBTN2 = digitalRead(speedPin);
  delay(5);
}

void loop() 
{ 
  getRain();
  getSpeed();
} 

void requestEvent() 
{
  Wire.write((byte)(RainCounter >> 8));
  Wire.write((byte)(RainCounter & 0xFF));
  Wire.write((byte)(WindCounter >> 8));
  Wire.write((byte)(WindCounter & 0xFF));
  RainCounter = 0;
  WindCounter = 0;
}
