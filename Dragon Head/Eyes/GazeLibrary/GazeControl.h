#ifndef GAZECONTROL_H
#define GAZECONTROL_H

#include <Wire.h>
#include <Arduino.h>

/*************** Gaze Control Library *************************/
/* helper functions for controling a Monster M4sk over I2C */

// Format of the data string that is passed back and forth between mask and things that control the mask
// The identifiers are terse because I found that sometimes trying to read more than 129 bytes from
// I2C bus will fail. 
static const char GazeControlMessageFormat[] = " F = %i X = %i Y = %i B = %i P = %i M = %i W = %i H = %i W_Min = %i H_Min = %i W_Max = %i H_Max = %i\n ";

/*
 * F: 1 or not 1  F = anything else by 1 the message will not be parsed Is a Face detected, 1 if this is a valid message or not Or any other valid eye input 
 * X: integer, Intended x position of eye center, for example center of rectangle of detected face or joystick position
 * Y: integer, Intended y position of eye center
 * B: integer, boolean 1 or not 1 B=1 will Blink. If a Blink = 1 is sent and no Blink = 0 closed eyes will time out after a bit
 * P: Integer, Desired Pupil Diameter (over-rides light sensor
 * M: Integer, -1,0,1 "Crosseye" set this to 1 to have the eyes go "crosseye" send -1 to have eyes look opposite 0 or anything else for normal
 * W: integer, "Width" Width of the x,y viewing plane (what is the width of the plane in pixels)
 * H: integer, "Height" Height of the x,y viewig plane (what is the height of the plane in pixels)
 * W_min "Minimum value of X" it's a wierd thing but detected faces are not found at the edge of 
 *         the plane, so the _min and _max parameters help map faces detected in the field of view to 
 *         an approprate Gaze direction
 * H_min "minimum valeu of Y"
 * W_max "maximum vlaue of X"
 * H_max "maximum value of Y"
 */

 typedef struct 
 {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int B; // BLINK Boolean, 0 if not blink 1 if blink
  int P; // Pupil diameter
  int M; // "Mirror eyes" 1 for crosseye -1 for walleye 0 for normal
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

} GazeControlStruct;

typedef struct
{

  float EyeX;
  float EyeY;

} EyeLocStruct;

void ParceIncomingString( String* str, GazeControlStruct* Gaze ); // assumes a string in GazeControlMessage format Fill out a Gaze based on string r
void BuildMessageString( String* str, GazeControlStruct* Gaze );  // Create a string from a Gaze Control struct
void LookAtFaceScaled(EyeLocStruct* Eye, GazeControlStruct* fat);
void GazeFromI2C_Client ( GazeControlStruct* Gaze, TwoWire* Bus, int ID );// called by master to read from a client
void GazeFromI2C_Master ( GazeControlStruct* Gaze, TwoWire* Bus);        // called by an I2C client to read from the I2C bus
void GazeToI2C_Master ( GazeControlStruct* Gaze, TwoWire* Bus); 
void GazeToI2C_Client ( GazeControlStruct* Gaze, TwoWire* Bus, int ID );
void CoreI2CWriteGaze( GazeControlStruct* Gaze, TwoWire* Bus );      // Core Write function
void CoreI2CReadGaze(GazeControlStruct* Gaze, TwoWire* Bus);          // Core Read function




const int CAM_CONTROLLER_ADDRESS = 0x52; // our address on both I2C busses, so any master knows where to talk to
const int JOY_FEATHERWING = CAM_CONTROLLER_ADDRESS + 1; // For Featherwing control
const int BUS_FREQ = 400000;  // make sure everyone uses the same bus fequency

/********************* End Library Headder ********************/



#endif