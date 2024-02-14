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



#define mRev true
#include <AccelStepper.h>
#define MotorInterfaceType 4 // Full 4 wires (2-phase)
#define MP1  8  //M1A
#define MP2  9  //M1B
#define MP3 10  //M2A
#define MP4 11  //M2B

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
  pinMode(4,INPUT);//sw4
  pinMode(3,INPUT);//sw3
  stepper.setCurrentPosition(0.0);
};

void loop() {
  int read3 = digitalRead(3);
  int read4 = digitalRead(4);
  if (mRev){
    while(read3==LOW){
      stepper.move(rev); //Set the target motor position (i.e. turn motor for 3 full revolutions)
      stepper.runToPosition(); // Run the motor to the target position 
      stepper.moveTo(0); //Set the target motor position (i.e. turn motor for 3 full revolutions)
      stepper.runToPosition();
      read3 = digitalRead(3);
      Serial.println(stepper.currentPosition());
      delay(1000);
  
    }
    while(read4 ==HIGH){
      stepper.moveTo(0); //Set the target motor position (i.e. turn motor for 3 full revolutions)
      stepper.runToPosition(); // Run the motor to the target position 
      read4 = digitalRead(4);
      Serial.println(stepper.currentPosition());
      delay(1000);
    }
  }
   if(read3 == HIGH && read4==HIGH)
  {
    digitalWrite( 8,LOW);
    digitalWrite( 9,LOW);    
    digitalWrite(10,LOW);    
    digitalWrite(11,LOW);
  }
}
