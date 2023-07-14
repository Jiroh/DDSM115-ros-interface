// define USE_USBCON before including ros.h to use atmega32u4
// for ddsm115 motor
#define USE_USBCON
#include <Arduino.h>

unsigned char sendbuf[32];
int txden = 15;             // txden assign #15

void setup() {
 
  Serial.begin(57600); //This pipes to the serial monitor
  Serial1.begin(115200); //This is the UART1, pipes to sensors attached to board

  // set motor ID
  sendbuf[0] = (unsigned char) 0xAA;   // motor ID settings
  sendbuf[1] = (unsigned char) 0x55;   // motor ID settings
  sendbuf[2] = (unsigned char) 0x53;   // motor ID settings
  sendbuf[3] = (unsigned char) 1;   // motor ID
  sendbuf[4] = (unsigned char) 0x00;   // null data
  sendbuf[5] = (unsigned char) 0x00;   // null data
  sendbuf[6] = (unsigned char) 0x00;   // null data
  sendbuf[7] = (unsigned char) 0x00;   // null data
  sendbuf[8] = (unsigned char) 0x00;   // null data
  sendbuf[9] = (unsigned char) 0x00;   // null data
  
  // transmit mode data
  for (int i = 0; i < 6; i++)
  {
  digitalWrite(txden, HIGH);
  Serial1.write(sendbuf, 10); 
  delay(500);// wait delay
 // digitalWrite(txden,LOW);
  Serial.println("set motor ID=1");
  }


 // Note：When setting the ID, please make sure that the bus has only one motor. ID setting is allowed once every power reset.
 }

void loop()
{
}
