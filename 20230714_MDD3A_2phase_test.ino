/* 2023.07.17 
  --<HW>--------------------------------------
    [Motor] 2phase motor (42BH2A47-174) 
      - power : constant current of 1.7[A]
      - 1[rev] = (360/1.8)*60 = 12,000 [steps] 
          - resolution : 1.8 [deg/step]
          - reduction ratio : 1:60
    [Rotating Stage] Colibri RTS9060B
    [Driver] MDD3A

  --<SW>--------------------------------------
    [HEAT] Apply LOW to all four wires when stopped.
    [CW only] Except for homing, forbid CCR motion.
    [add free jogging mode with sw5]


*/



#define mUnit false
#define mRev true
#include <AccelStepper.h>
#define M1A  8 
#define M1B  9 
#define M2A  10 
#define M2B  11 
#define MotorInterfaceType 4 // Full 4 wires (2-phase)

AccelStepper stepper = AccelStepper(MotorInterfaceType, MP1, MP2, MP3, MP4);//Define the pin sequence (IN1-IN3-IN2-IN4)
const int SPR = 12000;//Steps per revolution  2phase stepper stage(gear ratio :60)
const int rev = int(round(SPR/4.0));

//int unit = 1035;
int unit=10;


int inv(int steps)
{
  return -steps;
}
void setup() {
  Serial.begin(9600);
  stepper.setMaxSpeed(100);//Set the maximum motor speed in steps per second
  stepper.setAcceleration(50);//Set the maximum acceleration in steps per second^2
  pinMode(2,OUTPUT);
  pinMode(4,INPUT);
  pinMode(3,INPUT);
  stepper.setCurrentPosition(0.0);
}

void loop() {
  int read = digitalRead(4);
  int read2 = digitalRead(3);
  if (mRev){
  while(read==HIGH)
  {
    stepper.move(rev); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read = digitalRead(4);
    Serial.println(stepper.currentPosition());
    delay(1000);
  }
  while(read2 ==LOW)
  {
    stepper.moveTo(0); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read2 = digitalRead(3);
    Serial.println(stepper.currentPosition());
    delay(1000);
  }}
  if(read == LOW && read2==HIGH)
  {
    digitalWrite( 8,LOW);
    digitalWrite( 9,LOW);    
    digitalWrite(10,LOW);    
    digitalWrite(11,LOW);
  }
  if (mUnit){
   while(read==HIGH)
  {
    stepper.move(unit); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read = digitalRead(4);
    Serial.println(stepper.currentPosition());
    delay(1000);
  }
  while(read2 ==LOW)
  {
    stepper.move(-unit); //Set the target motor position (i.e. turn motor for 3 full revolutions)
    stepper.runToPosition(); // Run the motor to the target position 
    read2 = digitalRead(3);
    Serial.println(stepper.currentPosition());
    delay(1000);
  }}
  
}
