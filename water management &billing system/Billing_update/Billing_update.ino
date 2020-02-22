

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht (DHTPIN, DHTTYPE);
float temperature;
int humidity;
String command;
String data = "";


#define LED_BUILTIN 2
#define SENSOR  D2
#define SENSOR2 D5
long currentMillis = 0;
long currentMillis2 = 0;
long previousMillis = 0;
long previousMillis2 = 0;
int interval = 1000;
int interval2 = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
float calibrationFactor2 = 4.5;
volatile byte pulseCount;
volatile byte pulseCount2;
byte pulse1Sec = 0;
byte pulse1Sec2 = 0;

float flowRate;
float flowRate2;
unsigned int flowMilliLitres;
unsigned int flowMilliLitres2;

unsigned long totalMilliLitres;
unsigned long totalMilliLitres2;


void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}
void callback(char* topic, byte* payload, unsigned int payloadLength);
void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;
}
void callback(char* topic2, byte* payload2, unsigned int payloadLength2);


// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "lahari";
const char* password = "chinni988";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX
#define ORG "r2aqdc"
#define DEVICE_TYPE "nodemcu-32"
#define DEVICE_ID "987654321"
#define TOKEN "9876543210" //  Authentication Token OF THE DEVICE

//  PIN DECLARATIONS
#define led1 D3
#define led2 D4
//-------- Customise the above values --------
const char publishTopic[] = "iot-2/evt/Data/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/home/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);
int publishInterval = 5000; // 30 seconds
long lastPublishMillis;
void publishData();
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(D5,INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
  pinMode(D0, INPUT);
  pinMode(D1, OUTPUT);
  Serial.println();
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
   pulseCount2 = 0;
  flowRate2 = 0.0;
  flowMilliLitres2 = 0;
  totalMilliLitres2 = 0;
  previousMillis2 = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
  dht.begin();

  wifiConnect();
  mqttConnect();
}

void loop() {
  if (millis() - lastPublishMillis > publishInterval)
  {
    publishData();
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
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected())
  {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }

    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("callback invoked for topic: ");
  Serial.println(topic);
  for (int i = 0; i < payloadLength; i++) {
    command += (char)payload[i];
  }
  Serial.print("data: " + command);
  control_func();
  command = "";
}

void control_func()
{
  if (command == "lightoff")
  {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    Serial.println(".......lights are off..........");
  }
  else if (command == "lighton")
  {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    Serial.println(".......lights are on..........");
  }
  else
  {
    Serial.println("......no commands have been subscribed..........");
  }
}

void publishData()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {

    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming fro 
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
    delay(2000);
  }




    currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 > interval2) {

    pulse1Sec2 = pulseCount;
    pulseCount2 = 0; 
      flowRate2 = ((1000.0 / (millis() - previousMillis2)) * pulse1Sec2) / calibrationFactor2;
    previousMillis2 = millis();
    flowMilliLitres2 = (flowRate2 / 60) * 1000;
    totalMilliLitres2 += flowMilliLitres2;
    Serial.print("Flow rate2: ");
    Serial.print(int(flowRate2));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t"); 
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
    delay(1000);
    

  }
    delay(1000);
    digitalWrite(D1, HIGH);
    delay(10);
    digitalWrite(D1, LOW);
    int duration = pulseIn(D0, HIGH);
    int distance = duration * 0.0343 / 2;
    Serial.print("Distance:");
    Serial.println(distance);
    Serial.println("cm");
    delay(2000);
  
  String payload = "{\"d\":{\"flowRate\":";
  payload += int(flowRate);
  payload += ",""\"flowRate2\":";
  payload += int(flowRate2);
  payload += ",""\"Distance\":";
  payload +=  distance;
  payload += "}}";
  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
} 
