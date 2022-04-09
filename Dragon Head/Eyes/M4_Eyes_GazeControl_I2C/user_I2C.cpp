// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// This user.cpp communicates Eye status information over the I2C bus to 
// a device at the CAM_CONTROLLER_ADDRESS. 

#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

#include "globals.h"
#include <Wire.h>    //use SCl & SDA

#define DEBUG // This is a sample after all library code should have serial.print wrapped in #if defined(DEBUG)
#define NODTOSTRF // Adafruit or someone has not implemented DTOSTRF for SAMD
#include <GazeControl.h> // NOTE Pull the .cpp file and .h file from my github and put them into your Arduino/Libraries directory


// TODO: Create structure to hold the current eye state
// Then in Loop check to see if the eye state has changed
// if the eye state HAS chagned, then transmit the change to the I2C Controller

// don't forget prototyeps since this is not a .ino

void ScanBus();
void WriteGazeState(int ID);
void WTFI2CNotStringQuiet(int ID);

void user_setup(void) {

  // NOTE M4_Eyes starts up serial for debugging
  // Also NOTE that normal Wire.begin is not done here, because it is already set up and running elsewhere. 

  Serial.println("Beginning wire library...");
  // Wire.begin(SDA,SCL); // Join the I2C bus as master NOTE; this has already been done elsewhere
                       // we are doing it again just in case. 
  
  // Wire.setClock(BUS_FREQ);
  Serial.println("Monster M4sk is on the I2C bus ");
}


// Called once per frame. 

void user_loop(void) 
{

  ScanBus();

}


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
#endif // 0
