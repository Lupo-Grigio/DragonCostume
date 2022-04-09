
#include <Wire.h>    //use SCl & SDA

#define DEBUG // This is a sample after all library code should have serial.print wrapped in #if defined(DEBUG)

#include <GazeControl.h>

#define DEBUG_BAUD 115200

#if !defined(LED_BUILTIN)
#define LED_BUILTIN 5 // ESP32 LOLIN boards LED builtin is GPIO 5
#endif


int period = 1000;



void setup() 
{

  
  Serial.begin(DEBUG_BAUD);
  
  Serial.printf("Mosnter M4SK about to JOIN I2C on SDA %i SCL %i at Freq %i \n",SDA,SCL,BUS_FREQ);
  //Wire.begin();
  Serial.printf(" Wire begin bus2 returns \n");
  Wire.begin(SDA,SCL); // Join the I2C bus as master
  Serial.printf(" Wire setClock bus2 returns %i \n");
  Wire.setClock(BUS_FREQ);
  Serial.println("Monster M4sk about to scan I2C bus2 ");

}

void loop() 
{
  //vTaskDelay(period / portTICK_RATE_MS);
  delay(period);
  ScanBus();
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH);
  Serial.printf("Status bump bus2 %d %s\n",LED_BUILTIN,digitalRead(LED_BUILTIN) ? "LOW" : "HIGH" );

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
