// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// This user.cpp communicates Eye status information over the I2C bus to 
// a device at the CAM_CONTROLLER_ADDRESS. 

#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include <Wire.h>    //use SCl & SDA

#define DEBUG // This is a sample after all library code should have serial.print wrapped in #if defined(DEBUG)
// #define I2CDEBUG // bus scan and other functions.. running with the "production" code is likely to cause chaos
#define NODTOSTRF // Adafruit or someone has not implemented DTOSTRF for SAMD // MAKE SURE TO Set this before including GazeControl.h
#include <GazeControl.h> // NOTE Pull the .cpp file and .h file from my github and put them into your Arduino/Libraries directory

// TODO: Create structure to hold the current eye state
// Then in Loop check to see if the eye state has changed
// if the eye state HAS chagned, then transmit the change to the I2C Controller

// don't forget prototyeps since this is not a .ino

// struct to hold incoming Gaze Control
GazeControlStruct GC_Incoming; // Holds values read
GazeControlStruct GC_Outgoing; // fill with values to be written
#define RANDOM_COUNTDOWN 100
int countdown_to_random = RANDOM_COUNTDOWN;
bool I_AM_CLIENT = false;
float EyeValIReadLastTimeX, EyeValIReadLastTimeY;
bool ScaledEyetarget = false;

// variables in the .ino file I need access to to figure out if the curren eye target has changed

extern bool eyeInMotion;
extern float eyeNewX;
extern float eyeNewY;



void ReadEyeStateFromI2C(int ID); // check the I2C bus for new eye state
void WriteEyeStateToI2C(int ID); // and write it out to the bus
bool SameValueIReadLastTime();     // are these valid values the same as last time.. slight optimization hack
void pointEyesSomewhereNew();      // function copies the GC to variables used by M4_Eyes.ino
void CaptureEyeState();     //Copy the variables used by M4_eyes.ino into GC
//void UpdateEyePosition(float eyeX, float eyeY);
//float GetScaledEyeTargetX();
//float GetScaledEyeTargetY();

#ifdef I2CDEBUG
void ScanBus();
void WriteGazeState(int ID);
void WTFI2CNotStringQuiet(int ID);
#endif

void user_setup(void) {

  // NOTE M4_Eyes starts up serial for debugging
  // Also NOTE that normal Wire.begin is not done here, because it is already set up and running elsewhere. 

  Serial.println("Beginning wire library...");
  // Wire.begin(SDA,SCL); // Join the I2C bus as master NOTE; this has already been done elsewhere
                       // we are doing it again just in case. 
  
  // Wire.setClock(BUS_FREQ);
  Serial.println("Monster M4sk is on the I2C bus ");
  countdown_to_random = RANDOM_COUNTDOWN;


}


// Called once per frame. 
// Small state machine
// IF Main code changes Eye state Write that new eye state out

// IF I2C bus sends me a valid new eye state set the eye state to that

