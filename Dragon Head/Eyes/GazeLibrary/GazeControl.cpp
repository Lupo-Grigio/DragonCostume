/********************************* Gaze Library Functions *************************/

#include "GazeControl.h"

/* 
 * ParceIncomingString
 *
 * This function is called when a full set of co-ordinates have been read in. 
 * The message that comes from the camrea is pretty verbose, makes it handy for debugging
 * it will look something like Face "found = 1 X = 147 Y = 147 W = 240 H = 320\n"
 * it _may_ have leading and trailing spaces or stuff...
 * in the code running on the other microcontroller, or computer or whatever 
 * Simply copy the sprintf fuction from the sener and reverse it with scanf here. 
 * 
 */
void ParceIncomingString( String* str, GazeControlStruct* Gaze  )
{
  str->trim(); // remove leading and trailing whitespace
  
  // NOTE X and Y are SIGNED FLOAT values that come to us as STRINGS
  // The strings should be less than 8 bytes each but we are not taking a chance
  char X[16];
  char Y[16];


  sscanf(str->c_str(),GazeControlMessageFormat, 
                      &Gaze->IS,
                      X,
                      Y,
                      &Gaze->S,
                      &Gaze->B,
                      &Gaze->P,
                      &Gaze->M,
                      &Gaze->W,
                      &Gaze->H
                      );

  // and since Arduino sscanf does not support floats
  // turn the X and Y strings I just read into proper float values
  Gaze->X = atof(X);
  Gaze->Y = atof(Y);

#if defined(DEBUG)

 Serial.printf("Actual float values converted: X = %6.4f Y=%6.4f Message Parsed \n : ",Gaze->X,Gaze->Y);
 Serial.printf(GazeControlMessageFormat, 
                      Gaze->IS,
                      X,
                      Y,
                      Gaze->S,
                      Gaze->B,
                      Gaze->P,
                      Gaze->M,
                      Gaze->W     
                      );
  Serial.println(" : ");

#endif

}

/****************** Build a Gaze Control Message String *********************/

void BuildMessageString( String* str, GazeControlStruct* Gaze )
{
  char buf[1024];

  // So the problem is that %f is not supported by arduino and _may_ be 
  // supported by your particular chip, but it may be a non standard implementation
  // Soooo... Ancient technique.. use Atof and dtostrf to convert from float and back
  // build X as a string from float value
  // Build Y as a string from a float value
  char X[16];
  char Y[16];
  
  dtostrf(Gaze->X,6,4,X); // convert X into a string 6 chars long with a 4 place percision so it should look something like -0.1234
  dtostrf(Gaze->Y,6,4,Y); // and another. look up docs and examples for dtostrf. it is odd compared to most other functions at this level.

  sprintf(buf,GazeControlMessageFormat, 
                      Gaze->IS,
                      X,
                      Y,
                      Gaze->S,
                      Gaze->B,
                      Gaze->P,
                      Gaze->M,
                      Gaze->W,
                      Gaze->H    
                      );

  str->remove(0); // clear the string
  str->concat(buf);


#if defined(DEBUG)
  Serial.print(" BuildMessageString Built : ");
  Serial.println(str->c_str());

#endif

}

/********************** Read and Fill out a Gaze Control Structure from an I2C client *******************/
/* 
 * Called by I2C Bus Master devide to read from an I2C Client
 * 
 * Pass a pointer to a Gaze control struct
 * and a pointer to an I2C bus
 * and in the case this is being called by the Mster of the bus, pass the ID of the client
      that is to send you the message
 */

void GazeFromI2C_Client ( GazeControlStruct* Gaze, TwoWire* Bus, int ID )
{
  int Length = 129;             // by painful trial and error found that reading more than 129 bytes on I2C bus causes corruption of the Wire object

  Bus->requestFrom(ID, Length);    // request some bytes from peripheral device #ID
                                // requesting 130 bytes or more causes Bus->read() to return nothing but garbage
#if defined(DEBUG)
  Serial.flush(); // I've been getting a lot of output interlacing with my verbose routiens even at 115200 baud
  Serial.printf("\n Reading available %i from slave: %i \n",Bus->available(),ID);
  Serial.flush();
#endif

  CoreI2CReadGaze( Gaze, Bus);

}

/********************** Read and Fill out a Gaze Control Structure from an I2C Master *******************/
/* 
 * Called by I2C Bus Client divice to read from an I2C Bus Master
 * 
 * Pass a pointer to a Gaze control struct
 * and a pointer to an I2C bus
 *
 */


