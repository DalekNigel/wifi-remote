// ChiBots S&R project  remote control robot receiver
//
// added speed parameter parsing
//
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// NETWORK: Static IP details...
IPAddress ip(192, 168, 1, 52);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiUDP UDP;

int potVal = 0;          // analog speed parameter from sender
int timeOff = 0;         // adjusted PWM motor off time
unsigned long onMillis;
unsigned long offMillis;
int motorOn = 0;         // is motor on?

const char *ssid = "ChiFi";
const char *password = "robotics";
int debug=0;

const uint16_t UDP_LOCAL_PORT =  8051; // Must match UDP_REMOTE_PORT on sender
char TRIGGER_STRING[] = "ChiBots*";  // Must match TRIGGER_STRING on sender
int moving = 0;
unsigned long moveMillis;
int k = 0;
void setup() {
  // put your setup code here, to run once:

  if (debug) Serial.begin(115200);
  delay(10);

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
      else if (strncmp(packetBuffer+8, "Button1", 7) == 0)
      {
            digitalWrite(D3, LOW);   // Set motor 3 forward
            digitalWrite(D4, LOW);   // Set motor 4 forward
            digitalWrite(D1, LOW);   // Turn motor 1 off
            digitalWrite(D2, LOW);   // Turn motor 2 off
            moving=-1;
            moveMillis = millis();  
      }

  }

  ++k;
}

  // we are moving and command packets can be sent approx
  // every 50ms.  If no packet for 150ms stop the robot.
  if (moving>0 && (millis()-moveMillis > 150)){
          digitalWrite(D3, LOW);   // Set motor 3 forward
          digitalWrite(D4, LOW);   // Set motor 4 forward
          digitalWrite(D1, LOW);   // Turn motor 1 off
          digitalWrite(D2, LOW);   // Turn motor 2 off
          moving=0;
  }

  // control motor pulse - PWM
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
