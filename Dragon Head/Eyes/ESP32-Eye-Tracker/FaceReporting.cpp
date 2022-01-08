// function that executes from the main loop of the program
/*
 * Serial face Location reporting support
 * THIS CODE IS NOT THREAD SAFE.. Yet.
 * TODO: Define this as a class for use in other projects, maybe even a library?
 */
#include "Arduino.h"
#include "FaceReporting.h"


FaceLocationStruct FaceAt;

void FaceReportingSetup()
{

    // define pin modes for tx, rx:
    Serial.println("\n Hardware Serial Out Setting Pin Modes");
    pinMode(RXPIN, INPUT);
    pinMode(TXPIN, OUTPUT);
    delay(100);


    Serial2.begin(SERIAL2_BAUD,SERIAL_8N1, RXPIN, TXPIN);

#ifdef DEBUG_FACE

    while (!Serial2)
    {
        Serial.print(".");  
    }
#endif

    Serial.printf("\n ESP32 Second Serial Setup successful TX on %d and RX on %d\n",TXPIN,RXPIN);

    FaceAt.IS = false;
    FaceAt.X = 0;
    FaceAt.Y = 0;
    FaceAt.W = 0;
    FaceAt.H = 0;

    // Min and Max for each axis are calculated ranges, not read in
    // and will expand if face X and Y are out of their range
// TODO: Find the W and H for the frame buffer and replace the 400
    int WFudge = (int) (400 * WRangeFudge);
    int HFudge = (int) (400 * HRangeFudge);
    FaceAt.W_Min = 0 + WFudge;
    FaceAt.W_Max = 400 - WFudge;
    FaceAt.H_Min = 0 + HFudge;
    FaceAt.H_Max = 400 - HFudge;
}

void SetFaceFoundAt( int x, int y, int w, int h)
{

    // X and Y will never be zero, the face detection routien can't find faces that start at the edge of the FOV
    // so if FaceAt.X (or Y) are zero, this is the first time a face has been found
    // We are going to swag that X and Y have to be 20% the width of the range up from 0 and down from Max
   
    // NOTE there is no error checking here, maybe consider
    // making sure X and Y are !<= 0 && X and Y are !> h and w and frame rezolution
    // make sure H and W are not loarger than frame rezolution 


    if(w>0)
    {
      // you first wrote this code in gradeschool
      // you'd thought you'd never see it in the wild
      // TODO: Do some error checking, for Y values <= 0 or >= h we may be getting garbage
      int Fudge = (int) (w * WRangeFudge);
      int tmpWMin = 0 + Fudge;
      int tmpWMax = w - Fudge;

      if(FaceAt.W_Min < tmpWMin)
      {
        tmpWMin = FaceAt.W_Min; // keep Min value as is 
      }

      if(x < tmpWMin )
      {
        tmpWMin = x;            // unless x is smaller
      }

      if(FaceAt.W_Max > tmpWMax)
      {
        tmpWMax = FaceAt.W_Max;   // keep max value as is 
      }

      if(x > tmpWMax)
      {
        tmpWMax = x;               // unless x is larger
      }

      FaceAt.W_Min = tmpWMin;
      FaceAt.W_Max = tmpWMax;
    }

    if(h>0)
    {
      // you first wrote this code in gradeschool
      // you'd thought you'd never see it in the wild
      // TODO: Do some error checking, for Y values <= 0 or >= h we may be getting garbage
      int Fudge = (int) (h * HRangeFudge);
      int tmpHMin = 0 + Fudge;
      int tmpHMax = h - Fudge;

      if(FaceAt.H_Min < tmpHMin)
      {
        tmpHMin = FaceAt.H_Min; // keep max value as is 
      }

      if(y < tmpHMin )
      {
        tmpHMin = y;            // unless Y is smaller
      }

      if(FaceAt.H_Max > tmpHMax)
      {
        tmpHMax = FaceAt.H_Max;   // keep max value as is 
      }
      if(y > tmpHMax)
      {
        tmpHMax = y;              // unless Y is larger
      }

      FaceAt.H_Min = tmpHMin;
      FaceAt.H_Max = tmpHMax;
    }

  FaceAt.IS = true;
  FaceAt.X = x;
  FaceAt.Y = y;
  FaceAt.W = w;
  FaceAt.H = h;

}

 
void SetNoFaceFound()
{
  FaceAt.IS = false;
}
 

bool IsFaceFound()
{
  return FaceAt.IS;
}

void WriteFaceLocation()
{
 
  if (IsFaceFound())
  {
 
      // write the message out according to the format defined in FaceReporting.h
      Serial2.printf(FaceLocationRangeReportFormat,
                    FaceAt.IS,
                    FaceAt.X ,
                    FaceAt.Y ,
                    FaceAt.W ,
                    FaceAt.H ,
                    FaceAt.W_Min,
                    FaceAt.H_Min,
                    FaceAt.W_Max,
                    FaceAt.H_Max
      );
      
     // Debug write the message out according to the format defined in FaceReporting.h
#ifdef DEBUG_FACE
      Serial.printf(FaceLocationRangeReportFormat,
                    FaceAt.IS,
                    FaceAt.X ,
                    FaceAt.Y ,
                    FaceAt.W ,
                    FaceAt.H ,
                    FaceAt.W_Min,
                    FaceAt.H_Min,
                    FaceAt.W_Max,
                    FaceAt.H_Max
      );
      Serial.println("\n");
#endif
   }

}