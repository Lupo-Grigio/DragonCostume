// Wire Master Reader
// since both masks have to read... I'm making sure this code works first by testing on arduino
// then testing on a mask
// Based on original sample from Nicholas Zambetti <http://www.zambetti.com>

// Uses the wire library to read from teh i2c bus

// This example code is in the public domain.


#include <Wire.h>


////////// stuff that should be put in a library because it'll be shared by a bunch of different sketches
#define SLAVE_ADDRESS 0x52 // our address on both I2C busses, so any master knows where to talk to
                           // I borrowed this ID from the Adafruit Arcade library that maps Wii Nunchuck to I2C

#define DEBUG_BAUD 9600
// USE the same format string from the sister project to this one
// for example ../ESP32-Eye-Tracker/FaceRepoting.h
// ideally we would just include that here, but for a good explination
// why we can't google " arduino include headder files from other directories"
static const char EyeGaseRangeReportFormat[] = " Face found = %i X = %i Y = %i Blink = %i Pupil = %1 W = %i H = %i W_Min = %i H_Min = %i W_Max = %i H_Max = %i\n ";
typedef struct {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int Blink; // BLINK Boolean, 0 if not blink 1 if blink
  int Pupil; // pupil diameter value of 0 means the mask will set this to whatever the rest of the code does
  int W; // maximum value along the horizontal, Should be <= SCREEN_WIDTH
  int H; // maximum value along the vertical, should be <= SCREEN_HEIGHT
  
  // A face detected in the field of view will never have it's X and Y values
  // be either 0 or the maximum bounds of the edge of the FOV. 
  // ALSO, resolution could be changed dynamically
  // So we send along the minimum and maximum values along X and Y we have seen
  // this allows the Reciever to decide how to map the range of X and Y 
  // more dynamiclly and approprately
  // this will mean that X and Y will need to be mapped based on a range that is 
  // assumed to be changing for each frame. 
  int W_Min; // minimum range of the horizontal, should be >= 0 and <= W
  int H_Min; // minimum range of the vertical, should be >= 0 and <= H
  int W_Max; // minimum range of the horizontal, should be >= 0 and <= W
  int H_Max; // minimum range of the vertical, should be >= 0 and <= H

} EyeGaseStruct;

void ParceIncomingString( String str, EyeGaseStruct* fat );
// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global.
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop
String IncomingString = String(" ");
void ParceIncomingString( String str, EyeGaseStruct* fat  )
{
  str.trim(); // remove leading and trailing whitespace
  sscanf(str.c_str(),EyeGaseRangeReportFormat, 
                      &fat->IS,
                      &fat->X,
                      &fat->Y,
                      &fat->Blink,
                      &fat->Pupil,
                      &fat->W,
                      &fat->H,
                      &fat->W_Min,
                      &fat->H_Min,
                      &fat->W_Max,
                      &fat->H_Max
                      );

#if defined(ESP32) 
 Serial.printf(EyeGaseRangeReportFormat, 
                      fat->IS,
                      fat->X,
                      fat->Y,
                      fat->Blink,
                      fat->Pupil,
                      fat->W,
                      fat->H,
                      fat->W_Min,
                      fat->H_Min,
                      fat->W_Max,
                      fat->H_Max       
                      );
#else
  char bug[1000];
  //String bug = String(" ");
  sprintf(bug, EyeGaseRangeReportFormat, 
                      fat->IS,
                      fat->X,
                      fat->Y,
                      fat->Blink,
                      fat->Pupil,
                      fat->W,
                      fat->H,
                      fat->W_Min,
                      fat->H_Min,
                      fat->W_Max,
                      fat->H_Max       
                      );
  Serial.println(bug);
#endif                      
}


void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(DEBUG_BAUD);
}

#if !defined(ESP32)
int period = 1000;
unsigned long time_now = 0;
#endif


void loop()
{

#if defined(ESP32) 
  vTaskDelay(1000 / portTICK_RATE_MS);
#else
  time_now = millis();
  while(millis() < time_now + period)
  {
  }
#endif

  Serial.print("requesting data from I2C bus ID ");
  Serial.println(SLAVE_ADDRESS);

  HandleI2CInput(SLAVE_ADDRESS);

  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH); // heartbeat the builtin LED just so we know it's working

}


/*
 * Handel I2C Input, 
 * This function is called once for each time user_loop is called
 * BUT
 * Sometimes the serial buffer will not contain a complete F,X,Y,W,H message
 * Soooo
 */

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global IncomingString variable
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop

bool HandleI2CInput(int ID) // read from I2C device ID
{
  char c;
  bool ret = true;
  bool EOM = false;
  int min_message_length = sizeof(EyeGaseRangeReportFormat);

  min_message_length += 60; // at time of writing there are 10 parameters in the message so I'm gonna estimate 3 digits for each parameter then x2

  Wire.requestFrom(ID, min_message_length);    // request bytes from peripheral device #ID

  while(Wire.available()>0 && !EOM)    // slave may send less than requested
  {
    IncomingString += (char)Wire.read();
    if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
     {
      EOM = true;
      Serial.println("EOM");

      EyeGaseStruct FaceAt;
      ret = true;
      Serial.print("Full Message : ");
      Serial.println(IncomingString);
      
      ParceIncomingString(IncomingString,  &FaceAt);
      ret = true;
      IncomingString.remove(1); // empty the string
      // moveEyesRandomly = false; // UNCOMMENT WHEN RUNNING ON MASK IN PRODUCTON stop random eye movement TODO: Time this to prevent jerking
      LookAt( &FaceAt );
       
     }
  }
    
  return(ret); 
}

/*
 * Do any and all things to eye parameters based on what was read from input
 */
void LookAt(EyeGaseStruct* fat)
{

// See user serial for really good input map, For I2C we Ignore all the range information because
// X and Y are coming from another Monster M4sk, and do not need adjusting


  float screenX,screenY;
 //   eyeTargetX = (float)fat->X;                       UNCOMMENT WHEN RUNNING ON M4SK in "production"
 //   eyeTargetY = (float)fat->Y;

  //Serial.printf("Mapped values are %f , %f \n", eyeTargetX, eyeTargetY);
}
