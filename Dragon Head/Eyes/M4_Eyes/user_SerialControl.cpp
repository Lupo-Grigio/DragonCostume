#if 1

#if !defined(SERIAL_CONTROL)// Change to 0 to disable this code (must enable ONE user*.cpp only!)
#define SERIAL_CONTROL
#endif

#include "globals.h"
// Make Sure to #define SERIAL_CONTROL in globals.h or as a build option

// This file is based on the user.cpp template
// following the form of the previously approved user_* additions
// the idea is this file gets renamed to user.cpp

// Wiring Instructions
// To be VERY CLEAR
// When wiring up the monster mask to use this code
// * look at the monster mask from the front where the tft screens are
// * look at the MEMS PWM Microphone JST connector, it has 4 tiny little pins
// * The above code makes the far left pin a serial recieve pin RX (usually the yellow wire on the JST wires)
// * The second pin from the left becomes serial TX
// * The other 2 pins are +5 and Ground. You can _almost_ ignore those unless you want to power whatever is sending you serial (not recommended for anything that takes more current than a PWM microphone)
// * it is good pratice to tie the ground to ground somewhere, and for long cables twist the cable bundle gently so the ground wire wraps around TX and RX

// TODO: maybe spruce this method up so that individual user contributed functionality can be added through a "user.h" header file

// User globals can go here, recommend declaring as static, e.g.:
static int foo = 42;
static int bar = ((13.8 * 10) / 2) ; // a nice number

#define SERIAL1_BAUD 115200 // make sure this matches the rate at which your sender is transmitting

typedef struct {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int W; // Width of field
  int H; // Height of field
} FaceLocationStruct;

