#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);


const char apn[] = "internet";//"internet"
const char user[] = "mms";
const char pass[] = "mms";

#define ORG "nojzs3"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "1234"
#define TOKEN "123456789"
#define EVENT "status"
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

 
//Tópico onde vamos postar os dados de temperatura e umidade (modemGSM32_gprs é o nome do dispositivo no Ubidots)
#define TOPIC "iot-2/evt/Data/fmt/json"
 
//id do dispositivo que pegamos no painel do Ubidots
#define DEVICE_ID "16841A0594"
 
//URL do MQTT Server
char MQTT_SERVER[] = ORG ".messaging.internetofthings.ibmcloud.com";
 
//Porta padrão do MQTT
#define MQTT_PORT 1883
 #define INTERVAL 10000
#include <TinyGPS++.h>    
#include <SoftwareSerial.h>
SoftwareSerial SerialGSM(6, 7); // RX, TX
TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(8,9);
int button=3;
TinyGsm modemGSM(SerialGSM);
TinyGsmClient gsmClient(modemGSM);
PubSubClient client(MQTT_SERVER, MQTT_PORT, gsmClient);
uint32_t lastTime = 0;


void setup() 
{
  Serial.begin(9600);
  SerialGSM.begin(9600);
    ss.begin(9600);
  setupGSM(); //Inicializa e configura o modem GSM
  connectMQTTServer(); //Conectamos ao mqtt server
  pinMode(button,INPUT);
  delay(2000);
}

void setupGSM()
{
  Serial.println("Setup GSM...");
  //Inicializamos a serial onde está o modem
 //SerialGSM.begin(9600, SERIAL_8N1, 4, 2, false);
 SerialGSM.begin(9600);
  delay(3000);
 
 
  Serial.println(modemGSM.getModemInfo());

  if (!modemGSM.waitForNetwork()) 
  {
    Serial.println("Failed to connect to network");
    delay(10000);
    modemGSM.restart();
    return;
  }
 
  //Conecta à rede gprs (APN, usuário, senha)
  if (!modemGSM.isGprsConnected())
  {
    Serial.println(F("Connecting to"));
    Serial.print(apn);
  if (!modemGSM.gprsConnect(apn, "", "")) {
    Serial.println("GPRS Connection Failed");
    delay(10000);
    modemGSM.restart();
    return;
  }
  else Serial.println(" Ok ");
  }
 
  Serial.println("Setup GSM Success");
}

void connectMQTTServer() {
  Serial.println("Connecting to MQTT Server...");
  //Se conecta ao device que definimos
  if (client.connect(clientId,authMethod,token)) {
    //Se a conexão foi bem sucedida
    Serial.println("Connected");
  } else {
    //Se ocorreu algum erro
    Serial.print("error = ");
    Serial.println(client.state());
    delay(10000);
    modemGSM.restart();
  }
}

void loop() 
{
  //Faz a leitura da umidade e temperatura
//  readDHT(); 
 
  //Se desconectou do server MQTT
  if(!client.connected())
  {
    //Mandamos conectar
    connectMQTTServer();
  }
 
   //Tempo decorrido desde o boot em milissegundos
  unsigned long now = millis();
 
  //Se passou o intervalo de envio
  if(now - lastTime > INTERVAL)
  {
    //Publicamos para o server mqtt
    publishMQTT();
    //Mostramos os dados no display
    //Atualizamos o tempo em que foi feito o último envio
    lastTime = now;
  }
}

void publishMQTT()
{
  //Cria o json que iremos enviar para o server MQTT
  String msg = createJsonString();
  Serial.print("Publish message: ");
  Serial.println(msg);
  //Publicamos no tópico
  int status = client.publish(TOPIC, msg.c_str());
  Serial.println("Status: " + String(status));//Status 1 se sucesso ou 0 se deu erro
}


String createJsonString() 
{ 
   Serial.println("---------------");

 int buttonval=digitalRead(button);
 Serial.println("button status");
 Serial.println(buttonval);
  delay(2000);
  if(buttonval==2){

  Serial.println(".............................");
  SerialGSM.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  SerialGSM.println("AT+CMGS=\"+919182053680\"\r"); // Replace x with mobile number
  delay(1000);
  SerialGSM.println("im in danger need help");// The SMS text you want to send
  delay(100);
  SerialGSM.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }
ss.listen();
  Serial.print("Latitude  : ");
  Serial.println(gps.location.lat(), 5);
  Serial.print("Longitude : ");
  Serial.println(gps.location.lng(), 5);
  smartDelay(1000); 
  String data = "{";
      data+="\"latitude\":";
      data+=String(gps.location.lat(), 5);
      data+=",";
      data+="\"longitude\":";
      data+=String(gps.location.lng(), 5);
      data+="}";
      SerialGSM.listen();
  return data;
}

static void smartDelay(unsigned long ms)                // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
