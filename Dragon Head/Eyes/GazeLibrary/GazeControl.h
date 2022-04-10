#ifndef GAZECONTROL_H
#define GAZECONTROL_H

#include <Wire.h>
#include <Arduino.h>

#define NODTOSTRF // Adafruit or someone has not implemented DTOSTRF for SAMD // MAKE SURE TO Set this before including GazeControl.h

/*************** Gaze Control Library *************************/
/* helper functions for controling a Monster M4sk over I2C */

// Format of the data string that is passed back and forth between mask and things that control the mask
// The identifiers are terse because I found that sometimes trying to read more than 129 bytes from
// I2C bus will fail. 
// NOTE: Arduino based platforms do not reliably support %f. specificly sscanf for floats is not implemented 
// this is due to the severe memory limitations on some arduino compatable devices. (and if you have ever tried implementing an OS on a small device you know this in your soul)
// to get around this we will send float values as strings, and convert them back and forth using dtostrf() and atof()
static const char GazeControlMessageFormat[] = " F = %i X = %s Y = %s S = %i B = %i P = %i M = %i W = %i H = %i\n ";

/*
 * F: 1 or not 1  F = anything else by 1 the message will not be parsed Is a Face detected, 1 if this is a valid message or not Or any other valid eye input 
 * X: Float, Intended x position of eye center, This value is to be scaled to be a range between -1 and 1 See the M4_eyes code for further details
 * Y: Float, Intended y position of eye center, This value is to be scaled to be a range between -1 and 1 See the M4_eyes code for further details
 * S: 1 or not 1 if 1 X and Y are scaled (eyeTargetX and Y) if anything other than 1 X and Y are raw (eyeNewX eyeNewY)
 * B: integer, boolean 1 or not 1 B=1 will Blink. If a Blink = 1 is sent and no Blink = 0 closed eyes will time out after a bit
 * P: Integer, Desired Pupil Diameter (over-rides light sensor
 * M: Integer, -1,0,1 "Crosseye" set this to 1 to have the eyes go "crosseye" send -1 to have eyes look opposite 0 or anything else for normal
 * W: integer, "Width" Width of the x,y viewing plane (what is the width of the plane in pixels)
 * H: integer, "Height" Height of the x,y viewig plane (what is the height of the plane in pixels)
 * NOTE it is the responsability of the sending device to convert input into a well distrabuted value between -1 and 1 for the x and y values 
 */

 typedef struct 
 {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  float X; // X co-ord of face
  float Y; // Y co-ord of face
  int S; // if 1 are X and Y "scaled"
  int B; // BLINK Boolean, 0 if not blink 1 if blink
  int P; // Pupil diameter
  int M; // "Mirror eyes" 1 for crosseye -1 for walleye 0 for normal
  int W; // maximum value along the horizontal, Should be <= SCREEN_WIDTH
  int H; // maximum value along the vertical, should be <= SCREEN_HEIGHT
  
} GazeControlStruct;



void ParceIncomingString( String* str, GazeControlStruct* Gaze ); // assumes a string in GazeControlMessage format Fill out a Gaze based on string r
void BuildMessageString( String* str, GazeControlStruct* Gaze );  // Create a string from a Gaze Control struct
void GazeFromI2C_Client ( GazeControlStruct* Gaze, TwoWire* Bus, int ID );// called by an I2C MASTER to READ FROM an I2C CLIENT
void GazeFromI2C_Master ( GazeControlStruct* Gaze, TwoWire* Bus);        // called by an I2C CLIENT to READ FROM an I2C MASTER
void GazeToI2C_Master ( GazeControlStruct* Gaze, TwoWire* Bus);          // called by an I2C CLIENT to WRITE TO the I2C bus
void GazeToI2C_Client ( GazeControlStruct* Gaze, TwoWire* Bus, int ID ); // called by an I2C MASTER to WRITE TO an I2C CLIENT

void CoreI2CWriteGaze( GazeControlStruct* Gaze, TwoWire* Bus );      // Core Write function
void CoreI2CReadGaze(GazeControlStruct* Gaze, TwoWire* Bus);          // Core Read function

#ifdef NODTOSTRF
char *dtostrf (double val, signed char width, unsigned char prec, char *sout);
#endif

#ifdef FACETRACKING
typedef struct
{

  float EyeX;
  float EyeY;

} EyeLocStruct;

void LookAtFaceScaled(EyeLocStruct* Eye, GazeControlStruct* fat);   // convert the center of the face bounding box to X and Y values to float values in a range between -1 and 1
#endif


const int CAM_CONTROLLER_ADDRESS = 0x52; // our address on both I2C busses, so any master knows where to talk to
const int JOY_FEATHERWING = CAM_CONTROLLER_ADDRESS + 1; // For Featherwing control
const int BUS_FREQ = 400000;  // make sure everyone uses the same bus fequency

/********************* End Library Headder ********************/



#endif