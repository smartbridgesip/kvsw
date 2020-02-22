#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SSD1306_LCDHEIGHT 64

// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

Servo myservo1;
int Entry,Exit;
String command;
String data="";
void callback(char* topic, byte* payload, unsigned int payloadLength);//use for sending data
int duration1,distance1,duration2,distance2;
// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "likhith reddy";
const char* password = "alekhyareddy";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX
#define ORG "v7b7d6"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "654321"
#define TOKEN "123456789" //  Authentication Token OF THE DEVICE

//  PIN DECLARATIONS 
//-------- Customise the above values --------
const char publishTopic[] = "iot-2/evt/Data/fmt/json";//topic which is sended
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
void wifiConnect();
void mqttConnect();
void setup() {
 
  Serial.begin(115200);
  Serial.println();
 
  myservo.attach(D0);
 
  pinMode(D6,INPUT);
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
    command+= (char)payload[i];
  }
  Serial.print("data: "+ command);
  //control_func();
  command= "";
}

/*void control_func()
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
}*/

void publishData() 
{
  
   Exit = digitalRead(D6); // read input value 
   if (Exit == LOW)
   { 
    Serial.println(" Exit Gate open");
    myservo.write(90);   
    delay(1000);
   } 
   else 
   { 
    Serial.println(" Exit Gate close");
      myservo.write(0);
      delay(1000);
   }  
 
 
  String payload = "{\"d\":{\"Exit\":";
  payload += Exit;
  payload += "}}";
  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
