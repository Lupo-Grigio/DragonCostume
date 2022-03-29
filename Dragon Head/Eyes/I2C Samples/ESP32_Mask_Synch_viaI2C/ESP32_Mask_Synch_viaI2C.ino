/* Monster M4sk synchronizer using an ESP32 over I2C
 * this is a Sample used simply to set up 2 I2C busses, 
 * M4sk on Wire (I2C Bus 0) is Controller and Sends Face data to ESP32 via Master Writer
 * M4sk on Wire1 (I2C Bus 1) is Subordinate
 * Other samples will show how to send data from the ESP32 to both Controller and Subordiante
 * NOTE: This will not work with ESP32 libraries < 1.0.8 and highly recommended to use ESP32 library v2.0.1 or newer
 */

 /*
for 5V bus
 * Connect a 4.7k resistor between SDA and Vcc
 * Connect a 4.7k resistor between SCL and Vcc
for 3.3V bus
 * Connect a 2.4k resistor between SDA and Vcc
 * Connect a 2.4k resistor between SCL and Vcc
Original sample code from: Kutscher07: Modified for TTGO TQ board with builtin OLED
*/



#include <Wire.h>    //use SCl & SDA

// For Freenove ESP32cam using GPIO 2 and 15...  for I2C 1 and 32 and 33 for I2C2
// FYI I _hate_ magic numbers, SDA and SCL are Magic numbers
// Ideally I should be able to create a subordinate Wire instance via Wire(SDAPIN,SCLPIN,ADDRESS)
#define SDA 2
#define SCL 15

#define SDA1 32
#define SCL1 33

#define LED_BUILTIN  2

#define SLAVE_ADDRESS 0x52 // our address on both I2C busses, so any master knows where to talk to
                           // I borrowed this ID from the Adafruit Arcade library that maps Wii Nunchuck to I2C


extern TwoWire Wire1;// use SCL1 & SDA1

#define I2CEyeControllerID  8

// STUBS for testing, remove this when running on M4SK
float eyeTargetX;
float eyeTargetY;
bool moveEyesRandomly;

// USE the same format string from the sister project to this one
// for example ../ESP32-Eye-Tracker/FaceRepoting.h
// ideally we would just include that here, but for a good explination
// why we can't google " arduino include headder files from other directories"
static const char EyeGaseRangeReportFormat[] = " Face found = %i X = %i Y = %i Blink = %i Pupil = %1 W = %i H = %i W_Min = %i H_Min = %i W_Max = %i H_Max = %i\n ";

typedef struct {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int Blink; // BLINK Boolean, 0 if not blink 1 if blink
  int Pupil; // pupil diameter value of 0 means the mask will set this to whatever the rest of the code does
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

} EyeGaseStruct;

bool HandelSerialInput();
void ParceIncomingString( String str, EyeGaseStruct* eye );
void LookAt(EyeGaseStruct* eye);

// Since the serial read buffer may contain an incmplete message, it needs
// to persist between calls to user_loop, thus this global.
// Once a complete message has been recieved and parced IncomingString will be 
// emptied so the next message can be processed during the next call to user_loop
String IncomingString = String(" ");

void setup()
{
  Serial.begin(115200);
  Wire.begin(SLAVE_ADDRESS); // I2C bus 1 setup SLAVE
  Wire1.begin(SLAVE_ADDRESS); // I2C Bus 2 setup  SLAVE

  // setup interrupt response functions to recieve data from M4sk
  // Only the primary monster M4sk should ever send data
  Wire.onReceive(*receivePrimaryEyesEvent); // register event for when the master writes
  Wire1.onReceive(*receiveSecondaryEyesEvent); // for symmetry.. the slave should never write

  // setup interrupt response functions to send data to M4Sks 
  // When the ESP32 detects a face or recieves a gase change event from
  // Bluetooth, or Web, or controller or whatever
  Wire.onRequest(*WriteToPrimaryEvent); // register event
  Wire1.onRequest(*WriteToSecondaryEvent); // register event

  pinMode(LED_BUILTIN, OUTPUT);


  Serial.printf("2x I2C slave busses setup %i\n",SLAVE_ADDRESS);
}

void loop()
{

  vTaskDelay(1000 / portTICK_RATE_MS);

  Serial.printf("Status Bump %d\n",digitalRead(LED_BUILTIN));
  
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ? LOW : HIGH);
}


// READ DATA FROM PRIMARY MASK
// the primary mask is sending us an eye state change message
// read it, parce it, send it to the secondary. 
// TODO read whole message
// TODO parce message
// TODO send to secondary
void receivePrimaryEyesEvent(int dummy)
{
  Serial.printf("Request Slave Recieve from I2C Bus Master Eyes would be on : %i",SLAVE_ADDRESS);
  
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  

}

void receiveSecondaryEyesEvent(int dummy)
{
    Serial.print("Something not right Request Slave Recieve request sent on 2nd I2C :");

}

// Write an Eye change message to primary M4SK
void WriteToPrimaryEvent()
{
  Serial.println("Master on I2C bus 1 called for slave to write eye state");
  
  String Msg = String(" ");
  EyeGaseStruct eye;
  // this data will get set somewhee else by the Face AI, or web or bluetooth
  eye.IS       = 001;
  eye.X        = 002;
  eye.Y        = 031;
  eye.Blink    = 401;
  eye.Pupil    = 051;
  eye.W        = 601;
  eye.H        = 001;
  eye.W_Min    = 001;
  eye.H_Min    = 001;
  eye.W_Max    = 001;
  eye.H_Max    = 1000;
  FormatEyeMessageString( Msg, &eye  );

  Wire.write(Msg.c_str()); 
}

// Write an Eye change message to Secondary M4SK
void WriteToSecondaryEvent()
{
  Wire1.write("hello "); // respond with message of 6 bytes
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
void ParceIncomingString( String str, EyeGaseStruct* eye  )
{
  str.trim(); // remove leading and trailing whitespace
  sscanf(str.c_str(),EyeGaseRangeReportFormat, 
                      &eye->IS,
                      &eye->X,
                      &eye->Y,
                      &eye->Blink,
                      &eye->Pupil,
                      &eye->W,
                      &eye->H,
                      &eye->W_Min,
                      &eye->H_Min,
                      &eye->W_Max,
                      &eye->H_Max
                      );

 /* Serial.printf(EyeGaseRangeReportFormat, 
                      eye->IS,
                      eye->X,
                      eye->Y,
                      eye->Blink,
                      eye->Pupil,
                      eye->W,
                      eye->H,
                      eye->W_Min,
                      eye->H_Min,
                      eye->W_Max,
                      eye->H_Max       
                      );*/
}

// point the eyes at a location
void FormatEyeMessageString(String str, EyeGaseStruct* eye  )
{
 char bug[1000];
 // String bug = String(" ");
  sprintf(bug, EyeGaseRangeReportFormat, 
                      eye->IS,
                      eye->X,
                      eye->Y,
                      eye->Blink,
                      eye->Pupil,
                      eye->W,
                      eye->H,
                      eye->W_Min,
                      eye->H_Min,
                      eye->W_Max,
                      eye->H_Max       
                      );
  str.remove(1); // just make sure the string is clean
  str.concat(bug);
}


void LookAt(EyeGaseStruct* eye)
{

// See user serial for really good input map, For I2C we Ignore all the range information because
// X and Y are coming from another Monster M4sk, and do not need adjusting


  float screenX,screenY;
    eyeTargetX = (float)eye->X;                       
    eyeTargetY = (float)eye->Y;

  //Serial.printf("Mapped values are %f , %f \n", eyeTargetX, eyeTargetY);
}

