#include <FastLED.h>
#include <Servo.h>

#define NUM_LEDS 16
#define DATA_PIN 3

//
const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};
int newFlashInterval = 0;
float servoFraction = 0.0; // fraction of servo range to move

int servoMode = 1;
int commitUser = 1;

unsigned long curMillis;

unsigned long prevReplyToPCmillis = 0;
unsigned long replyToPCinterval = 1000;
//


byte incomingByte;
int bytecount = 0;
int user = 0;
int size = 1;
int servo2pos = 180;
int servo1pos = 85;
boolean ser1rev = false;
boolean ser2rev = false;

int servocount = 0;
int ani1 = 0;
int ani2 = 0;
int anicount = 0;
int rows[4][4] = { {0,7,8,15}, {1,6, 9, 14}, {2, 5, 10, 13}, {3, 4, 11, 12} };

int arr1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
int front1 = 0;
int back1 = 0;
int arr2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
int front2 = 0;
int back2 = 0;

int ramp2angles[3] = {90, 115, 140};
int ramp1angles[3] = {170, 150, 130};

Servo servo1;
Servo ramp1;
Servo servo2;
Servo ramp2;
CRGB leds[NUM_LEDS];
int ledcheck[4] = {0, 0, 0, 0};

//=============

void getDataFromPC() {

    // receive data from PC and save it into inputBuffer
    
  if(Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant
      
    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
  }
}

//=============
 
void parseData() {

    // split the data into its parts
    
  char * strtokIndx; // this is used by strtok() as an index
  
  /*strtokIndx = strtok(inputBuffer,",");      // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
  
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  newFlashInterval = atoi(strtokIndx);     // convert this part to an integer
  
  strtokIndx = strtok(NULL, ","); 
  servoFraction = atof(strtokIndx);     // convert this part to a float
*/
  strtokIndx = strtok(inputBuffer,","); // this continues where the previous call left off
  commitUser = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  servoMode = atoi(strtokIndx);
  switch(commitUser){
    case 1:
      if(front1 == back1 && arr1[back1] != 0){
        break;
      }
      arr1[back1] = servoMode;
      if(back1 == 9){
        back1 = 0;
      }
      else{
        back1 = back1 + 1;
      }
      break;
    case 2:
      if(front2 == back2 && arr2[back2] != 0){
        break;
      }
      arr2[back2] = servoMode;
      if(back2 == 9){
        back2 = 0;
      }
      else{
        back2 = back2 + 1;
      }
      break;
  }
  
  
}

//=============

void replyToPC() {

  if (newDataFromPC) {
    newDataFromPC = false;
    Serial.print("<Msg ");
    Serial.print(messageFromPC);
    Serial.print(" NewFlash ");
    Serial.print(commitUser);
    Serial.print(" SrvFrac ");
    Serial.print(servoMode);
    Serial.print(" Time ");
    Serial.print(curMillis >> 9); // divide by 512 is approx = half-seconds
    Serial.println();
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
    Serial.println(">");
  }
}

void setup() {
  servo1.attach(9);
  servo1.write(servo1pos);
  servo2.attach(10);
  servo2.write(servo2pos);
  ramp1.attach(6);
  ramp2.attach(11);
  ramp2.write(140);
  ramp1.write(170);
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  Serial.begin(9600);
  for(int x = 0; x<16; x++){
    leds[x] = CRGB::Black;
  }
  FastLED.show();
  Serial.println("<Arduino is ready>");

}

void loop() {

  while (Serial.available() > 0) {
    curMillis = millis();
    getDataFromPC();
    replyToPC();
  }
  servocount = servocount + 1;
  if(servocount == 5){
    if(servo1pos == 85){
      if(arr1[front1] != 0){
        ramp1.write(ramp1angles[arr1[front1]-1]);
        arr1[front1] = 0;
        if(front1 == 9){
          front1 = 0;
        }
        else{
          front1 = front1 + 1; 
        }
        ani1 = 1;
        servo1pos = 90;
        servo1.write(servo1pos);
      }
    }
    else{
      if(ser1rev == true){
        servo1pos = servo1pos - 1;
        servo1.write(servo1pos);
        if(servo1pos == 85){
          ser1rev = false;
        }
      }
      else{
        servo1pos = servo1pos + 1;
        servo1.write(servo1pos);
        if(servo1pos == 170){
          ser1rev = true;
        }
      }
    }
    
    if(servo2pos == 180){
      if(arr2[front2] != 0){
        ramp2.write(ramp2angles[arr2[front2]-1]);
        arr2[front2] = 0;
        if(front2 == 9){
          front2 = 0;
        }
        else{
          front2 = front2 +1; 
        }
        ani2 = 1;
        servo2pos = 170;
        servo2.write(servo2pos);
      }
    }
    else{
      if(ser2rev == true){
        servo2pos = servo2pos + 1;
        servo2.write(servo2pos);
        if(servo2pos == 180){
          ser2rev = false;
        }
      }
      else{
        servo2pos = servo2pos - 1;
        servo2.write(servo2pos);
        if(servo2pos <= 90){
          ser2rev = true;
        }
      }
    }
    servocount = 0;
    //Serial.println(servo2pos);
    //Serial.println(ser2rev);
  }
  anicount = anicount + 1;
  if(anicount == 150){
    for(int x = 0; x<16; x++){
        leds[x] = CRGB::Black;
    }
    for(int i = 0; i<4; i++){
      ledcheck[i] = 0;
    }
    switch(ani1){
      case 0:
        break;
      case 6:
      case 5:
      case 4:
        for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Red;
        }
        ledcheck[0] = 1;
      case 3:
        for(int j = 0; j<4; j++){
            leds[rows[1][j]] = CRGB::Red;
        }
        ledcheck[1] = 1;
      case 2:
        for(int j = 0; j<4; j++){
          leds[rows[2][j]] = CRGB::Red;
        }
        ledcheck[2] = 1;
      case 1:
        for(int j = 0; j<4; j++){
          leds[rows[3][j]] = CRGB::Red;
        }
        ledcheck[3] = 1;
      default:
        if(ani1 == 6){
          ani1 = 0;
        }
        else{
          ani1 = ani1 + 1;
        }
    }
    switch(ani2){
      case 0:
        break;
      case 6:
      case 5:
      case 4:
        if(ledcheck[3] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[3][j]] = CRGB::Purple;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[3][j]] = CRGB::Blue;
          }
        }
        
      case 3:
        if(ledcheck[2] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[2][j]] = CRGB::Purple;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[2][j]] = CRGB::Blue;
          }
        }
      case 2:
        if(ledcheck[1] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[1][j]] = CRGB::Purple;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[1][j]] = CRGB::Blue;
          }
        }
      case 1:
        if(ledcheck[0] == 1){
          for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Purple;
          }
        }
        else{
          for(int j = 0; j<4; j++){
              leds[rows[0][j]] = CRGB::Blue;
          }
        }
      default:
        if(ani2 == 6){
          ani2 = 0;
        }
        else{
          ani2 = ani2 + 1;
        }
    }
    FastLED.show();
    anicount = 0;
  }
  delay(1);


}