void user_loop(void) 
{
  
  if(!eyeInMotion) // DO NOT Read or Write to I2C bus while eyes are moving
  {
    ReadEyeStateFromI2C(CAM_CONTROLLER_ADDRESS); // check the I2C bus for new eye state
    if( (GC_Incoming.IS) && !SameValueIReadLastTime()) // if the new eye state is valid and the Eye target is different 
    {
      I_AM_CLIENT = true;
      // I read a valid eye control state from the I2C bus and it was DIFFERENT
      // from what I have stored or what was written last
      // also if I never read a valid eye state from the I2C bus
      // moveEyesRandomly will never get set to false. 
      moveEyesRandomly = false; 

      countdown_to_random = RANDOM_COUNTDOWN;  // reset countdown timer
      pointEyesSomewhereNew(); // function copies the GC to variables used by M4_Eyes.ino
    }
    else
    {
        // if we ever go 100 or more ticks without reading an external eye control
        // assume we are master OR 
        // no new state read from external controller
        // our own state has not changed
        // tick down countdown to indempendant control
        countdown_to_random--;
        if(countdown_to_random <= 0)
        {
          // make myself my own master until I read a valid value again
          I_AM_CLIENT = false;
          moveEyesRandomly = true; 
          countdown_to_random = 1; // to avoid countdown going to rediculious low and looping back around
        }
    }
    
    // IF I read a valid control message and the data was different that what I already had
    // I set the variables in the m4_ino... and there won't be any difference between those values and
    // the values I have for eye target

    // IF there is still a difference in values that means
    // I've been told (or I calculated) a new master state and need to write it out. 
    // This avoids a bunch of un-necessary communications on the I2C bus. 
    // The CAM Controller actually decides which M4sk is the "Primary" and will
    // pass on writes from Primary to Secondary, and ignore writes from Secondary. 
    // Primary and Secondary are determined by who is plugged into which I2C bus

    // ALSO.. if I read in a new eye val from the I2C bus
    // it won't change newEyeX until the next loop...
    if(TargetChanged) // new value added to M4Eyes code
    {
      CaptureEyeState();         // copy the Eye State // if there is flickering issues on the secondary it might be this...
      WriteEyeStateToI2C(CAM_CONTROLLER_ADDRESS); // and write it out to the bus
    }

  }  
  // if the eye is in motion don't do anything

#ifdef I2CDEBUG
  ScanBus();
#endif
}

bool SameValueIReadLastTime()
{
  bool ret = false;
  if((GC_Incoming.X == EyeValIReadLastTimeX) && (GC_Incoming.Y == EyeValIReadLastTimeY))
  {
    ret = true;
  }
  else
  {
    EyeValIReadLastTimeX = GC_Incoming.X;
    EyeValIReadLastTimeY = GC_Incoming.Y;
  }
  return ret;
}


void ReadEyeStateFromI2C(int ID) // check the I2C bus for new eye state
{
  
  GazeFromI2C_Client ( &GC_Incoming, &Wire, ID ); // hate magic numbers and suprises, but Wire is set up deep and hidden in the 
                                         // adafruit arcade library

#ifdef DEBUG
  String IS = String(" ");

  BuildMessageString (&IS, &GC_Incoming ); 

  Serial.printf(" M4SK Read Face Control Message client = %d : ", I_AM_CLIENT);
  Serial.println(IS);
#endif

}

void WriteEyeStateToI2C(int ID) // and write it out to the bus
{
  if(GC_Outgoing.IS == 1) // Is this is a valid GC to write?
  {
    GazeToI2C_Client ( &GC_Outgoing, &Wire, ID );

  #ifdef DEBUG
    Serial.printf("Wrote GC_Outgoing.IS is %i ",GC_Outgoing.IS);
    String IS = String(" ");

    BuildMessageString (&IS, &GC_Outgoing ); 

    Serial.printf(" M4SK WROTE Face Control Message to %d",ID);
    Serial.println(IS);
  #endif
    
    GC_Outgoing.IS = 0; // I wrote it once, don't write it again unles it changes

  }
  #ifdef DEBUG
  Serial.println("Write to I2C Called, but outgoing was INVALID");
  #endif
}


// NOTE I can NOT place eyeNewX and eyeNewX int GC because the UNITS ARE DIFFERENT
// se above code
void pointEyesSomewhereNew()      // function copies the GC to variables used by M4_Eyes.ino
{
  if(I_AM_CLIENT) // if I'm not a client don't bother
  { 
#ifdef DEBUG
    Serial.printf("I'm Client Pointing Eyes Somewhere Scaled is %i\n",GC_Incoming.S);
#endif
    if(GC_Incoming.S == 0 ) // If what is in the GC struct for X and Y are "RAW"
    {
      // Set the "last time I read" values so If I see these values come in from the I2C bus I'll ignore it
      // I need to tell the M4_eyes.ino that the eyeNewX and eyeNewY are coming from an external source and not create it
      ScaledEyeTarget = true;
      // eyeNewX = GC_Incoming.X; // this used to work, but current version busted
      // eyeNewY = GC_Incoming.Y; 
    }
    else
    {
      ScaledEyeTarget = false; // use "scaled" values in setting eyeTarget
      eyeTargetX = GC_Incoming.X;
      eyeTargetY = GC_Incoming.Y; // TODO Do pupil, blink and all the rest   
    }

  }
}

