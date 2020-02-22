#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
Servo servo;
int Status = 12;  // Digital pin D6
int sensor = D7;  // Digital pin D7
String command;
String data="";
void callback(char* topic, byte* payload, unsigned int payloadLength);

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "AndroidAP";
const char* password = "januakbar";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX
#define ORG "br1azj"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "8185"
#define TOKEN "123456789" //  Authentication Token OF THE DEVICE

//  PIN DECLARATIONS 
#define led1 D0
#define led2 D1
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
  Serial.println();
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(D2,OUTPUT);//trig
  pinMode(D3,INPUT);//echo
  wifiConnect();
  mqttConnect();
servo.attach(2); //D4

servo.write(0);

delay(1000);
  pinMode(sensor, INPUT);   // declare sensor as input
  pinMode(Status, OUTPUT);  // declare LED as output
  Serial.begin(115200);
pinMode (D1,INPUT); //echo
pinMode (D2,OUTPUT); //trigger

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
    command+= (char)payload[i];
  }
  Serial.print("data: "+ command);
  control_func();
  command= "";
}

void control_func()
{
  if(command== "lightoff")
 {
 digitalWrite(led1,LOW);
      digitalWrite(led2,LOW);
     Serial.println(".......lights are off..........");   
  }
  else if(command== "lighton")
  {
     digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
     Serial.println(".......lights are on..........");
  }
  else
  {
    Serial.println("......no commands have been subscribed..........");
    }
}

void publishData() 
{
  digitalWrite(D2,HIGH);
  delay(10);
  digitalWrite(D2,LOW);
  int duration = pulseIn(D1,HIGH);
  int distance = (duration/2)*0.0343;
  Serial.print("Distance: ");
  Serial.println(distance);
  
  String payload = "{\"d\":{\"Distance\":";
  payload += distance;
  payload += "}}";
  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
  long state = digitalRead(sensor);
    if(state == HIGH) {
      digitalWrite (Status, HIGH);
      Serial.println("Motion detected!");
      servo.write(140);
      delay(300);
    }
    else {
      digitalWrite (Status, LOW);
      Serial.println("Motion absent!");
      servo.write(0);
      delay(300);
    }
}
