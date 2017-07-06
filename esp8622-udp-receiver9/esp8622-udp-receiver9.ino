
// ChiBots S&R project  remote control robot Receiver  06-22-2017
//
//  Chris Adams  snarpco.com   chibots.org
//
//  Using Pololu ROMI robot chassis with motor driver/power board
//        ESP8266 NodeMCU 0.9 board
//        homebrew Gripper with gear motor and L293D - motor driver chip
//
// In ARDUINO
//  Start Arduino and open Preferences window.
//  Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json 
//  into Additional Board Manager URLs field. 
//
//  You can add multiple URLs, separating them with commas.
//
//  Open Boards Manager from Tools > Board menu and 
//  install esp8266 platform 
// 
//  (and don't forget to select your ESP8266 board from Tools > Board menu after installation).
//
//                set programmer=AVRISP-mkII
//                board=NodeMCU 0.9 (ESP-12 Module)
//
//   serial port driver if needed:   http://www.wch.cn/download/CH341SER_EXE.html
//
//
//  UDP portion based on some code from the Arduino Forum in a 
//  topic started by "mrhyde17" 5/8/2016
//  https://forum.arduino.cc/index.php?topic=401445.0
//
//  send formatted data packets using UDP to vehicle based
//  on joystick and buttons.
//
//  example packet        [ChiBots*Forward-020]
//                         |       |       | 
//                         |       |       |
//   UDP trigger (8 chars)-+       |       |
//   Command (8 chars)-------------+       |
//   Parameter (3 chars)-------------------+
//
//   Using ESP8266 ESP-12 NodeMCU Using Arduino IDE connecting
//   to wireless router with WiFi
//
//
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// NETWORK: Static IP details...
IPAddress ip(192, 168, 1, 52);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP UDP;

int potVal = 0;          // analog speed parameter from sender
//int GripButton = 0;           // gripper test button D8


int timeOff = 0;         // adjusted PWM motor off time
unsigned long onMillis;
unsigned long offMillis;
int motorOn = 0;         // is motor on?

const char *ssid = "ChiFi";
const char *password = "robotics";
int debug=1;

const uint16_t UDP_LOCAL_PORT =  8051; // Must match UDP_REMOTE_PORT on sender
char TRIGGER_STRING[] = "ChiBots*";  // Must match TRIGGER_STRING on sender
int moving = 0;
unsigned long moveMillis;
int k = 0;
void setup() {
  // put your setup code here, to run once:

  if (debug) Serial.begin(57600);
  delay(10);
  
//  pinMode(D8, INPUT_PULLUP);   // D8 - button for Gripper test

  pinMode(D6, OUTPUT);     // Gripper OPEN motor as output 
  pinMode(D5, OUTPUT);     // Gripper CLOSE motor as output
  digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
  digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off

  pinMode(D0, OUTPUT);     // Initialize LED D0 as an output
  digitalWrite(D0, HIGH);   // Turn LED off
 
  pinMode(D1, OUTPUT);     // Initialize the motor 1 pin as an output
  pinMode(D2, OUTPUT);     // Initialize the motor 2 pin as an output
  pinMode(D3, OUTPUT);     // Initialize the motor 1 direction pin as an output
  pinMode(D4, OUTPUT);     // Initialize the motor 2 direction pin as an output

  digitalWrite(D1, LOW);   // Turn motor 1 off
  digitalWrite(D2, LOW);   // Turn motor 2 off
  digitalWrite(D3, LOW);   // Set motor 1 forward
  digitalWrite(D4, LOW);   // Set motor 2 forward

  if (debug){
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }

  // Static IP Setup Info Here...
  WiFi.config(ip, gateway, subnet);


  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debug) Serial.print(".");
  }

  if (debug){
    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("PROGRAM: starting UDP");
  }

  UDP.begin(UDP_LOCAL_PORT);
/*    if (UDP.begin(UDP_LOCAL_PORT) == 1)
    {
      Serial.println("PROGRAM: UDP started");
    }
    else
    {
      Serial.println("PROGRAM: UDP not started");
    }
*/  

}

