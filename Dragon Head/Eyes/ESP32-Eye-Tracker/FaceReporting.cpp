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

    Serial.printf("\n Beginning Hardware serial port 2 TX on pin %d, RX on pin %d\n",RXPIN,TXPIN);

    Serial2.begin(SERIAL2_BAUD,SERIAL_8N1, RXPIN, TXPIN);
    while (!Serial2)
    {
        Serial.print(".");  
    }
    Serial.printf("\n ESP32 Second Serial Setup successful TX on %d and RX on %d\n",TXPIN,RXPIN);

    FaceAt.IS = false;
    FaceAt.X = 0;
    FaceAt.Y = 0;
    FaceAt.W = 0;
    FaceAt.H = 0;
    FaceAt.W_Min = 0;
    FaceAt.H_Min = 0;
    FaceAt.W_Max = 0;
    FaceAt.H_Max = 0;
}

void SetFaceFoundAt( int x, int y, int w, int h)
{

    // X and Y will never be zero, the face detection routien can't find faces that start at the edge of the FOV
    // so if FaceAt.X (or Y) are zero, this is the first time a face has been found
    // We are going to swag that X and Y have to be 20% the width of the range up from 0 and down from Max

    if(w>0)
    {
        // you first wrote this code in gradeschool
        // you'd thought you'd never see it in the wild
        int Fudge = (int) (w * RangeFudge);
        int tmpWMin = 0 + Fudge;
        int tmpWMax = w - Fudge;

        if(tmpWMin < FaceAt.W_Min)
            FaceAt.W_Min = tmpWMin;

        if(tmpWMax > FaceAt.W_Max)
            FaceAt.W_Max = tmpWMax;
    }

    if(h>0)
    {
        // you first wrote this code in gradeschool
        // you'd thought you'd never see it in the wild
        int Fudge = (int) (h * RangeFudge);
        int tmpHMin = 0 + Fudge;
        int tmpHMax = h - Fudge;

        if(tmpHMin < FaceAt.H_Min)
            FaceAt.H_Min = tmpHMin;

        if(tmpHMax > FaceAt.H_Max)
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
      Serial2.printf(FaceLocationReportFormat,
                    FaceAt.IS,
                    FaceAt.X ,
                    FaceAt.Y ,
                    FaceAt.W ,
                    FaceAt.H 
      );
      
     // Debug write the message out according to the format defined in FaceReporting.h
#ifdef DEBUG_FACE
      Serial.printf(FaceLocationReportFormat,
                    FaceAt.IS,
                    FaceAt.X ,
                    FaceAt.Y ,
                    FaceAt.W ,
                    FaceAt.H 
      );
      Serial.println("\n");
#endif
   }

}