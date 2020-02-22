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
#define DEVICE_ID "1234"
#define TOKEN "12345678" //  Authentication Token OF THE DEVICE

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
  pinMode(D7,INPUT);//D7 is echo
  pinMode(D8,OUTPUT);//D8 is trigger
  Serial.begin(115200);
  Serial.println();
  pinMode(D3,INPUT);//D3
  pinMode(D4,OUTPUT);//D4
  myservo.attach(D0);
 
  pinMode(D5,INPUT);
 
  wifiConnect();
  mqttConnect();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  

  
// display a line of text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,5);
  display.print("Hello");
 
 
  // update display with all of the above graphics
  display.display();
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
   Entry = digitalRead(D5); // read input value 
   if (Entry == LOW)
   { // check if the input is HIGH
    Serial.println(" Entry Gate open");
      myservo.write(90);
      delay(1000);  
   } 
   else 
   {
    Serial.println(" Entry Gate close"); 
      myservo.write(0); 
   }
   
    digitalWrite(D8,HIGH);
    delay(10);
    digitalWrite(D8,LOW);
    duration1= pulseIn(D7,HIGH);
    distance1=duration1*0.0343/2;
    Serial.print("Distance1 :");
    Serial.print(distance1);
    Serial.println ("cm");
    delay(1000);
                 if(distance1<10){
                  display.clearDisplay();
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(0,5);
                  display.print("Slot1 not avaiable");
                  // update display with all of the above graphics
                  display.display();
                 }

                 
                 else{
                  display.clearDisplay();
                // display a line of text
                  display.setTextSize(2);
                  display.setTextColor(WHITE);
                  display.setCursor(0,5);
                  display.print("Slot1 avaiable");
                  // update display with all of the above graphics
                  display.display();
                 }

                 
 digitalWrite(D4,HIGH);
  delay(10);
  digitalWrite(D4,LOW);
  duration2= pulseIn(D3,HIGH);
  distance2=duration2*0.0343/2;
 Serial.print("Slot2 :");
 Serial.print("Distance2 :");
 Serial.print(distance2);
 Serial.println ("cm");

 
             if(distance2<10){
              display.clearDisplay();
            // display a line of text
              display.setTextSize(2);
              display.setTextColor(WHITE);
              display.setCursor(0,5);
              display.print("Slot2 not avaiable");
              // update display with all of the above graphics
              display.display();
              }
             else{
              
              display.clearDisplay();
            // display a line of text
              display.setTextSize(2);
              display.setTextColor(WHITE);
              display.setCursor(0,5);
              display.print("Slot2 avaiable");
             
             
              // update display with all of the above graphics
              display.display();
              }
  String payload = "{\"d\":{\"Slot1\":";
  payload += distance1;
  payload += ",""\"Entry\":";
  payload +=  Entry;
  payload += ",""\"Slot2\":";
  payload +=  distance2;
  payload += "}}";
  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}
