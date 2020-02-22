
/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <PubSubClient.h>

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "mani"; 
const char* password = "manisasu27";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX

#define ORG "82v4a2"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "654321"
#define TOKEN "12345678"  //  Authentication Token OF THE DEVICE

//-------- Customise the below values --------
const char publishTopic[] = "iot-2/evt/home/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/test/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;


WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);

int publishInterval = 5000; // 30 seconds
long lastPublishMillis;
String Name, euuid;
int rssi;

BLEScan* pBLEScan;

String getValue(String data, char separator, int index);
void publishData(String Name, String euuid, float rssi);
void wifiConnect();
void mqttConnect();

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      String value =  advertisedDevice.toString().c_str();


      if (value.indexOf("serviceUUID") > 0)
      {
        String data1 = getValue(value, ':', 1);
        Name = getValue(data1, ',', 0);

        if (Name == " KVSW")
        {
          Serial.println(value);
          rssi = advertisedDevice.getRSSI();
          Serial.println(rssi);

          if (Name != '\0')
          {
            Serial.println(Name);
          }

          String data2 = getValue(value, ':', 8);
          euuid = getValue(data2, ',', 0);
          if (euuid != '\0')
          {
            Serial.println(euuid);
          }
          if (millis() - lastPublishMillis > publishInterval)
          {
            publishData(Name, euuid, rssi);
            lastPublishMillis = millis();
          }

          if (!client.loop()) {
            mqttConnect();
          }
        }



      }

    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  wifiConnect();
  mqttConnect();
}

void loop() {
  // put your main code here, to run repeatedly:
  int scanTime = 5; //In seconds
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
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

void publishData(String Name, String euuid, float rssi)
{

  //Generating payload for publishing the values
  String payload = "{\"d\":{\"HALL\":";
  payload += "\"" "LH12" "\"";
  payload += ",""\"EUUID\":";
  payload +=  "\"" + euuid + "\"";
  payload += ",""\"RSSI\":";
  payload +=  rssi;
  payload += "}}";

  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }


}