void GazeFromI2C_Master( GazeControlStruct* Gaze, TwoWire* Bus )
{

#if defined(DEBUG)
  Serial.print("I2C BUS Client requested client to READ from I2C by Master ");
#endif

  CoreI2CReadGaze( Gaze, Bus);


}

/* Core I2CReadGaze function
 * if this was a class this would be a private funciton... too lazy to transform this to a class
 * Called by the Master and Client read functions
 * ASSUME the TwoWire that is being passed in is ready to be read from, e.g. on master RequestFrom must be call prior to this
 */

void CoreI2CReadGaze(GazeControlStruct* Gaze, TwoWire* Bus)
{
  char c;
  String IncomingString(" ");

  IncomingString.remove(0); // empty the string, just in case. I've seen garbage in the string at creation

  while(Bus->available()>0 )    
  {
    c = Bus->read();

#if defined(DEBUG)
    Serial.printf("Read a char %c :",c);
    if (c == '\n'){Serial.println("EOM");}
#endif

    if (isAscii(c) )
    {
      IncomingString += c;

#if defined(DEBUG)
      Serial.printf("%c was added to ",c);
      Serial.println(IncomingString); // because F***** printf cant turn a String into a %s FIIIIUUUKK MMEEE
#endif

    }
    else
    {
      // don't add garbage to string. 
    }

  }

  ParceIncomingString( &IncomingString, Gaze  ); // now that I've read data from the Wire, turn that string into something useful

}

/* 
 * Write Gaze to a CLIENT from a MASTER
 * Naming is getting a little confusing now
 */
void GazeToI2C_Client(GazeControlStruct* Gaze, TwoWire* Bus, int ID)
{
  Bus->beginTransmission(ID); // transmit to device #4
  
  CoreI2CWriteGaze( Gaze, Bus);
  
  Bus->endTransmission();    // stop transmitting

}

/* 
 * Write Gaze to a MASTER from a Client
 * Naming is getting a little confusing now
 * This function exists soly to make sure code is symmetrical between master and client
 */
void GazeToI2C_Master(GazeControlStruct* Gaze, TwoWire* Bus)
{
 
  CoreI2CWriteGaze( Gaze, Bus );

}

/* Core Write Function 
 *  Format a carector buf from a GGazeControlStruct* and write it to the passed in Wire
 *  I2C Master must call BeginTransmission prior and after calling this
 */
void CoreI2CWriteGaze( GazeControlStruct* Gaze, TwoWire* Bus)
{

  String buf = String(" ");
  BuildMessageString( &buf, Gaze );

#if defined(DEBUG)
  Serial.print("Writing to I2C Bus : ");
  Serial.println(buf.c_str());
#endif

  Bus->write(buf.c_str()); 

}

#ifdef NODTOSTRF  // turns out not all arduino implementations have dtostrf

   
/*
https://github.com/arduino/Arduino/blob/a2e7413d229812ff123cb8864747558b270498f1/hardware/arduino/sam/cores/arduino/avr/dtostrf.c
  dtostrf - Emulation for dtostrf function from avr-libc
  Copyright (c) 2013 Arduino.  All rights reserved.
  Written by Cristian Maglie <c.maglie@bug.st>
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}

#endif


#ifdef FACETRACKING

/* TODO Removing the maxes from the message
 * and transforming the message from int's to floats
 * The face detection functions will have to figure how to deal with the mins and maxes that used to be transmitted

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
*/
// 
void LookAtFaceScaled(EyeLocStruct* Eye, GazeControlStruct* fat)
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
    Eye->EyeX = (
                  (float)constrain(
                    map(fat->X, fat->W_Min, fat->W_Max, -10000, 10000),    // Need to map X which is an int to a float value between -1 and 1
                  -10000, 10000)
                  ) / (float)10000.0000;                       // so I map to a range that is [-1,1]x10000 then just divide the result by 10000 as a float. 
                
    Eye->EyeY = (
                  (float)constrain(                                 // see commentary in rendering code for why -y
                    map(fat->Y, fat->H_Min, fat->H_Max, -10000, 10000),    // where would X be if it was in a range of DISPLAY_SIZE
                  -10000, 10000)
                  ) / (float)10000.0000;
#if defined(DEBUG)
  Serial.printf("Mapped values are %f , %f \n", Eye->EyeX, Eye->EyeY);
#endif


}
#endif

