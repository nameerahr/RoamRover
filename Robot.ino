#include <Servo.h>
#include <SPI.h> // Handles SPI communicaton 
#include <nRF24L01.h> // Control nRF module
#include <RF24.h> // Control nRF module

// ------------ PINS -------------

// ANALOG
int servoPin = A0;
int sonicEcho = A1;
int sonicTrig = A2;
int IRRight = A4;
int IRLeft = A5;

// DIGITAL
int motorEnB = 3;
int motorIn4 = 2;
int motorIn3 = 4;
int motorIn2 = 5;
int motorIn1 = 7;
int motorEnA = 6;
int NRF_CSN = 9;
int NRF_CE = 10;
int NRF_MOSI = 11;
int NRF_MISO = 12;
int NRF_SCK = 13;

// ------------ VARIABLES -------------

Servo servo;

RF24 communication(NRF_CE, NRF_CSN);
const byte address[6] = "88888"; // Address used to communicate between the nRF modules
int xyPosition[2];
int prevState = 0; // 1 = manual, 2 = obstacle avoiding automatic

void setup() {

    // Initialize serial communication
    Serial.begin(9600);

    // Output pins
    pinMode(sonicTrig, OUTPUT);
    pinMode(motorEnB, OUTPUT);
    pinMode(motorIn4, OUTPUT);
    pinMode(motorIn3, OUTPUT);
    pinMode(motorIn2, OUTPUT);
    pinMode(motorIn1, OUTPUT);
    pinMode(motorEnA, OUTPUT);
    pinMode(servoPin, OUTPUT);
    servo.attach(servoPin);

    // Input pins
    pinMode(IRLeft, INPUT);
    pinMode(IRRight, INPUT);
    pinMode(sonicEcho, INPUT);

    // Servo
    servo.write(95);
    delay(1000);

    // nRF communication
    communication.begin();
    communication.openReadingPipe(0, address); // Set the address through which communication will occur, using pipe 0
    communication.startListening(); // Set robot as receiver

    delay(3000);
}

void loop() {

  if(communication.available()){
    manual();
    prevState = 1;
  }else if(prevState == 1){
    delay(300); // Delay between joystick communication transmission
    prevState = 2;
  }else{
    obstacleAvoid();
    prevState = 2;
  }

  // lineFollowing();
}

void obstacleAvoid(){

    // Motor speeds
    analogWrite(motorEnA, 100);
    analogWrite(motorEnB, 100);

    moveForward();

    if(readDistance() < 23 ){
      stop();
      servoScan();
    }
    
    delay(10);
}

void manual(){
  
  communication.read(xyPosition, sizeof(xyPosition));

  Serial.print("Data[0]: ");
  Serial.print(xyPosition[0]);
  Serial.print("      Data[1]: ");
  Serial.println(xyPosition[1]);

  if(readDistance() > 20){
    if(xyPosition[0] < 450 && (xyPosition[1] > 450 && xyPosition[1] < 550)){
      analogWrite(motorEnA, 100);  
      analogWrite(motorEnB, 100); 

      moveForward();
    }else if(xyPosition[1] < 450){
      analogWrite(motorEnA, 120);  
      analogWrite(motorEnB, 120); 

      turnRight();
    }else if(xyPosition[1] > 550){ 
      analogWrite(motorEnA, 120);  
      analogWrite(motorEnB, 120); 

      turnLeft();
    }else if(xyPosition[0] > 550 && (xyPosition[1] > 450 && xyPosition[1] < 550)){ 
      analogWrite(motorEnA, 100);  
      analogWrite(motorEnB, 100); 

      moveBackward();
    }else{
      stop();
    }
  }else{   // If forward distance < 20, only allow backup until front is clear 
    if(xyPosition[0] > 550 && (xyPosition[1] > 450 && xyPosition[1] < 550)){ 
      moveBackward();
    }else{
      stop();
    }
  }
}

void lineFollowing(){

    // Motor speeds
    analogWrite(motorEnA, 150);
    analogWrite(motorEnB, 150);

    if(digitalRead(IRLeft) == 1 && digitalRead(IRRight) == 1){ // Both IR sensors on white
      moveForward();
    }

    if(digitalRead(IRLeft) == 1 && digitalRead(IRRight) != 1){ // Right IR sensor is on black line
      turnRight();
      delay(100);
    }

    if(digitalRead(IRLeft) != 1 && digitalRead(IRRight) == 1){ // Left IR sensor is on black line
      turnLeft();
      delay(100);
    }
}

void servoScan(){

    // Check right side
    servo.write(0);
    delay(200); // Wait for ultrasonic sensor to get in position before reading distance
    int rightDistance = readDistance();
    delay(1000); 

    // Check left side
    servo.write(200); 
    delay(400); // Wait for ultrasonic sensor to get in position before reading distance
    int leftDistance = readDistance();
    delay(1000); 

    // Face ultrasonic sensor forward
    servo.write(95); 
    delay(1000);

    if(leftDistance < rightDistance){
      analogWrite(motorEnA, 220);  
      analogWrite(motorEnB, 220);

      turnRight();
      delay(870);
    }else{
      analogWrite(motorEnA, 220);
      analogWrite(motorEnB, 220);

      turnLeft();
      delay(950);
    }
}

int readDistance(){

    digitalWrite(sonicTrig, LOW);
    delayMicroseconds(2); // Wait to avoid serial monitor collision
 
    // Turn on the Trigger to generate pulse for 10 microseconds
    digitalWrite(sonicTrig, HIGH); 
    delayMicroseconds(10);
    digitalWrite(sonicTrig, LOW);
  
    int duration = pulseIn(sonicEcho, HIGH); // Time (in microseconds) for the pulse to reach the receiver (pulse reflected off object)
    int distance = duration/2 * 0.0344 ; // Calculate distance (in cm) using time (time/2 * speed of sound in cm/microsecond)

    return distance;
}

void moveForward(){

  // Right side motors
  digitalWrite(motorIn4, LOW); // Backward motor
  digitalWrite(motorIn3, HIGH); // Forward motor

  // Left side motors
  digitalWrite(motorIn2, HIGH); // Forward motor
  digitalWrite(motorIn1, LOW); // Backward motor
}

void moveBackward(){

  // Right side motors
  digitalWrite(motorIn4, HIGH);
  digitalWrite(motorIn3, LOW);

  // Left side motors
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn1, HIGH);
}

void stop(){

  // Right side motors
  digitalWrite(motorIn4, LOW);
  digitalWrite(motorIn3, LOW);

  // Left side motors
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn1, LOW);
}

void turnRight(){

  // Right side motors 
  digitalWrite(motorIn4, LOW); 
  digitalWrite(motorIn3, HIGH); 

  // Left side motors
  digitalWrite(motorIn2, LOW); 
  digitalWrite(motorIn1, HIGH); 
}

void turnLeft(){ 
  
  // Right side motors 
  digitalWrite(motorIn4, HIGH);
  digitalWrite(motorIn3, LOW);

  // Left side motors
  digitalWrite(motorIn2, HIGH);
  digitalWrite(motorIn1, LOW); 
}
