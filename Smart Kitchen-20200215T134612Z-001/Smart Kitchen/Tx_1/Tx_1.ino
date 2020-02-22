//This sketch is from a tutorial video for networking more than two nRF24L01 tranciever modules on the ForceTronics YouTube Channel
//the code was leverage from the following code http://maniacbug.github.io/RF24/starping_8pde-example.html
//This sketch is free to the public to use and modify at your own risk

#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/

const int pinCE = 7; //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = 8; //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out


// defines pins numbers
const int trigPin = 2;
const int echoPin = 4;
// defines variables
int duration;
byte distance;

RF24 wirelessSPI(pinCE, pinCSN); // Create your nRF24 object or wireless SPI connection

const uint64_t wAddress = 0xB00B1E50D2LL;              // Pipe to write or transmit on

void setup()
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
  Serial.begin(9600);   //start serial to communicate process
  randomSeed(analogRead(0)); //create unique seed value for random number generation
  wirelessSPI.begin();            //Start the nRF24 module
  wirelessSPI.openWritingPipe(wAddress);        //open writing or transmit pipe
  wirelessSPI.stopListening(); //go into transmit mode
}


void loop()
{
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  //distance = duration * 0.034 / 2;
  distance = 25;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);


  wirelessSPI.write( &distance, 1 );
  wirelessSPI.stopListening(); //go back to transmit mode
  delay(1000);

}
