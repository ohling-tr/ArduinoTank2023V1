#include <Arduino.h>

/*************************************************************
  
 ______     __  __     __     __         _____        ______        ______   ______     __   __     __  __    .......(\_/) 
/\  == \   /\ \/\ \   /\ \   /\ \       /\  __-.     /\  __ \      /\__  _\ /\  __ \   /\ "-.\ \   /\ \/ /     ......( '_') 
\ \  __<   \ \ \_\ \  \ \ \  \ \ \____  \ \ \/\ \    \ \  __ \     \/_/\ \/ \ \  __ \  \ \ \-.  \  \ \  _"-.    ..../""""""""""""\======░   
 \ \_____\  \ \_____\  \ \_\  \ \_____\  \ \____-     \ \_\ \_\       \ \_\  \ \_\ \_\  \ \_\\"\_\  \ \_\ \_\     /"""""""""""""""""""""""\   
  \/_____/   \/_____/   \/_/   \/_____/   \/____/      \/_/\/_/        \/_/   \/_/\/_/   \/_/ \/_/   \/_/\/_/     \_@_@_@_@_@_@_@_/
                                                                                                              
  This sketch was based on existing sketches from the following:
    DFRobot:  https://www.dfrobot.com/blog-494.html
    jlmyra:   https://github.com/jlmyra/Arduino-Blynk-Joystick-4-Motor-Robot-Rover
 
 *************************************************************/

