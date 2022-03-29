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


#define SERIAL1_BAUD 115200 // make sure this matches the rate at which your sender is transmitting

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


// HERE BE MAGIC!
// This next block of code sets up the second UART on the SAMD51 to act as a serial port
// On the Monster M4SK this turns the PWM Micraphone port into a serial connection
// If you google this code you will find it in a couple places with vague descriptions
// of what it does. All of these definitions are buried deep in the Adafruit libraries
// I wish I could document all these definitions... If you import this project into platform.io
// running in VSCode you can actually use VSCode to referse-figureout what all this does
// in the Arduino IDE... lots of luck....
Uart Serial1( &sercom3, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;
void SERCOM3_0_Handler() { Serial1.IrqHandler(); }
void SERCOM3_1_Handler() { Serial1.IrqHandler(); }
void SERCOM3_2_Handler() { Serial1.IrqHandler(); }
void SERCOM3_3_Handler() { Serial1.IrqHandler(); }

// HOW TO WIRE!
// To be VERY CLEAR
// When wiring up the monster mask to use this code
// * look at the monster mask from the front where the tft screens are
// * look at the MEMS PWM Microphone JST connector, it has 4 tiny little pins
// * The above code makes the far left pin a serial recieve pin RX (usually the yellow wire on the JST wires)
// * The second pin from the left becomes serial TX
// * The other 2 pins are +5 and Ground. You can _almost_ ignore those unless you want to power whatever is sending you serial (not recommended for anything that takes more current than a PWM microphone)
// * it is good pratice to tie the ground to ground somewhere, and for long cables twist the cable bundle gently so the ground wire wraps around TX and RX


void user_setup(void) 
{
    if(!Serial)
    {
      Serial.begin(DEBUG_BAUD);
/*      while (!Serial)
        yield();*/
    }
  Serial.println("\n Serial1 control over Monster M4SK Eyes");
  Serial.printf("\n Setting up PWM port to be Serial port 1 TX and RX are %d %d\n",PIN_SERIAL1_TX,PIN_SERIAL1_RX);
  // If I knew what those pins where defined as....
  

  Serial1.begin(SERIAL1_BAUD);
  
  while (!Serial1)
  {
    Serial.print(".");
  }
  Serial.printf("\n Serial1 started successfully at %d Baud and ready to read\n",SERIAL1_BAUD);
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
 * Sometimes the serial buffer will not contain a complete F,X,Y,W,H message
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
  if(Serial1.available()>0) // hey there is something there
  {
    while(Serial1.available()>0)
    {
          IncomingString += (char)Serial1.read();
    }
    
    if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
    {
      GaseLocationStruct FaceAt;
      ret = true;
      Serial.print("Full Message : ");
      Serial.println(IncomingString);
      
      ParceIncomingString(IncomingString,  &FaceAt);
      ret = true;
      IncomingString.remove(1); // empty the string
      moveEyesRandomly = false; // stop random eye movement TODO: Time this to prevent jerking
      LookAt( &FaceAt );
    }
    else
    {
      Serial.print(IncomingString);
      Serial.println(" Reached end of buffer before EOM");
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
  sscanf(str.c_str(),FaceLocationRangeReportFormat, 
                      &fat->IS,
                      &fat->X,
                      &fat->Y,
                      &fat->W,
                      &fat->H,
                      &fat->W_Min,
                      &fat->H_Min,
                      &fat->W_Max,
                      &fat->H_Max
                      );

  Serial.printf(FaceLocationRangeReportFormat, 
                      fat->IS,
                      fat->X,
                      fat->Y,
                      fat->W,
                      fat->H,
                      fat->W_Min,
                      fat->H_Min,
                      fat->W_Max,
                      fat->H_Max       
                      );
}

// point the eyes at a location

void LookAt(GaseLocationStruct* fat)
{


  // X is some % of the distance from W_Min to W_Max inclusive (X could be = W_Min or max)
  // Y is some % of the distance from H_Min to H_Max inclusive (X could be = H_Min or max)

  // the range for eyeTargetX is a Float value between -1 and 1 with 0 being stright forward
  // the range for eyeTargetY is a float value between -1 and 1 with a 0 being straight forward

  // This is why the map() function exists. Because we all had to learn x is what percent of Y in algebra
  // and because the arduino map function can return out of range values...
  // this is why the constrain() function exists

  // BUUUT... the X and Y values don't actually go from 0 to W and H

  float screenX,screenY;
    eyeTargetX = (
                  (float)constrain(
                    map(fat->X, fat->W_Min, fat->W_Max, -10000, 10000),    // Need to map X which is an int to a float value between -1 and 1
                  -10000, 10000)
                  ) / (float)10000.0000;                       // so I map to a range that is [-1,1]x10000 then just divide the result by 10000 as a float. 
                
    eyeTargetY = (
                  (float)constrain(                                 // see commentary in rendering code for why -y
                    map(fat->Y, fat->H_Min, fat->H_Max, -10000, 10000),    // where would X be if it was in a range of DISPLAY_SIZE
                  -10000, 10000)
                  ) / (float)10000.0000;

  Serial.printf("Mapped values are %f , %f \n", eyeTargetX, eyeTargetY);
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
