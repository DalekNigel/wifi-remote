// ChiBots S&R rescue vehicle remote control sender
//
//  Chris Adams - 2017
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

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// NETWORK: Static IP details...
IPAddress ip(192, 168, 1, 51);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP UDP;

int potVal = 0;       // analog pot reading
char potTens;        // String of pot reading
char potOnes;

const char *ssid = "ChiFi";
const char *password = "robotics";

const uint16_t UDP_LOCAL_PORT =  8050;
const uint16_t UDP_REMOTE_PORT = 8051;
const char UDP_REMOTE_HOST[] =   "192.168.1.52";
char TRIGGER_STRING[] = "ChiBots*";

void setup() {
  // put your setup code here, to run once:
  pinMode(D0, INPUT_PULLUP);   // 0 - Forward
  pinMode(D2, INPUT_PULLUP);   // 2 - Reverse
  pinMode(D3, INPUT_PULLUP);   // 3 - Left
//  pinMode(4, INPUT_PULLUP);   // 4 - dont use
  pinMode(D5, INPUT_PULLUP);   // 5 - Right
//  pinMode(6, INPUT_PULLUP);   // 6 - f2
  pinMode(D7, INPUT_PULLUP);   // 7 - f3
//  pinMode(8, INPUT_PULLUP);   // 8 - f4

  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Sender Connecting to ");
  Serial.println(ssid);

  // Static IP Setup Info Here...
  WiFi.config(ip, gateway, subnet);


  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Sender WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


Serial.println("PROGRAM: starting UDP");
if (UDP.begin(UDP_LOCAL_PORT) == 1)
{
  Serial.println("PROGRAM: UDP started");
}
else
{
  Serial.println("PROGRAM: UDP not started");
}

 
}

void loop() {
  char CMD_STRING[12];

  // read potentiometer for speed control
  potVal = analogRead(A0)/50;
  potVal = potVal + 10;    // too fast- increase pwm delay
  potTens = potVal/10;
  potOnes = potVal - (potTens * 10)+'0';
  potTens = potTens+'0';

  // Serial.print(" potVal-50 =");
  // Serial.println(potVal);


  // read joystick and buttons
  if (digitalRead(D0) == LOW){
    strcpy(CMD_STRING, "Forward-000");
    CMD_STRING[9] = potTens;
    CMD_STRING[10] = potOnes;
    SendUpacket(CMD_STRING);
  }else if (digitalRead(D2) == LOW){
    strcpy(CMD_STRING, "Reverse-000");
    CMD_STRING[9] = potTens;
    CMD_STRING[10] = potOnes;
    SendUpacket(CMD_STRING);
  }else if (digitalRead(D5) == LOW){
    strcpy(CMD_STRING, "Left----035");
    SendUpacket(CMD_STRING);
  }else if (digitalRead(D3) == LOW){
    strcpy(CMD_STRING, "Right---035");
    SendUpacket(CMD_STRING);
  }else if (digitalRead(D7) == LOW){
    strcpy(CMD_STRING, "Button1-050");
    SendUpacket(CMD_STRING);
  }


delay(50);

}

void SendUpacket(const char* CMD_STRING)
{
  Serial.print(TRIGGER_STRING);
  Serial.println(CMD_STRING);
  UDP.beginPacket(UDP_REMOTE_HOST, UDP_REMOTE_PORT);
  UDP.write(TRIGGER_STRING);
  UDP.write(CMD_STRING);
  UDP.endPacket();
}