void loop() {
  // put your main code here, to run repeatedly:

  
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; // buffer to hold incoming packet,
  int packetSize = UDP.parsePacket();

//  GripButton = digitalRead(D8);

if (packetSize > 0)
{
  //Serial.print("PROGRAM: received UDP packet of size ");
  //Serial.println(packetSize);
  //Serial.print("PROGRAM: from ");
  IPAddress remote = UDP.remoteIP();
/*  for (int i = 0; i < 4; i++)
  {
    Serial.print(remote[i], DEC);
    if (i < 3)
    {
      Serial.print(".");
    }
  }
  Serial.print(", port ");
  Serial.println(UDP.remotePort());
  */
  UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  // Serial.print("PROGRAM: contents: ");
  packetBuffer[packetSize] = '\0';
  if (debug){
      Serial.print("[");
      Serial.print(packetBuffer);
      Serial.print("]  ");
      Serial.println(k);
    
//    Serial.print("GripButton = ");
//    Serial.println(GripButton);
  }
  if (strncmp(packetBuffer, TRIGGER_STRING, strlen(TRIGGER_STRING)) == 0)
  {
      // trigger received
      potVal = ((packetBuffer[17]-'0')*10)+(packetBuffer[18]-'0');
      if (debug){
          Serial.print("potVal = ");
          Serial.println(potVal);
      }
      if (strncmp(packetBuffer+8, "Forward", 7) == 0 ){
         if ( moving != 1){       // are we already moving?
            digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
            digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
            digitalWrite(D3, LOW);   // Set motor 1 forward
            digitalWrite(D4, LOW);   // Set motor 2 forward
            digitalWrite(D1, HIGH);   // Turn motor 1 on
            digitalWrite(D2, HIGH);   // Turn motor 2 on
            motorOn = 1;
            onMillis = millis();
            moving=1;
          } 
          moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "Reverse", 7) == 0){
         if ( moving != 2){       // are we already moving?
            digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
            digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
            digitalWrite(D3, HIGH);   // Set motor 1 reverse
            digitalWrite(D4, HIGH);   // Set motor 2 reverse
            digitalWrite(D1, HIGH);   // Turn motor 1 on
            digitalWrite(D2, HIGH);   // Turn motor 2 on
            motorOn = 1;
            onMillis = millis();
            moving=2;
         }
         moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "Left---", 7) == 0){
         if ( moving != 3){       // are we already moving?
            digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
            digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
            digitalWrite(D3, LOW);   // Set motor 1 forward
            digitalWrite(D4, HIGH);   // Set motor 2 reverse
            digitalWrite(D1, HIGH);   // Turn motor 1 on
            digitalWrite(D2, HIGH);   // Turn motor 2 on
            motorOn = 1;
            onMillis = millis();
            moving=3;
         }
         moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "Right--", 7) == 0){
         if ( moving != 4){       // are we already moving?
            digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
            digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
            digitalWrite(D3, HIGH);   // Set motor 1 reverse
            digitalWrite(D4, LOW);    // Set motor 2 forward
            digitalWrite(D1, HIGH);   // Turn motor 1 on
            digitalWrite(D2, HIGH);   // Turn motor 2 on
            motorOn = 1;
            onMillis = millis();
            moving=4;
         }
         moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "Button1", 7) == 0){
         if ( moving != -1){       // are we already opening Gripper?
            digitalWrite(D1, LOW);   // Turn motor 1 off
            digitalWrite(D2, LOW);   // Turn motor 2 off
            digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
            digitalWrite(D6, HIGH);   // Turn Gripper OPEN motor on
            moving=-1;
         }
          moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "Button2", 7) == 0){
         if ( moving != -2){       // are we already closing Gripper?
            digitalWrite(D1, LOW);   // Turn motor 1 off
            digitalWrite(D2, LOW);   // Turn motor 2 off
            digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
            digitalWrite(D5, HIGH);   // Turn Gripper CLOSE motor on
            moving=-2;
         }
          moveMillis = millis();  
      }
      else if (strncmp(packetBuffer+8, "ONLINE-", 7) == 0){
          digitalWrite(D0, LOW);   // Turn LED on
          delay(5);
          digitalWrite(D0, HIGH);   // Turn LED off
      }

  }

  ++k;
}

  // we are moving and command packets can be sent approx
  // every 50ms.  If no packet for 150ms stop the robot.
  if (moving != 0 && (millis()-moveMillis > 150)){
          digitalWrite(D6, LOW);   // Turn Gripper OPEN motor off
          digitalWrite(D5, LOW);   // Turn Gripper CLOSE motor off
          digitalWrite(D3, LOW);   // Set motor 3 forward
          digitalWrite(D4, LOW);   // Set motor 4 forward
          digitalWrite(D1, LOW);   // Turn motor 1 off
          digitalWrite(D2, LOW);   // Turn motor 2 off
          moving=0;
  }

  // control drive motor pulse - PWM
  if(moving > 0){
   // PWM - motor ON time (7ms)
   if (motorOn && millis() > onMillis + 7){   
    digitalWrite(D1, LOW);     // Turn motor 1 off
    digitalWrite(D2, LOW);     // Turn motor 2 off
    motorOn = 0;
    offMillis = millis();
   }

   // Robot does not turn right and left fast enough
   // almost stalls. Adjust turn speed here by decreasing
   // motor off time.
   timeOff = potVal;
   if ((moving == 3)||(moving == 4)){
     if (potVal < 12)
        timeOff = 9;    // set limit, so we dont turn too fast
     else
        timeOff = potVal - 3;
   }
   if (timeOff < 0)
      timeOff = 0;      // dont go below zero

   // PWM - motor OFF time (0-20ms)    
   if (!motorOn && millis() > offMillis + timeOff) {  
    digitalWrite(D1, HIGH);    // Turn motor 1 on
    digitalWrite(D2, HIGH);    // Turn motor 2 on
    motorOn = 1;
    onMillis = millis();
   }
  }
  // end PWM    

}