/* Tim Ohling
  V2 - replaces the ugly nested if/elif with Switch and case
  still nested, but generally prettier
*/
/* Tim Ohling
  2023 V1 - replace obsolete Blynk with RemoteXY
*/
/*
   -- New project --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.8 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.11.1 or later version;
     - for iOS 1.9.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__SOFTSERIAL
#include <SoftwareSerial.h>
#include <RemoteXY.h>
#include <AFMotor.h>

// RemoteXY connection settings 
#define REMOTEXY_SERIAL_RX 10
#define REMOTEXY_SERIAL_TX 11
#define REMOTEXY_SERIAL_SPEED 9600

#define EN_PIN 9

// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 27 bytes
  { 255,3,0,0,0,20,0,16,31,1,5,0,22,27,30,30,2,26,31,4,
  0,11,66,7,18,2,26 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int8_t joystick_1_x; // from -100 to 100  
  int8_t joystick_1_y; // from -100 to 100  
  int8_t slider_1; // =0..100 slider position 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

//*************************************************************

AF_DCMotor motorDriveRight(3, MOTOR3_A); // create motor #2, 64KHz pwm
AF_DCMotor motorDriveLeft(4, MOTOR4_A);
int state = 0;

//######### SETUP ######################################
void setup()
{
  //pinMode(EN_PIN, OUTPUT);
  //digitalWrite(EN_PIN, LOW);

  Serial.begin(9600);
  RemoteXY_Init (); 

  delay(1000);
    
  Serial.print("Waiting for connections...");
  Serial.println(RemoteXY.connect_flag);
  Serial.println("Adafruit Motorshield v1 - DC Motor");

  // Set the speed to start, from 0 (off) to 255 (max speed)
  motor_set_speed(0,0);
}

//**********VOID LOOP**********
void loop()
{
    RemoteXY_Handler (); //catch remote app structure
    getSpeed(RemoteXY.slider_1);
    getJoystickXY(RemoteXY.joystick_1_x, RemoteXY.joystick_1_y);
}
//**********END VOID LOOP**********

//*******************************************************
// common joystick logging to serial (use serial monitor on PC)
// tro 6/7/2018
//
void joystick_log(String strJoystick, String strDirection, int x_value, int y_value) {
  Serial.print("JOYSTICK: ");
  Serial.print(strJoystick);
  Serial.print(" DIRECTION: ");
  Serial.print(strDirection);
  Serial.print("  x_direction: ");
  Serial.print(x_value);
  Serial.print("  y_direction: ");
  Serial.println(y_value);
}

//*******************************************************
// common motor run passing direction values (defined as integers)
// tro 6/7/2018
//
void motor_run(int leftMotorDir, int rightMotorDir) {
  motorDriveRight.run(rightMotorDir);
  motorDriveLeft.run(leftMotorDir); 
}

void motor_set_speed(int leftSpeed, int rightSpeed) {
  motorDriveRight.setSpeed(rightSpeed);
  motorDriveLeft.setSpeed(leftSpeed);  
}

//**********Blynk Subroutines**********

//**********Set the Motor Speed**********
// This function sets the motor speed via Blynk Slider Object
// the variable pinValue ranges from 0-255
// On the phone app set the Slider Ouput to Virtual V2
// This function will set the speed

//BLYNK_WRITE(V2)
void getSpeed(int8_t speed)
{
  int pinValue = map(speed, 0, 100, 0, 255); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V2 Slider value is: ");
  Serial.println(pinValue);
  motor_set_speed(pinValue, pinValue);
}
//**********END Set the Motor Speed**********


//**********Translate the Joystick Position to a Rover Direction**********
//
//This function translates the joystick movement to a Rover direction.
//Blynk Joysick is centered at y=128, x=128 with a range of 0-255. Thresholds set the Joystick 
//sensitivity. These are my adjustments for my touch, you may need something different. Making
//the range too tight will make the rover hard to control. Note: Y values greater than 128 will
//drive the motors FOWARD. Y values less than 128 will drive the motorS in REVERSE. The Rover will
//turn in the direction of the "slow" or unpowered (RELEASE) wheels.
//
//  Joystick Movement along x, y Axis
// (Inside the * is the Threshold Area)
//            y=100--(y_position=100, x_position=0; y_direction=+1, x_direction=0)
//           * | *
//           * | *
//           * | *
//   ********* | *********
//x=-100---------------------x=100--(y_position=0, x_position=0; y_direction=0, x_direction=0)
//   ********* | *********
//           * | *
//           * | * (Inside the * is the Threshold Area)
//           * | *
//            y=-100--(y_position=-100, x_position=0; y_direction=-1, x_direction=0)

//BLYNK_WRITE(V0)
void getJoystickXY(int8_t joystickX, int8_t joystickY)
{
  const int X_THRESHOLD_LOW = 108; //X: 50 - 10
  const int X_THRESHOLD_HIGH = 148; //X: 10 + 10   

  const int Y_THRESHOLD_LOW = 108;
  const int Y_THRESHOLD_HIGH = 148;
      
  //int x_position = map(joystickX, -100, 100, 0, 255);  //Read the Joystick x Position -100-100
  //int y_position = map(joystickY, -100, 100, 0, 255);  //Read the Joystick y Position -100-100
  int x_position = joystickX + 128;
  int y_position = joystickY + 128;

  int x_direction;  //Variable for Direction of Joystick Movement: x= -1, 0, 1
  int y_direction;  //Variable for Direction of Joystick Movement: y= -1, 0, 1
 
  Serial.print("x_position: ");
  Serial.print(x_position);
  Serial.print("  y_position: ");
  Serial.println(y_position);

//Determine the direction of the Joystick Movement

  x_direction = 0;
  y_direction = 0;

  if (x_position > X_THRESHOLD_HIGH) {
    x_direction = 1;
  } else if (x_position < X_THRESHOLD_LOW) {
    x_direction = -1;
  }
  if (y_position > Y_THRESHOLD_HIGH) {
    y_direction = 1;
  } else if (y_position < Y_THRESHOLD_LOW) {
    y_direction = -1;
  }
//if x and y are within the threshold their values then x_direction = 0 and y_direction = 0

//Move the Rover (Rover will move in the direction of the slower wheels)
//0,0(Stop); 0,1(Forward); 0,-1(Backward); 1,1(Right up diagonal); 1,0(Right); 1,-1(Right down diagonal);
//-1,0(Left); -1,1(Left up diagonal); -1,-1(Left down diagonal)

    switch (x_direction) {
// x = -1 
    case -1 :
        switch (y_direction) {
// y = -1 Back Diagonal Left
        case -1 :
            joystick_log("left-down", "BACKWARD SOFT LEFT", x_direction, y_direction);
            motor_run(RELEASE, BACKWARD);
            break;
// y = 0 Left on x axis 
        case 0 :    
            joystick_log("left", "HARD LEFT (ROTATE COUNTER-CLOCKWISE)", x_direction, y_direction);
            motor_run(BACKWARD, FORWARD);
            break;
// y = 1 Forward Diagonal Left   
        case 1 :
            joystick_log("left-up", "FORWARD SOFT LEFT", x_direction, y_direction);
            motor_run(RELEASE, FORWARD);
            break;
        }
        break;
// x = 0 centered        
    case 0 :
// y = -1 Backward
        switch (y_direction) {
        case -1 :
            joystick_log("down", "BACKWARD", x_direction, y_direction);
            motor_run(BACKWARD, BACKWARD);
            break;
// y = 0 Stop
        case 0 : 
            joystick_log("centered", "STOP", x_direction, y_direction);
            motor_run(RELEASE, RELEASE);
            break;
// y = 1 Forward 
        case 1 :
        //y_direction == 1
            joystick_log("up", "FORWARD", x_direction, y_direction);
            motor_run(FORWARD, FORWARD);
            break;
        }
    break;
// x_direction == 1
    case 1 :
// y = -1 Backward Diagonal Right
        switch (y_direction) {
            case -1 : 
                joystick_log("right-down", "BACKWARD SOFT RIGHT", x_direction, y_direction);
                motor_run(BACKWARD, RELEASE);
                break;       
// y = 0 Right on x-axis
            case 0 :
                joystick_log("right", "HARD RIGHT (CLOCKWISE SPIN)", x_direction, y_direction);
                motor_run(FORWARD, BACKWARD);
                break;
// y = 1 Forward Diagonal Right
            case 1 : 
                joystick_log("right-up", "SOFT RIGHT", x_direction, y_direction);
                motor_run(FORWARD, RELEASE); 
                break;
        }
        break;
    }
}