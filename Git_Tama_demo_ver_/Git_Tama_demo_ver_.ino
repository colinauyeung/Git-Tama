#include <FastLED.h>
#include <Servo.h>

#define NUM_LEDS 16 //define the number of the leds
#define DATA_PIN 3  //define the pin for the leds

//Variables for reading from the serial port
//Buffer setup variables
byte incomingByte;
int bytecount = 0;

//Store the user and commit size recieved from serial
int user = 0;
int size = 1;

//the current positions of each of the servos for firing
int servo2pos = 180;
int servo1pos = 85;

//Are either of the servos reversing
boolean ser1rev = false;
boolean ser2rev = false;

//Servo timing count
int servocount = 0;

//animation counters
//Green leds
int ani1 = 0;
//Blue leds
int ani2 = 0;

//Animation timing counter
int anicount = 0;

//Denotes the leds in each rows of the led matrix
int rows[4][4] = { {0,7,8,15}, {1,6, 9, 14}, {2, 5, 10, 13}, {3, 4, 11, 12} };

//Firing Queue for servo1
int arr1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

//Front and back of the queue
int front1 = 0;
int back1 = 0;

//Firing Queue for servo2
int arr2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

//Front and back of the queue
int front2 = 0;
int back2 = 0;

//The valid angles for each of the ramps
int ramp2angles[3] = {90, 115, 140};
int ramp1angles[3] = {170, 150, 130};

Servo servo1;
Servo ramp1;
Servo servo2;
Servo ramp2;
CRGB leds[NUM_LEDS];

//array for each the current color of each led row
int ledcheck[4] = {0, 0, 0, 0};

void setup() {

  //Attach servo 1 to pin 9 and set it to it's intial position
  servo1.attach(9);
  servo1.write(servo1pos);

  //Attach servo 2 to pin 10 and set it to it's intial position
  servo2.attach(10);
  servo2.write(servo2pos);

  //attach the ramps to pins
  ramp1.attach(6);
  ramp2.attach(11);

  //set up leds
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);

  //set up serial
  Serial.begin(9600);

  //intialize the leds to black
  for(int x = 0; x<16; x++){
    leds[x] = CRGB::Black;
  }
  for(int x = 0; x<4; x++){
    ledcheck[x] = CRGB::Black;
  }
  FastLED.show();

}