// NOTE: If the eyes jump once in a while this is likely why
void CaptureEyeState()     //Copy the variables used by M4_eyes.ino into GC
{
    if(!I_AM_CLIENT) // if I'm not a client don't bother
    {
      if(moveEyesRandomly)
      {
          // if eyes are moving randomly eyeNewX and eyeNewY are computed differently than eyeTargetX and eyeTargetY
          // I am communicating eyeTargetX and eyeTargetY...
          // so if the eye values are calculated differently I have to calculate what eyeTargetX and Y should be 
          // based on eyeNewX and eyeNewY... to see where I get this 
          // go look in the m4_eyes.ino
          // honestly how the eye gase is controlled externally should have been designed in from the start...

          float r = (float)mapDiameter - (float)DISPLAY_SIZE * M_PI_2; // radius of motion
          r *= 0.6;  // calibration constant

          GC_Outgoing.X = eyeNewX;
          GC_Outgoing.Y = eyeNewY;
          GC_Outgoing.S = 0;
      }
      else
      {
        GC_Outgoing.X = eyeTargetX;
        GC_Outgoing.Y = eyeTargetY;// TODO Do pupil, blink and all the rest
        GC_Outgoing.S = 1;
      }
      GC_Outgoing.IS = 1;
    }
#ifdef DEBUG
  Serial.printf("Captured Eyes state IS %i X = %f Y = %f Scaled was %i\n",GC_Outgoing.IS,GC_Outgoing.X,GC_Outgoing.Y,GC_Outgoing.S);
#endif
}


float GetScaledEyeTargetX()
{
  return GC_Incoming.X;
}
float GetScaledEyeTargetY()
{
  return GC_Incoming.Y;
}

void UpdateEyePosition(float X, float Y)
{
  ScaledEyeTarget = true;
  GC_Outgoing.IS = 1;
  GC_Outgoing.X = X;
  GC_Outgoing.Y = Y;
  GC_Outgoing.S = 1;
}

#ifdef I2CDEBUG // debugging functions


void ScanBus()
{
  byte error;
  int address;
  int nDevices;
  Serial.println("Scanning... bus2");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      if (address == CAM_CONTROLLER_ADDRESS)
      {
        Serial.printf("Found the camera controller \n ", address);

        // TEST Writing to it like I'm Primary
        WriteGazeState(address);    

        // Test Reading from it like Im secondary
 
        WTFI2CNotStringQuiet(address);

      }
      else if (address == JOY_FEATHERWING)
      {
        Serial.printf("Found a Joy Featherwing \n", address);

        WTFI2CNotStringQuiet(address);
      }

      nDevices++;
    }

    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done scanning pausing\n");
  }
}

// Better Handel I2C Input
void WTFI2CNotStringQuiet(int ID)
{
  String IS = String(" ");
  GazeControlStruct GZ;

  GazeFromI2C_Client ( &GZ, &Wire, ID );

  BuildMessageString (&IS, &GZ ); 

  Serial.printf(" M4SK Read Face Control Message ");
  Serial.println(IS);
 
}

void WriteGazeState(int ID)
{
  String IS = String(" ");
   GazeControlStruct GF;

  GF.IS = 1 ; 
  GF.X = -44.41222 ; 
  GF.Y = -2.3333 ; 
  GF.B = 4 ; 
  GF.P = 5 ; 
  GF.M = 6 ; 
  GF.W = 7 ; 
  GF.H = 888 ; 
 
  GazeToI2C_Client ( &GF, &Wire, ID );
 
  BuildMessageString (&IS, &GF ); 

  Serial.printf(" M4SK WROTE Face Control Message to %d",ID);
  Serial.println(IS);

}
#endif // I2CDEBUG
#endif // 0
