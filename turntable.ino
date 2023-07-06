#include <AccelStepper.h> 


//====================[str table]==============================================
#define STR_LED_INTENSITY       0
#define STR_MOTOR_SPEED         1
#define STR_MOTOR_ACCELERATION  2
#define STR_MOTOR_STEPCOUNT     3
#define STR_MOTOR_CW            4
const String stringTable[5] = {"LED.INTENSITY", "MOTOR.SPEED", "MOTOR.ACCELERATION", "MOTOR.STEPCOUNT", "MOTOR.CW"};
int nsteps = 1;

//====================[ctrl variables]=========================================
typedef struct ControlVars
{
    const String version;
    String  inputString;        // holds incoming data
    boolean stringComplete;     // to check whether the incoming data is complete
    volatile boolean configuring;
    volatile boolean inInterrupt;
} ControlVars;

//====================[PIN def]=================================================
const int PIN_LED = 2;
const int PIN_SERIAL_INTERRUPT = 3;         // Interruption on PIN3, push-button connected to pull-up
const int PIN_SW = 4;          // micro switch pin pull-down
const int PIN_MOTORPOWER_FEEDBACK = 5;

//====================[LED]=====================================================
typedef struct _LEDProperties
{
    volatile boolean    state;
    int                 intensity;  // 80
} LEDProperties;

//====================[MOTOR]===================================================
enum MotorStates
{
    UNCONFIGURED,   
    STANDBY,              // motor stops
    RCW,                  // turns clock-wise direction
    RCCW,                 // turns anticlock-wise direction
    HOME                  // motor goes home
};

typedef struct _MotorInfo
{
    const int       stepsPerCycle;
    const uint8_t   intfc;
    const String    type;
} MotorInfo;

typedef struct _MotorProperties
{
    boolean clockwise;
    float   speed;
    float   acceleration;
    float   prev_acceleration;      // cache until we need it so we avoid calling stepper.setAcceleration() unless needed
    int     stepCount;
    int   currentStep;
    volatile MotorStates state;
} MotorProperties;




//=============================================================================




boolean CheckDrivePowered()
{
    int read = analogRead(PIN_MOTORPOWER_FEEDBACK);
    float voltage = 5.0f*(float)read/1024.00f;
    
    #if DEBUG
      DebugPrintParamVal("Voltage", voltage);
    #endif
      return voltage > 4.0f;
}




//********************************************************************************* VAIRABLES
  ControlVars controller  = {"1.2.0", "", false, false, false };

  
  LEDProperties led;

  void SetLedState(boolean ledOn)
  {
    int pwmvalue =  ledOn ? ((int)((float)led.intensity / 100 * 255)) : 0;
    analogWrite(PIN_LED, pwmvalue);
    led.state = ledOn;
  }
  int val_SW = 0;                 //SW (pull-down)
  #define MotorInterfaceType 8 
  #define IN1  8        // IN1 on the ULN2003
  #define IN2  9        // IN2 on the ULN2003
  #define IN3  10       // IN3 on the ULN2003
  #define IN4  11       // IN4 on the ULN2003
  const int nsteps_full = 2038;//Steps per revolution
  const int nsteps_quarter = int(nsteps_full/4.0);
  MotorProperties motor;
  const MotorInfo motorInfo = { nsteps_full, MotorInterfaceType, "M_ULN2003" };
  AccelStepper stepper = AccelStepper(motorInfo.intfc,IN1,IN2,IN3,IN4);   
  
//*********************************************************************************** SETUP
void setup()
{
  //pinMode(PIN_DRIVE_DISABLED, OUTPUT);
  pinMode(PIN_SERIAL_INTERRUPT, INPUT);
  pinMode(PIN_SW, INPUT);
  
  
  
  
  SetLedState(false);
  led.intensity = 100;
  motor.currentStep = 0;
  //digitalWrite(PIN_DRIVE_DISABLED, HIGH);
  //stepper.setEnablePin(PIN_DRIVE_DISABLED);
  stepper.setPinsInverted(false, false, true);
  //stepper.enableOutputs();
  Serial.begin(115200);
  Serial.println("READY");
   
}

// this is the actual loop that is ran in the arduino
void loop()
{
// Three cases are defined here: HOME, motor turns anticlock-wise direction until it reaches position zero (home)
// RCCW, motor turns anticlock-wise direction and RCW motor turns clock-wise direction
  
  
   switch (motor.state)
    {
    case HOME:
    val_SW = digitalRead( PIN_SW );
    delay(100);
    while (val_SW == HIGH)
    {
      stepper.moveTo(-nsteps);
      stepper.runToPosition();
      delay(1);
      val_SW = digitalRead( PIN_SW ); 
     }
    motor.state = STANDBY;

    case RCCW:
        //stepcount = - stepcount;
        // don't put a break here!
    case RCW:
      if (motor.currentStep > 0)
      {
        stepper.moveTo(+nsteps);
        stepper.runToPosition();
        motor.currentStep--;
      }
      if (motor.currentStep == 0)
        {
          motor.state = STANDBY;
        }
      break;
      default:
      break;
      }
}