void loop() {

  //While there are serial inputs, read data from the serial
  while (Serial.available() > 0) {

    //Read a byte from serial
    incomingByte = Serial.read();

    //if this is the first byte recieved
    switch(bytecount){

      //Set byte as the user
      case 0:
        switch(incomingByte){
          case 48:
            user = 0;
            break;
          case 49:
            user =1;
            break;
          default:
            user = 0;
            break;
        }
        bytecount = bytecount + 1;
        break;

      //if this is the second byte recieved
      case 1:

        //set byte as the commit size
        switch(incomingByte){
          case 48:
            size = 1;
            break;
          case 49:
            size = 2;
            break;
          case 50:
            size = 3;
            break;
          default:
            size =1;
            break;
        }
        bytecount = bytecount + 1;
        break;

      //If this is the third byte recieved
      case 2:

        //Depending on the user
        switch(user){

          //if user 1
          case 0:

            //Push the commit size on to the firing queue 1
            if(front1 == back1 && arr1[back1] != 0){
              break;
            }
            arr1[back1] = size;
            if(back1 == 9){
              back1 = 0;
            }
            else{
              back1 = back1 + 1;
            }
            break;

          //if user 2
          case 1:
            //Push the commit size on to the firing queue 2
            if(front2 == back2 && arr2[back2] != 0){
              break;
            }
            arr2[back2] = size;
            if(back2 == 9){
              back2 = 0;
            }
            else{
              back2 = back2 + 1;
            }
            break;
        }

        //reset user and size
        user = 0;
        size = 1;

        //reset byte count
        bytecount = 0;

        //Print firing queue to check
        for(int i = 0; i<10 ; i++){
          Serial.print(arr1[i]);
          Serial.print(", ");
        }
        Serial.println();
        for(int i = 0; i<10 ; i++){
          Serial.print(arr2[i]);
          Serial.print(", ");
        }
        Serial.println();
        Serial.println();
        break;
    }

  }

  //increment the servo timer
  servocount = servocount + 1;

  //Every 5 intervals
  if(servocount == 5){

    //If the servo is in it's neutral state
    if(servo1pos == 85){

      //and there's something in the firing queue
      if(arr1[front1] != 0){

        //Move the ramp to the value in the firing queue
        ramp1.write(ramp1angles[arr1[front1]-1]);

        //pop the value
        arr1[front1] = 0;
        if(front1 == 9){
          front1 = 0;
        }
        else{
          front1 = front1 + 1;
        }

        //Start the led animation
        ani1 = 1;

        //Start moving the servo
        servo1pos = 90;
        servo1.write(servo1pos);
      }
    }

    //Otherwise
    else{

      //If the servo is in reverse
      if(ser1rev == true){

        //continue to move the servo in reverse
        servo1pos = servo1pos - 1;
        servo1.write(servo1pos);

        //if at max, stop reversing
        if(servo1pos == 85){
          ser1rev = false;
        }
      }

      //otherwise
      else

        //move the servo forward
        servo1pos = servo1pos + 1;

        //if at max, stop moving formward
        servo1.write(servo1pos);
        if(servo1pos == 170){
          ser1rev = true;
        }
      }
    }

    //If the servo is in it's neutral state
    if(servo2pos == 180){

      //and there's something in the firing queue
      if(arr2[front2] != 0){

        //Move the ramp to the value in the firing queue
        ramp2.write(ramp2angles[arr2[front2]-1]);

        //pop the value
        arr2[front2] = 0;
        if(front2 == 9){
          front2 = 0;
        }
        else{
          front2 = front2 +1;
        }

        //Start the led animation
        ani2 = 1;

        //Start moving the servo
        servo2pos = 170;
        servo2.write(servo2pos);
      }
    }

    //Otherwise
    else{

      //If the servo is in reverse
      if(ser2rev == true){

        //continue to move the servo in reverse
        servo2pos = servo2pos + 1;
        servo2.write(servo2pos);

        //if at max, stop reversing
        if(servo2pos == 180){
          ser2rev = false;
        }
      }

      //otherwise
      else{

        //move the servo forward
        servo2pos = servo2pos - 1;
        servo2.write(servo2pos);

        //if at max, stop moving formward
        if(servo2pos <= 90){
          ser2rev = true;
        }
      }
    }

    //reset the servo timer
    servocount = 0;

  }

  //animation timer increment
  anicount = anicount + 1;

  //If the animation counter has past 150
  if(anicount == 150){

    //Reset all the leds
    for(int x = 0; x<16; x++){
        leds[x] = CRGB::Black;
    }

    //Reset the led checks
    for(int i = 0; i<4; i++){
      ledcheck[i] = 0;
    }

    //On aniation 1
    switch(ani1){

      //If it hasn't started, break
      case 0:
        break;
      case 6:
      case 5:

      //on count of 4 or greater
      //Light up farest row
      case 4:
        for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Red;
        }
        ledcheck[0] = 1;

      //on count of 3 or greater
      //Light up 2nd farest row
      case 3:
        for(int j = 0; j<4; j++){
            leds[rows[1][j]] = CRGB::Red;
        }
        ledcheck[1] = 1;

      //on count of 2 or greater
      //Light up 2nd closest row
      case 2:
        for(int j = 0; j<4; j++){
          leds[rows[2][j]] = CRGB::Red;
        }
        ledcheck[2] = 1;

      //on count of 1 or greater
      //Light up closest row
      case 1:
        for(int j = 0; j<4; j++){
          leds[rows[3][j]] = CRGB::Red;
        }
        ledcheck[3] = 1;

      //Increment the animation count and reset if it maxes out
      default:
        if(ani1 == 6){
          ani1 = 0;
        }
        else{
          ani1 = ani1 + 1;
        }
    }

    //On aniation 2
    switch(ani2){

      //If it hasn't started, break
      case 0:
        break;
      case 6:
      case 5:

      //on count of 4 or greater
      //Light up farest row
      //If's already been colored by animation 1, use alt color
      case 4:
        if(ledcheck[3] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[3][j]] = CRGB::Cyan;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[3][j]] = CRGB::Blue;
          }
        }

      //on count of 3 or greater
      //Light up 2nd farest row
      //If's already been colored by animation 1, use alt color
      case 3:
        if(ledcheck[2] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[2][j]] = CRGB::Cyan;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[2][j]] = CRGB::Blue;
          }
        }

      //on count of 2 or greater
      //Light up 2nd closest row
      //If's already been colored by animation 1, use alt color
      case 2:
        if(ledcheck[1] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[1][j]] = CRGB::Cyan;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[1][j]] = CRGB::Blue;
          }
        }

      //on count of 1 or greater
      //Light up closest row
      //If's already been colored by animation 1, use alt color
      case 1:
        if(ledcheck[0] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Cyan;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Blue;
          }
        }

      //Increment the animation count and reset if it maxes out
      default:
        if(ani2 == 6){
          ani2 = 0;
        }
        else{
          ani2 = ani2 + 1;
        }
    }

    //Show leds
    FastLED.show();
    anicount = 0;
  }
  delay(1);


}
