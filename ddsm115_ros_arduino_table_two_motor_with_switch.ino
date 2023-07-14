// define USE_USBCON before including ros.h to use atmega32u4
// for ddsm115 motor
#define USE_USBCON
#include <Arduino.h>
#include <ros.h>
#include <geometry_msgs/Twist.h>


unsigned char sendbuf[32];
int txden = 15;             // txden assign #15

//assign stop pin #14 and led pin #8
const uint8_t stop_button_pin1 = 14;
const uint8_t stop_lamp = 8;

//variable
unsigned long pre_time;
String mode;

double w_r=0, w_l=0;
//wheel_rad is the wheel radius ,wheel_sep is
double wheel_rad = 0.100, wheel_sep = 0.235;
int lowSpeed = 200;
int highSpeed = 50;
double speed_ang=0, speed_lin=0;


void messageCb(const geometry_msgs::Twist& twist) {
  const float speed_ang = twist.angular.z;
  const float speed_lin = twist.linear.x;
  w_r = ((speed_lin/(wheel_rad)) + ((speed_ang*wheel_sep)/(2.0*wheel_rad)))*10;
  w_l = ((speed_lin/(wheel_rad)) - ((speed_ang*wheel_sep)/(2.0*wheel_rad)))*-10;
}

ros::NodeHandle nh;
ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", &messageCb);

// CRC8 lookup table
const uint8_t CRC8Table[256] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

// CRC8 calculation using lookup table
uint8_t crc8(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0x00;
  while (len--) {
    crc = CRC8Table[crc ^ *data++];
  }
  return crc;
}

void setup() {
 
  Serial.begin(57600); //This pipes to the serial monitor
  Serial1.begin(115200); //This is the UART1, pipes to sensors attached to board

  //pin mode
  pinMode(stop_button_pin1, INPUT_PULLUP);
  pinMode(stop_lamp, OUTPUT);

  //variable initialize
  mode = "run";


  // set velocity loop mode ID=1
  sendbuf[0] = (unsigned char) 0x01;   // motor ID=1
  sendbuf[1] = (unsigned char) 0xA0;   // motor mode switch command
  sendbuf[2] = (unsigned char) 0x00;   // null data
  sendbuf[3] = (unsigned char) 0x00;   // null data
  sendbuf[4] = (unsigned char) 0x00;   // null data
  sendbuf[5] = (unsigned char) 0x00;   // null data
  sendbuf[6] = (unsigned char) 0x00;   // null data
  sendbuf[7] = (unsigned char) 0x00;   // null data
  sendbuf[8] = (unsigned char) 0x00;   // null data
  sendbuf[9] = (unsigned char) 0x02;   // set velosity loop
  
  // transmit mode data ID=1
  digitalWrite(txden, HIGH);
  Serial1.write(sendbuf, 10); 
  delay(5);// wait delay
  digitalWrite(txden,LOW); 
  
  // set velocity loop mode ID=2
  sendbuf[0] = (unsigned char) 0x02;   // motor ID=2
  sendbuf[1] = (unsigned char) 0xA0;   // motor mode switch command
  sendbuf[2] = (unsigned char) 0x00;   // null data
  sendbuf[3] = (unsigned char) 0x00;   // null data
  sendbuf[4] = (unsigned char) 0x00;   // null data
  sendbuf[5] = (unsigned char) 0x00;   // null data
  sendbuf[6] = (unsigned char) 0x00;   // null data
  sendbuf[7] = (unsigned char) 0x00;   // null data
  sendbuf[8] = (unsigned char) 0x00;   // null data
  sendbuf[9] = (unsigned char) 0x02;   // set velosity loop
  
  // transmit mode data ID=2
  digitalWrite(txden, HIGH);
  Serial1.write(sendbuf, 10); 
  delay(5);// wait delay
  digitalWrite(txden,LOW); 


  
  nh.initNode();
  nh.subscribe(sub);
}

void loop() {


  // Check for emergency stop button press
  if(digitalRead(stop_button_pin1) == LOW){
    mode = "stop";
  } else {
    mode = "run";
  }


  // Check the current mode
  if(mode == "run"){
    digitalWrite(stop_lamp, LOW);  // Turn off the stop lamp

    // Process packet data ID=1
    sendbuf[0] = (unsigned char)0x01;  // Motor ID=1 Right
    sendbuf[1] = (unsigned char)0x64;  // Rotate motor command
    sendbuf[2] = (unsigned char)w_r;   // Velocity high 8 bits
    sendbuf[3] = (unsigned char)0x00;  // Velocity low 8 bits
    sendbuf[4] = (unsigned char)0x00;  // Null data
    sendbuf[5] = (unsigned char)0x00;  // Null data
    sendbuf[6] = (unsigned char)0x02;  // Acceleration time
    sendbuf[7] = (unsigned char)0x00;  // Brake
    sendbuf[8] = (unsigned char)0x00;  // Null data

    // CRC8 calculation using lookup table
    sendbuf[9] = crc8(sendbuf, 9);  // CRC8

    // Transmit data
    digitalWrite(txden, HIGH);
    Serial1.write(sendbuf, 10);
    delay(2);  // Wait delay
    digitalWrite(txden, LOW);

    // Process packet data ID=2
    sendbuf[0] = (unsigned char)0x02;  // Motor ID=2 Left
    sendbuf[1] = (unsigned char)0x64;  // Rotate motor command
    sendbuf[2] = (unsigned char)w_l;   // Velocity high 8 bits
    sendbuf[3] = (unsigned char)0x00;  // Velocity low 8 bits
    sendbuf[4] = (unsigned char)0x00;  // Null data
    sendbuf[5] = (unsigned char)0x00;  // Null data
    sendbuf[6] = (unsigned char)0x02;  // Acceleration time
    sendbuf[7] = (unsigned char)0x00;  // Brake
    sendbuf[8] = (unsigned char)0x00;  // Null data

    // CRC8 calculation using lookup table
    sendbuf[9] = crc8(sendbuf, 9);  // CRC8

    // Transmit data
    digitalWrite(txden, HIGH);
    Serial1.write(sendbuf, 10);
    delay(2);  // Wait delay
    digitalWrite(txden, LOW);
  

  }else if(mode == "stop"){
    // Stop the motors
    w_r = 0x00;
    w_l = 0x00;

    // Turn on the stop lamp
    digitalWrite(stop_lamp, HIGH);
    delay(5);  // Wait delay
  }
   
  nh.spinOnce();
  delay(1);
}
