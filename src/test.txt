#include <ModbusMaster.h>
#include <SoftwareSerial.h>

#define SerialMon Serial
SoftwareSerial mySerial(19, 18); // RX, TX
ModbusMaster node;
float distance;
float mode;
String distanceStr = "";
int distanceArray[15];


int calculateMode(int arr[], int size) {
  int maxCount = 0, mode = 0;
  for (int i = 0; i < size; i++) {
    int count = 0;
    for (int j = 0; j < size; j++) {
      if (arr[j] == arr[i]) {
        count++;
      }
    }
    if (count > maxCount) {
      maxCount = count;
      mode = arr[i];
    }
  }
  return mode;
}

void resetDistance(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = 0;
  }
}

void getDistance() {
  SerialMon.print("Ultrasonic Sensor: ");
  // Calculate distance
  for (int i = 0; i <= 14; i++) {
    if (node.readHoldingRegisters(0x0200, 15) == node.ku8MBSuccess) {
      SerialMon.println("OK");
      
      distance = node.getResponseBuffer(14) * 0.003384 * 2.0 * 2.54;
      distanceArray[i] = distance;
      SerialMon.println("Distance " + String(i) + ": " + String(distanceArray[i])); 
      delay(1000);
    }
    else {
      SerialMon.println("Failed");
      break;
    }
  }
  mode = calculateMode(distanceArray, 15); // Solve for mode
  resetDistance(distanceArray, 15);
  distanceStr = String(mode);
  SerialMon.println("Distance after reset: " + String(distanceArray[0]));
}

void setup() {
  // Modbus communication runs at 9600 baudrate
  Serial.begin(115200);
  mySerial.begin(9600);
  // Modbus slave ID 1
  node.begin(1, mySerial);
}

void loop()
{
  getDistance();
}