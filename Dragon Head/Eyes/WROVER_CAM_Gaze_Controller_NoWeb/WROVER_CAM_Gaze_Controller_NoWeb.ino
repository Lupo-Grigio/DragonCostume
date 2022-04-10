// Simple WROVER sketch to help verify ESP32s can talk on the i2c bus and is connected to something
// Using my Gaze Libaray
#include <Wire.h>   
#include <GazeControl.h>
#include "DragonOTA.h"

#define DEBUG_BAUD 115200
#define DEBUG // This is a sample after all library code should have serial.print wrapped in #if defined(DEBUG)

#if !defined(LED_BUILTIN)
int LED_BUILTIN = 5;// ESP32 LOLIN boards LED builtin is GPIO 5
#endif

int I2C_BUS1_SDA = 2;
int I2C_BUS1_SCL = 15;

int I2C_BUS2_SDA = 32;
int I2C_BUS2_SCL = 33;


int period = 500;

GazeControlStruct GC;


void setup() 
{
  Serial.begin(DEBUG_BAUD);
  //while (!Serial){ delay(10);}
     
  Serial.println("\n I2C Dual Busses Client test starting ");
  
  /* I2C Bus 1 setup */
  Serial.printf(" Beggining Bus1 begin returns %i \n", Wire.begin(CAM_CONTROLLER_ADDRESS,I2C_BUS1_SDA,I2C_BUS1_SCL,BUS_FREQ) ); // Join Bus 1 as an I2C SLAVE
  Wire.onReceive(*ReadFromBus1); // register event for when the master on bus 1 wants to write data to me
  Wire.onRequest(*WriteToBus1);     // register interrupt function for when master on bus 1 wants to recieve data from me 
  Serial.printf("2x I2C busses Bus 1 Slave ID %i SCA is on pin %i SDL is on pin %i \n",CAM_CONTROLLER_ADDRESS, I2C_BUS1_SDA, I2C_BUS1_SCL);
  Serial.printf(" Wire setClock for bus 1 returns %i \n",Wire.setClock(BUS_FREQ));

  /* BUS2 setup */
  Serial.printf(" Beggining Bus2 begin returns %i \n", Wire1.begin(CAM_CONTROLLER_ADDRESS,I2C_BUS2_SDA,I2C_BUS2_SCL,BUS_FREQ) ); // Join Bus 1 as an I2C SLAVE
  Wire1.onReceive(*ReadFromBus2); // register event for when the master on bus 1 wants to write data to me
  Wire1.onRequest(*WriteToBus2);     // register interrupt function for when master on bus 1 wants to recieve data from me 
  Serial.printf("2x I2C busses Bus2 Slave ID is %i SCA is on pin %i SDL is on pin %i \n",CAM_CONTROLLER_ADDRESS, I2C_BUS2_SDA, I2C_BUS2_SCL);
  Serial.printf(" Wire setClock for bus2 returns %i \n",Wire1.setClock(BUS_FREQ));

  Gaze.IS = 0;
  X = 0;
  Y = 0;
  Gaze.S = 0;
  Gaze.B = 0;
  Gaze.P = 0;
  Gaze.M = 0;
  Gaze.W = 0;
  Gaze.H = 0;

  DragonOTA_Setup(); // enable OTA

}

void loop() 
{
  vTaskDelay(period / portTICK_RATE_MS);
  // delay(period);
  
  void Handle_DragonOTA();

  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH);
  Serial.printf("Dual Client Status bump %d %s\n",LED_BUILTIN,digitalRead(LED_BUILTIN) ? "LOW" : "HIGH" );

}

// READ DATA FROM MASTER on BUS1 

void ReadFromBus1(int dummy)
{

  GazeFromI2C_Master ( &GC, &Wire); 

#if defined(DEBUG)
  Serial.print(" Read from Master On Bus 1 : ");
  String aStr = String(" ");
  BuildMessageString( &aStr, &GE );
  Serial.println(aStr.c_str());
#endif 
}

// BUS 1 is Primary, if we are asked to Write to Bus 1 by BUS1's Master, write empty
void WriteToBus1()
{
  GazeControlStruct GF;

  GF.IS = 0 ; 
  GF.X = 0.000 ; 
  GF.Y = 0.000 ; 
  GF.B = 0 ; 
  GF.P = 0 ; 
  GF.M = 0 ; 
  GF.W = 0 ; 
  GF.H = 0; 
 

  GazeToI2C_Master(&GF, &Wire);

#if defined(DEBUG)
  Serial.print(" Wrote to Master On Bus 1 : ");
  String aStr = String(" ");
  BuildMessageString( &aStr, &GF );
  Serial.println(aStr.c_str());
#endif 

}

/* READ DATA FROM MASTER on BUS2 */

void ReadFromBus2(int dummy)
{
  // Bus 2 just gets written too by bus 1
  // so whatever bus 2 want's to write, passive agressively
  // read it and throw it away
  GazeControlStruct GE; // empty gaze

  GazeFromI2C_Master ( &GE, &Wire1); 

#if defined(DEBUG)
  Serial.print(" Read from Master On Bus 22222 : ");
  String aStr = String(" ");
  BuildMessageString( &aStr, &GE );
  Serial.println(aStr.c_str());
#endif 
}

// Write to Bus2 whatever was read in by bus1 and put
void WriteToBus2()
{
 
  GazeToI2C_Master(&GC, &Wire1);

#if defined(DEBUG)
  Serial.print(" Wrote to Master On Bus 2 : ");
  String aStr = String(" ");
  BuildMessageString( &aStr, &GF );
  Serial.println(aStr.c_str());
#endif 
}