void serialEvent()
{
    //serial commands not related to configuration but HOME/GO/STOP/RESET...
    if (!controller.configuring)
    {
      if (controller.inInterrupt || controller.configuring)
      {
        return;
      }
      
      controller.inInterrupt = true;
      interrupts();
      while (!Serial.available());
      char data = (char)toupper(Serial.read());
      // Echo data back to user ;-)
      Serial.print("<"); //-----------------------------------------[  "<data>"  ]
      Serial.print(data);
      Serial.print(">");
    
      switch (data)
      {
      case 'R':
        motor.currentStep = 0;
        break;
      case 'S':
        motor.currentStep = 0;
        break;
      case '?':
        PrintControllerState();
        break;
      case 'G':
        if (motor.state == STANDBY){
          if (motor.clockwise) {
            motor.state = RCW;
          }
          else {
            motor.state = RCCW;
          }
          motor.currentStep = motor.stepCount;
        }
        break;
      case 'C':
        motor.currentStep = 0;
        motor.state = UNCONFIGURED;
        controller.configuring = true;
        break;
      case 'H':                   // New case H (home) is defined in which the motor returns to position zero 
        motor.state =  HOME;        // call the case HOME in the arduino loop
        break;
      case 'I':
        SetLedState(true);
        break;
      case 'X':
        SetLedState(false);
        break;
      default:
        Serial.print("ERR:01");
        break;
      }
      Serial.println();
      controller.inInterrupt = false;
      return;
    }

    String configString = Serial.readString();
    int length = configString.length();
    // the line must end with ------------------------------------> [  !  ]
    // because we subtract 1 afterwards
    if ((length == 0) || (configString[length-1] != '!'))
    {
        Serial.println("ERR:02");
        controller.configuring = false;
        return;
    }

    String parameter = "";
    String value     = "";
    boolean switchToValue = false;
    int counter = 0;
    while (counter < length - 1)        // no need to include the ! at the end
    {
        char character = configString[counter];
        ++counter;

        if (character == ' ') {
            switchToValue = true;
            continue;
        }

        if (switchToValue) {
            value += character;
        }
        else{
            parameter += character;
        }
    }

    boolean paramValid = true;
    parameter.toUpperCase();

    if (parameter == stringTable[STR_MOTOR_SPEED]){
      motor.speed = (float)atof(value.c_str());
    }
    else if (parameter == stringTable[STR_MOTOR_ACCELERATION]){
      motor.acceleration = (float)atof(value.c_str());
    }
    else if (parameter == stringTable[STR_MOTOR_STEPCOUNT]){
      motor.stepCount = atoi(value.c_str());
    }
    else if (parameter == stringTable[STR_MOTOR_CW]){
      motor.clockwise = (boolean)(atoi(value.c_str()));
      if (motor.clockwise){
        stepper.moveTo(+nsteps);
      }
      else{
        stepper.moveTo(-nsteps);
      }
    delayMicroseconds(2);
    }
    else if (parameter == stringTable[STR_LED_INTENSITY]){
      led.intensity = atoi(value.c_str());
      if (led.intensity > 100){
        led.intensity = 100;
        }
      else if (led.intensity < 0){
        led.intensity = 0;
      }
    }
    else{
        paramValid = false;
    }

    if (paramValid){
        Serial.println("OK");
    }
    else{
        Serial.println("ERR:03");
    }


    controller.configuring = false;
    if (motor.speed > 0.0f && motor.acceleration > 0.0f && motor.stepCount > 0)
    {
        motor.state = STANDBY;
    }
}

void PrintControllerState()
{
    // Constant values
    Serial.print("Controller.Version: V");
    Serial.print(controller.version);
    Serial.print(";Motor.Type: ");
    Serial.print(motorInfo.type);
    Serial.print(";Motor.StepsPerCycle: ");
    Serial.print(motorInfo.stepsPerCycle);
    // Motor specific values
    Serial.print(";Motor.Powered: ");
    Serial.print(CheckDrivePowered());
    Serial.print(";Motor.Status: ");
    Serial.print(motor.state);
    Serial.print(";Motor.CurrentPosition: ");
  Serial.print(motor.currentStep);
    // Motor user-settable values
    Serial.print(";" + stringTable[STR_MOTOR_SPEED] + ": ");
    Serial.print(motor.speed, 4);
    Serial.print(";" + stringTable[STR_MOTOR_ACCELERATION] + ": ");
    Serial.print(motor.acceleration, 4);
    Serial.print(";" + stringTable[STR_MOTOR_CW] + ": ");
    Serial.print(motor.clockwise);
    Serial.print(";" + stringTable[STR_MOTOR_STEPCOUNT] + ": ");
    Serial.print(motor.stepCount);
    // LED values
    Serial.print(";Led.Status: ");
    Serial.print(led.state);
    Serial.print(";" + stringTable[STR_LED_INTENSITY] + ": ");
    Serial.print(led.intensity);
}

