#if 1

#if !defined(SERIAL_OUTPUT)// Change to 0 to disable this code (must enable ONE user*.cpp only!)
#define SERIAL_OUTPUT
#endif

#include "globals.h"
// Make Sure to #define SERIAL_OUTPUT in globals.h or as a build option

// This file is based on the user.cpp template
// following the form of the previously approved user_* additions
// the idea is this file gets renamed to user.cpp

// This simple extension writes the eye gase location to the USB output


#define SERIAL_BAUD 115200 // make sure this matches the rate at which your sender is transmitting

// USE the same format string from the sister project to this one
// for example ../ESP32-Eye-Tracker/FaceRepoting.h
// ideally we would just include that here, but for a good explination
// why we can't google " arduino include headder files from other directories"
static const char EyeGaseFormat[] = " Looking At X = %i Y = %i Blink = %i\n ";

typedef struct {
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int Blink; // >0 for blink 0 for no blink
  

} GaseLocationStruct;

bool HandelSerialInput();
void ParceIncomingString( String str, GaseLocationStruct* fat );
void LookAt(GaseLocationStruct* fat);

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global.
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop
String IncomingString = String(" ");




void user_setup(void) 
{
    if(!Serial)
    {
      Serial.begin(SERIAL_BAUD);
/*      while (!Serial)
        yield();*/
    }
  

// Called once after the processing of the configuration file. This allows
// user configuration to also be done based on the config file.
#include <ArduinoJson.h>          // JSON config file functions
void user_setup(StaticJsonDocument<2048> &doc) {
}



/*
 * Handel Serial Input, 
 * This function is called once for each time user_loop is called
 * BUT
 * Sometimes the serial buffer will not contain a complete  message
 * Soooo
 */

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop



bool HandelSerialInput()
{
  char c;
  bool ret = false;
  if(Serial.available()>0) // hey there is something there
  {
    while(Serial.available()>0)
    {
          IncomingString += (char)Serial.read();
    }
    
    if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
    {
      GaseLocationStruct FaceAt;
      ret = true
      
      ParceIncomingString(IncomingString,  &FaceAt);
      ret = true;
      IncomingString.remove(1); // empty the string
      moveEyesRandomly = false; // stop random eye movement TODO: Time this to prevent jerking
      LookAt( &FaceAt );
    }
    else
    {
     
      // incomplete message, just throw it away, something may be goig wrong

    }
  }
  return(ret); 

}
/*
 * This function is called when a full set of co-ordinates have been read in. 
 * The message that comes from the camrea is pretty verbose, makes it handy for debugging
 * it will look something like Face "found = 1 X = 147 Y = 147 W = 240 H = 320\n"
 * it _may_ have leading and trailing spaces or stuff...
 * in the code running on the other microcontroller, or computer or whatever 
 * Simply copy the sprintf fuction from the sener and reverse it with scanf here. 
 * 
 */
void ParceIncomingString( String str, GaseLocationStruct* fat  )
{
  str.trim(); // remove leading and trailing whitespace
  sscanf(str.c_str(),EyeGaseFormat, 
                      &fat->X,
                      &fat->Y,
                      &fat->Blink
                      );
}

// point the eyes at a location
// unlike other input, say from sensors, this is pre-scaled co-ords passed to us
// from another Monster M4SK
// NOTE this is using the eyeTargetX and Y from the M4_Eyes.ino

void LookAt(GaseLocationStruct* fat)
{

    eyeTargetX = fat->X;                 
                
    eyeTargetY = fat->Y;

}

static int count = 0;
void user_loop(void) {


  if (!HandelSerialInput())
  {
    count++;
    if(count > 100) // no faces found so we do a non time based delay before randomly looking around again
     moveEyesRandomly = true;
  }
  else
  {
    count = 0;
  }

}


#endif // 0 && !defined(SERIAL_CONTROL) 
