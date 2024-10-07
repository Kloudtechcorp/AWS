#include <Wire.h> 

#define rainPin 2
#define speedPin 3
#define SLAVE_ADDR1 0x03

int prevBTN1 = 1, newBTN1 = 0;
uint16_t RainCounter = 0;

int prevBTN2 = 1, newBTN2 = 0;
uint16_t WindCounter = 0;
int lastWindCount = 0;
uint16_t GustCounter = 0;
int maxGustCount = 0;
int wind = 0;

unsigned long lastTime = 0;
const unsigned long Timer = 3000;
unsigned long time = 0;

void getRain()
{
  newBTN1 = digitalRead(rainPin);
  if (!prevBTN1 && newBTN1) {
    RainCounter++;
    Serial.print("Rain Counter: ");
    Serial.println(RainCounter);
  }
  prevBTN1 = newBTN1;
  delay(5);
}

void getSpeed()
{
  newBTN2 = digitalRead(speedPin);
  if (!prevBTN2 && newBTN2)
  {
    WindCounter++;
    Serial.print("Speed Counter: ");
    Serial.println(WindCounter);
  }
  prevBTN2 = newBTN2;
  delay(5);
}

void getGustCounter()
{ 
  wind = WindCounter - lastWindCount;
  if (wind > 0)
  {
    GustCounter = wind;
  }
  if (GustCounter > maxGustCount)
  {
    maxGustCount = GustCounter;
    Serial.print("Gust Counter: ");
    Serial.println(maxGustCount);
  }
  lastWindCount = WindCounter;
  delay(5);
}

void setup() 
{ 
  Wire.begin(SLAVE_ADDR1);                // join i2c bus with address #2 
  Wire.onRequest(requestEvent); // register event
  Serial.begin(9600);           // start serial for output 
} 

void loop() 
{ 
  getRain();
  getSpeed();
  time = millis();
  if ((time - lastTime) > Timer)
  {
    getGustCounter();
    lastTime = time;
    GustCounter = 0;
  }
} 

void requestEvent() 
{
  Wire.write((byte)(RainCounter >> 8));
  Wire.write((byte)(RainCounter & 0xFF));
  Wire.write((byte)(WindCounter >> 8));
  Wire.write((byte)(WindCounter & 0xFF));
  Wire.write((byte)(maxGustCount >> 8));
  Wire.write((byte)(maxGustCount & 0xFF));
  WindCounter = 0;
  RainCounter = 0;
  maxGustCount = 0;
}
