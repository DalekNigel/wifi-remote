// ChiBots S&R rescue vehicle remote control Sender    09-28-2017
//
//  Chris Adams  snarpco.com   chibots.org
//
//  Using ESP8266 NodeMCU 0.9 board
//        24k pot connected to A0 for speed control
//        Atari 2600 joystick
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
//   LED flashes and an ONLINE packet is sent when sender is connected to WIFI router.
//
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
unsigned long joyMillis;    // last joystick activity

int buttons;        // 1 or 2 buttons on joystick - set by jumper on D7
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

  pinMode(D0, OUTPUT);     // Initialize LED D0 as an output
  pinMode(D1, INPUT_PULLUP);   // 1 - Forward
  pinMode(D2, INPUT_PULLUP);   // 2 - Reverse
  pinMode(D3, INPUT_PULLUP);   // 3 - Left
  pinMode(D4, INPUT_PULLUP);   // 4 - Right
  pinMode(D5, INPUT_PULLUP);   // 5 - button2
  pinMode(D6, INPUT_PULLUP);   // 6 - button1
 
  pinMode(D7, INPUT_PULLUP);   // single button jumper
  

  joyMillis = millis();     // init activity checker
  Serial.begin(57600);
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
  // when pin7 is low - use single button joystick
  // button1 = button + left
  // button2 = button + right
  if (digitalRead(D7) == LOW){
    buttons=1;
    Serial.println("buttons=1");
  }else{
    buttons=2;
    Serial.println("buttons=2");
  }

 
}

void loop() {
  char CMD_STRING[12];

  // read potentiometer for speed control
  potVal = analogRead(A0)/50;
  potVal = potVal + 12;    // too fast- increase pwm delay
  potTens = potVal/10;
  potOnes = potVal - (potTens * 10)+'0';
  potTens = potTens+'0';

  // Serial.print(" potVal-50 =");
  // Serial.println(potVal);


  // read joystick and buttons

  if (digitalRead(D6) == LOW && buttons==2){
    strcpy(CMD_STRING, "Button1-050");
    SendUpacket(CMD_STRING);
  }
  else if (digitalRead(D5) == LOW){
    strcpy(CMD_STRING, "Button2-050");
    SendUpacket(CMD_STRING);
  }
  // forward and button1 not pressed
  else if (digitalRead(D1) == LOW && digitalRead(D6)){
    strcpy(CMD_STRING, "Forward-000");
    CMD_STRING[9] = potTens;
    CMD_STRING[10] = potOnes;
    SendUpacket(CMD_STRING);
  }
  // reverse and button1 not pressed
  else if (digitalRead(D2) == LOW && digitalRead(D6)){
    strcpy(CMD_STRING, "Reverse-000");
    CMD_STRING[9] = potTens;
    CMD_STRING[10] = potOnes;
    SendUpacket(CMD_STRING);
  }
  else if (digitalRead(D4) == LOW){    // left or button1
    if (digitalRead(D6) == LOW && buttons==1)
      strcpy(CMD_STRING, "Button1-050");
    else
      strcpy(CMD_STRING, "Left----055");   // 35 was too fast
    SendUpacket(CMD_STRING);
  }
  else if (digitalRead(D3) == LOW){    // right or button2
    if (digitalRead(D6) == LOW && buttons==1)
      strcpy(CMD_STRING, "Button2-050");
    else
      strcpy(CMD_STRING, "Right---055");
    SendUpacket(CMD_STRING);
  }
  if (millis()-joyMillis > 1500){     // send ONLINE message
    digitalWrite(D0, LOW);   // Turn LED on
    delay(5);
    digitalWrite(D0, HIGH);   // Turn LED off
    strcpy(CMD_STRING, "ONLINE--035");
    SendUpacket(CMD_STRING);
  }


delay(50);

}

void SendUpacket(const char* CMD_STRING)
{
  Serial.print(TRIGGER_STRING);
  Serial.println(CMD_STRING);

//  Serial.println(isConnectedToAP());
  UDP.beginPacket(UDP_REMOTE_HOST, UDP_REMOTE_PORT);
  UDP.write(TRIGGER_STRING);
  UDP.write(CMD_STRING);
  UDP.endPacket();
  joyMillis = millis();     // reset activity checker
}

