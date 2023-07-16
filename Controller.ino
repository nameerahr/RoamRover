#include <SPI.h> // Handles SPI communicaton 
#include <nRF24L01.h> // Control nRF module
#include <RF24.h> // Control nRF module

// ------------ PINS -------------

// DIGITAL
int NRF_CE = 2;
int NRF_CSN = 3;
int NRF_MOSI = 11;
int NRF_MISO = 12;
int NRF_SCK = 13;
int LED = 8;
int switchSelect = 10;

// ANALOG
int joyY = A0;
int joyX = A2;

// ------------ VARIABLES -------------

int xyPosition[2];

RF24 communication(NRF_CE, NRF_CSN); // CE, CSN

const byte address[6] = "88888";

void setup() {
    // Initialize serial communication
    Serial.begin(9600);

    // nRF communication setup
    communication.begin();
    communication.openWritingPipe(address); // set address
    communication.stopListening(); // set as transceiver

    // Output pins
    pinMode(LED, OUTPUT);

    // Input pins
    pinMode(joyX, INPUT);
    pinMode(joyY, INPUT);
    pinMode(switchSelect, INPUT);
}

void loop() {

  if(digitalRead(switchSelect) == HIGH){
    digitalWrite(LED, HIGH);
    readJoystick(); // send out joystick positions to robot through nRF    
    delay(100);
  }else{
    digitalWrite(LED, LOW); // turn off LED and don't send joystick positions to robot
  }

}

void readJoystick(){

  xyPosition[0] = analogRead(joyX);
  xyPosition[1] = analogRead(joyY);

  Serial.print("X val: ");
  Serial.print(xyPosition[0]);
  Serial.print(".    ");
  Serial.print("Y val: ");
  Serial.println(xyPosition[1]);


  if(digitalRead(switchSelect) == HIGH){
    communication.write(&xyPosition, sizeof(xyPosition));
    delay(100);
  }
}