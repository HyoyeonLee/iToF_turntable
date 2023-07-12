//Arduino Code - Move from A to B by a specific number of steps with set speed and acceleration

// Include the Arduino Stepper.h library:
#include <AccelStepper.h> //Include the AccelStepper library

// Define the motor pins:
#define MP1  8 // IN1 on the ULN2003
#define MP2  9 // IN2 on the ULN2003
#define MP3  10 // IN3 on the ULN2003
#define MP4  11 // IN4 on the ULN2003

#define MotorInterfaceType 8 // Define the interface type as 8 = 4 wires * step factor (2 for half step)
#define deg0 0
#define deg90 900
#define deg180 1940
#define deg270 2940
#define deg360 3980
AccelStepper stepper = AccelStepper(MotorInterfaceType, MP1, MP3, MP2, MP4);//Define the pin sequence (IN1-IN3-IN2-IN4)
const int SPR = 1035*4;//Steps per revolution

int unit = 1035;
//int unit=5;


int inv(int steps)
{
  return -steps;
}
void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(500);//Set the maximum motor speed in steps per second
  stepper.setAcceleration(50);//Set the maximum acceleration in steps per second^2
  pinMode(2,OUTPUT);
  pinMode(4,INPUT);
  pinMode(3,INPUT);
  stepper.setCurrentPosition(0.0);
}

void loop() {
  int read = digitalRead(4);
  int read2 = digitalRead(3);
  //Serial.println(read);
  while(read==HIGH)
  {
    stepper.move(inv(unit)); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read = digitalRead(4);
    Serial.println(inv(stepper.currentPosition()));
  }
  while(read2 ==LOW)
  {
    stepper.move(inv(-unit)); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read2 = digitalRead(3);
    Serial.println(inv(stepper.currentPosition()));
  }
  Serial.println(inv(stepper.currentPosition()));
  delay(100);

 // digitalWrite(2,LOW);
}
