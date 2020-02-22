#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET D5
/* Object named display, of the class Adafruit_SSD1306 */
Adafruit_SSD1306 display1(OLED_RESET);
Adafruit_SSD1306 display2(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 400;
DynamicJsonBuffer jsonBuffer(capacity);
void callback(char* topic, byte* payload, unsigned int payloadLength);

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "mouni";
const char* password = "mounikareddy";


// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX

#define ORG "8ou8e3"
#define DEVICE_TYPE "NODEMCU"
#define DEVICE_ID "123456"
#define TOKEN "12345678"  //  Authentication Token OF THE DEVICE


String data = "";
float main_temp, wind_speed;
int  main_humidity, count = 0;
const char* weather_0_description;
const int School = D4;
const int Emergency = D6;
const int Hospital = D7;

int Sch = 0;
int Hsp = 0;  
int Emr = 0;
//-------- Customise the below values --------
const char publishTopic[] = "iot-2/evt/home/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/sign/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
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

  wifiConnect();
  mqttConnect();

  display1.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  /* Initialize display with address 0x3C */
  display1.clearDisplay();  /* Clear display */
  display1.setTextSize(1);  /* Select font size of text. Increases with size of argument. */
  display1.setTextColor(WHITE); /* Color of text*/
  display1.display();
  display2.clearDisplay();  /* Clear display */
  display2.setTextSize(1);  /* Select font size of text. Increases with size of argument. */
  display2.setTextColor(WHITE);
  display2.display();
}

void loop() {


   Sch = digitalRead(School);
   if(Sch==1)
     {
    Serial.println("school button pressed");
       school();
     }
   Hsp = digitalRead(Hospital);
   if(Hsp==1)
     {
    Serial.println("Hospital button pressed");
       Hsptl();
     }

 if(digitalRead(Emergency)==1)
     {
       Serial.println("Emergency button pressed");
       publishData();
      }
 

  count = count + 1;
  if (count ==30)
  {
    client.disconnect();
    httprequest();
    delay(2000);
    count=0;
  }
  delay(1000);
  if (millis() - lastPublishMillis > publishInterval)
  {
    //publishData();
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
    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {
    // Serial.println(client.subscribe(topic));
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {

  Serial.print("callback invoked for topic: ");
  Serial.println(topic);


  for (int i = 0; i < payloadLength; i++) {
    //Serial.print((char)payload[i]);
    data += (char)payload[i];
  }

  Serial.print("data: " + data);
  oled_display();
  data = "";
}


void httprequest()
{
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  if (http.begin(wifiClient, "http://api.openweathermap.org/data/2.5/weather?lat=15.834536&lon=78.029366&appid=4f43597ca5d9ea57556f307cc70f1ace&units=metric")) {  // HTTP


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);

        //const char* json = http.getString();

        JsonObject& root = jsonBuffer.parseObject(payload);

        float coord_lon = root["coord"]["lon"]; // 78.49
        float coord_lat = root["coord"]["lat"]; // 17.39

        JsonObject& weather_0 = root["weather"][0];
        int weather_0_id = weather_0["id"]; // 721
        const char* weather_0_main = weather_0["main"]; // "Haze"
        const char* weather_0_description = weather_0["description"]; // "haze"
        const char* weather_0_icon = weather_0["icon"]; // "50d"

        const char* base = root["base"]; // "stations"


        JsonObject& main = root["main"];
        float main_temp = main["temp"]; // 24.85
        float main_feels_like = main["feels_like"]; // 25.33
        float main_temp_min = main["temp_min"]; // 24.44
        int main_temp_max = main["temp_max"]; // 25
        int main_pressure = main["pressure"]; // 1019
        int main_humidity = main["humidity"]; // 61
        float wind_speed = root["wind"]["speed"];
        Serial.println(main_temp);
        Serial.println(main_humidity);
        Serial.println(wind_speed);
        Serial.println(weather_0_description);
        //oled_temp_display();
        display2.clearDisplay();
        display1.setTextColor(WHITE);
        display1.setTextSize(2);
        display2.setTextSize(2);
        display1.setCursor(5, 5);
        display1.print("Temp:");
        display1.print(main_temp);
        display1.setCursor(5, 23);
        display1.print("Humd:");
        display1.print(main_humidity);
        display2.setCursor(5, 5);
        display2.print("wind:");
        display2.print(wind_speed);
        display2.setCursor(5, 23);
        display2.print("Clim:");
        display2.print(weather_0_description);

        display1.display();
        display1.display();
        display2.display();
        delay(5500);
        //        display1.clearDisplay();
                display2.clearDisplay();
        //        display1.display();
                display2.display();

             if(main_temp>30)
             {
               temphigh();
             }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }
}

void oled_display()
{
  display2.setTextColor(WHITE);
  display2.setTextSize(2);
  display2.setCursor(0, 5);
  Serial.println(data);
  display2.print(data);
  display2.display();
  delay(5500);
  display2.clearDisplay();
  display2.display();
}
void school()
{
   display2.setTextColor(WHITE);
    display2.setTextSize(2);
    display2.setTextSize(2);
    display2.setCursor(0,5);
    display2.print("School Zone");
    display2.setCursor(0,40);
    display2.print("spd :30KpH");
    display2.display(); 
    
}


void temphigh()
{
   display2.setTextColor(WHITE);
    display2.setTextSize(2);
    display2.setTextSize(2);
    display2.setCursor(0,5);
    display2.print("Temperature High");
    display2.setCursor(0,40);
    display2.print("spd :75KpH");
    display2.display(); 
    
}
void Hsptl()
{
   display2.setTextColor(WHITE);
    display2.setTextSize(2);
    display2.setTextSize(2);
    display2.setCursor(5,5);
    display2.print("Hospital  Zone");
    display2.setCursor(0,40);
    display2.print("spd :30KpH");
    display2.display(); 
    
}



void publishData() 
{
  String payload ="{\"alert\":\"emergency\"}";
  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }

  delay(2000);
}