bool HandelSerialInput();
void ParceIncomingString( String str, FaceLocationStruct* fat );
void LookAt(FaceLocationStruct* fat);

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
      FaceLocationStruct FaceAt;
      ret = true;
      Serial.print("Full Message : ");
      Serial.println(IncomingString);
      
      ParceIncomingString(IncomingString,  &FaceAt);
      Serial.printf(" Parsed data looks like :%i,%i,%i,%i,%i\n",
                            FaceAt.IS,
                            FaceAt.X,
                            FaceAt.Y,
                            FaceAt.W,
                            FaceAt.H  
                    );
      ret = true;
      IncomingString.remove(1); // empty the string
      moveEyesRandomly = false; // stop random eye movement TODO: Time this to prevent jerking
      LookAt( &FaceAt );
    }
    else
    {
      Serial.print(IncomingString);
      Serial.println(" Reached end of buffer before EOM");
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
void ParceIncomingString( String str, FaceLocationStruct* fat  )
{
  str.trim(); // remove leading and trailing whitespace
  sscanf(str.c_str()," Face found = %i X = %i Y = %i W = %i H = %i \n", 
                      &fat->IS,
                      &fat->X,
                      &fat->Y,
                      &fat->W,
                      &fat->H
                      );
  Serial.printf(" Pointer Playground returned %i X = %i Y = %i W = %i H = %i \n", 
                      fat->IS,
                      fat->X,
                      fat->Y,
                      fat->W,
                      fat->H
  );
}

// point the eyes at a location
// TODO What is the eye scale, how wide and how tall
// TODO Go find the scale factor code from the old python
void LookAt(FaceLocationStruct* fat)
{
  // X is some % of the distance from 1 to W, the eyes horizontal scale is -1 to 1
  // Y is some % of the distance from 1 to H, the eyes vertical scale is -11 to 1


  // Face location X is some point between 1 and W, 
  // the eyeTargetX needs to be a value between 1 and DISPLAY_SIZE 
  //   that is porportional to X's distance between 1 and W
  // ditto for Y and H
  // This is why the map() function exists. Because we all had to learn x is what percent of Y in algebra
  // and because the arduino map function can return out of range values...
  // this is why the constrain() function exists

  float screenX,screenY;
    eyeTargetX = (
                  (float)constrain(
                    map(fat->X, 1, fat->W, -10000, 10000),    // Need to map X which is an int to a float value between -1 and 1
                  -10000, 10000)
                  ) / (float)10000.0000;                       // so I map to a range that is [-1,1]x10000 then just divide the result by 10000 as a float. 
                
  eyeTargetY = (
                  (float)constrain(                                 // see commentary in rendering code for why -y
                    map(fat->Y, 1, fat->H, -10000, 10000),    // where would X be if it was in a range of DISPLAY_SIZE
                  -10000, 10000)
                  ) / (float)10000.0000;
  // scale the camera co-ordinates to screen co-ordinates
  /*
   * now in a sane world we would just map the incoming range to screen co-ordinates
   
  eyeTargetX = (float)constrain(
                map(fat->X, 1, fat->W, 110, 310),    // where would X be if it was in a range of DISPLAY_SIZE
                110, 310);
  eyeTargetY = (float)constrain(                                 // see commentary in rendering code for why -y
                map(fat->Y, 1, fat->H, 110, 310),    // where would X be if it was in a range of DISPLAY_SIZE
                110, 310);
  
  int r = (mapDiameter - DISPLAY_SIZE * M_PI_2) * 0.9;
  eyeTargetX = (float)constrain(
                map(fat->X, 1, fat->W, -r, r),    // where would X be if it was in a range of DISPLAY_SIZE
                -r,r);
  eyeTargetY = (float)constrain(                                 // see commentary in rendering code for why -y
                map(fat->Y, 1, fat->H, -r, r),    // where would X be if it was in a range of DISPLAY_SIZE
               -r,r);
               */
    /* 
    now I have translated the incoming co-ordinates to screen co-ordinates
    any sane person would have jsut set eyeTargetX, eyeTargetY to those values and we're goldent
    buuuutttt noooooo. 
   The eye gase point calculation is really bizzarre.. 
  
                  
    r is the radius in X and Y that the eye can go, from (0,0) in the center.
   
   float r = ((float)mapDiameter - (float)DISPLAY_SIZE * M_PI_2) * 0.75;
              eyeNewX = random(-r, r);
              float h = sqrt(r * r - eyeNewX * eyeNewX);
              eyeNewY = random(-h, h);

    and if you just set eyeTargetX and eyeTargetY, and turn off random movement... your values get mangled by the following code

        float r = ((float)mapDiameter - (float)DISPLAY_SIZE * M_PI_2) * 0.9;
        eyeX = mapRadius + eyeTargetX * r;
        eyeY = mapRadius + eyeTargetY * r;

    
  // r is the radius in X and Y that the eye can go, from (0,0) in the center.
  // NOTE: This differens from teh calculation in M4_eyes I'm using integer math here
  // because I'm using the map function
  int r = (mapDiameter - DISPLAY_SIZE * M_PI_2) * 0.9; 
  // so now our scale of values ranges from -r to r
  // our input data scale is (for x) 1 to W (for y) 1 to H 

 // maybe need to divide by r and subtract radius? do the reverse of what happens in M4_Eyes
  eyeTargetX = (float)constrain(
                map(fat->X, 1, fat->W, -r, r),    // where would X be if it was in a constrained range of r to -r
                -r,r);
  eyeTargetY = (float)constrain(                                 // see commentary in rendering code for why -y
                map(fat->Y, 1, fat->H, -r, r),    // where would X be if it was in a constrained range of r to -r
                -r,r);
  int r = (mapDiameter - DISPLAY_SIZE * M_PI_2) * 0.9; 

  eyeTargetX = mapRadius - eyeTargetX / r; // man I dunno, I'm doing the reverse of what m4_eyes does to the numbers
  eyeTargetY = mapRadius - eyeTargetX / r;
                      */

  Serial.printf("Mapped values are %f , %f \n", eyeTargetX, eyeTargetY);
}

static int count = 0;
void user_loop(void) {

/*
 * psudeo code
 * if (read_complete_message() 
 * {   // process and store any data in the UART serial buffer
 *    when input char = End Of Message
 *    set_random_look = false
 *    parse_message(string message, *Address)     // true if message is parced successfully
 *    {
 *        LookAt(Location)
 *    }
 * } 
 * else
 * {
 *   set_random_look = true
 * }
 */
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

  // Set values for the new X and Y.
  //eyeTargetX = heatSensor.x;
 // eyeTargetY = -heatSensor.y;
}


#endif // 0 && !defined(SERIAL_CONTROL) 
