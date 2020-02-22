
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "Deepu";
const char* password = "12345678";


// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX

#define ORG "0wzxse"
#define DEVICE_TYPE "ESP8266"
#define DEVICE_ID "smartKitchen"
#define TOKEN "smartKitchen123"  //  Authentication Token OF THE DEVICE

//-------- Customise the below values --------
const char publishTopic[] = "iot-2/evt/kitchen/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;


Servo myservo;
WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);
//void publishData(int jar1, int jar2, int gas_value, int weight);

const int pinCE = D4; //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = D8; //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out
#define gas A0
int publishInterval = 5000; // 30 seconds
long lastPublishMillis;

RF24 wirelessSPI(pinCE, pinCSN); // Declare object from nRF24 library (Create your wireless SPI)
const uint64_t rAddress[] = {0xB00B1E50D2LL, 0xB00B1E50C3LL};  //Create pipe addresses for the 2 nodes to recieve data, the "LL" is for LongLong type

void setup()
{

  pinMode(gas, OUTPUT);
  myservo.attach(D0);//pin attached to servo motor
  Serial.begin(9600);  //start serial to communication
  Serial.println();
  wirelessSPI.begin();  //Start the nRF24 module
  wirelessSPI.openReadingPipe(1, rAddress[0]);     //open pipe o for recieving meassages with pipe address
  wirelessSPI.openReadingPipe(2, rAddress[1]);     //open pipe o for recieving meassages with pipe address
  wirelessSPI.startListening();                 // Start listening for messages
  wifiConnect();
  mqttConnect();
}

void loop()
{

  int gas_value, pos, weight = 10;
  byte pipeNum = 0;
  byte newPacket = false;
  byte jar1, jar2;

  if (wirelessSPI.available(&pipeNum)) {
    newPacket = true;
    if (pipeNum == 1 || pipeNum == 2) {
      wirelessSPI.read(&jar1, 1);
      // Serial.println(jar1);
      wirelessSPI.read(&jar2, 1);
      // Serial.println(jar2);
    }
  }
  if (newPacket) {
    Serial.print(jar1);
    Serial.print(F(", "));
    Serial.println(jar2);
  }
  delay(1000);

  //Read the LPG gas leakage from MQ6 sensor
  gas_value = analogRead(gas);

  // Fan control code based on the Gas sensor values
  if (gas_value > 800)
  {
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(1000);
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(1000);                       // waits 15ms for the servo to reach the position
    }
  }
  if (millis() - lastPublishMillis > publishInterval)
  {
    publishData(jar1, jar2, gas_value, weight);
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
  }
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }

    Serial.println();
  }
}

void publishData(int jar1, int jar2, int gas_value, int weight)
{

  //Generating payload for publishing the values
  String payload = "{\"d\":{\"Jar1\":";
  payload += jar1;
  payload += ",""\"Jar2\":";
  payload +=  jar2;
  payload += ",""\"Gas_level\":";
  payload +=  gas_value;
  payload += ",""\"Weight\":";
  payload +=  weight;
  payload += "}}";

  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
