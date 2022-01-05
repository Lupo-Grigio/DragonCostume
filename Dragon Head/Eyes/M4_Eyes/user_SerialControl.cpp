#if 1 // Change to 0 to disable this code (must enable ONE user*.cpp only!)

// Use I2C sensor to set look position of eyes
// TODO: 
// rename this to user_i2c_eyecontrol

#include "globals.h"
class Location {
public:
    int x, y, h, w, valid;
};
// I2C slave data read handeler
// adapted from the Wire Reciever arduino sample
// by Nicholas Zambetti <http://www.zambetti.com>
// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print("Recieved via I2C interrupt : ");
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}

// read x and y co-ords from I2C bus with SAMD in i2c master mode
// call with recieve int xval; int yval; XYviaI2C( &xval, &yval, deviceID);
int recieveXYviaI2C( int *X, int *Y, int deviceID)
{
  int valid = 0;
  int H = 0;
  int W = 0;
  *X = 0;
  *Y = 0;

 Wire.requestFrom(75, 5);    // request 5 bytes from slave device #75 (ESP32 CAM sets itself up as device 75
                             /*
                              * The data sent will be an X and Y cordinate within a square sized
                              * H units high and W units wide
                              * Byte 1 = are there co-ordinates 0 for no >0 for yes (send code sends an int 1)
                              * Byte 2 = x co-ordinate
                              * Byte 3 = y co-ordinate
                              * Byte 4 = W or Width of the co-ordinate plane
                              * Byte 5 = H or Height of the co-ordinate plane
                              */
  while(Wire.available())    // slave may send less than requested
  {
    valid = (int)Wire.read(); // first byte is "am I sending co-ordinate data 0 for no >0 for yes
    *X = (int)Wire.read(); // First byte is X
    *Y = (int)Wire.read(); // second byte is Y 
     W =  (int)Wire.read(); // Next byte is Width
     H =  (int)Wire.read(); // Last is the Height

    if (valid > 0)
    {
      Serial.print("i2C co-ords : found");         
      Serial.print(*X);
      Serial.print(" , ");
      Serial.print(*Y);
      Serial.print(" , ");
      Serial.print(H);
      Serial.print(" , ");
      Serial.print(W);
    }

  }
  
  return valid;
}

void SetEyeFocus(int x, int y)
{
    // Set values for the new X and Y.
  eyeTargetX = x;
  eyeTargetY = y;
}

// This file provides a crude way to "drop in" user code to the eyes,
// allowing concurrent operations without having to maintain a bunch of
// special derivatives of the eye code (which is still undergoing a lot
// of development). Just replace the source code contents of THIS TAB ONLY,
// compile and upload to board. Shouldn't need to modify other eye code.

// User globals can go here, recommend declaring as static, e.g.:
// static int foo = 42;

// Called once near the end of the setup() function. If your code requires
// a lot of time to initialize, make periodic calls to yield() to keep the
// USB mass storage filesystem alive.
void user_setup(void) {
  
    //adding Setup for I2C input support for controls and all that
 Wire.begin();                // join i2c bus with address random number 4 because M4SK
 // Wire.onReceive(receiveEvent); // register read eventfunction will be called when an I2C value is available

}

// Called periodically during eye animation. This is invoked in the
// interval before starting drawing on the last eye (left eye on MONSTER
// M4SK, sole eye on HalloWing M0) so it won't exacerbate visible tearing
// in eye rendering. This is also SPI "quiet time" on the MONSTER M4SK so
// it's OK to do I2C or other communication across the bridge.
// This function BLOCKS, it does NOT multitask with the eye animation code,
// and performance here will have a direct impact on overall refresh rates,
// so keep it simple. Avoid loops (e.g. if animating something like a servo
// or NeoPixels in response to some trigger) and instead rely on state
// machines or similar. Additionally, calls to this function are NOT time-
// constant -- eye rendering time can vary frame to frame, so animation or
// other over-time operations won't look very good using simple +/-
// increments, it's better to use millis() or micros() and work
// algebraically with elapsed times instead.
void user_loop(void) {

  int x_loc, y_loc = 0;
  
  if ( recieveXYviaI2C(  &x_loc, &y_loc, 75) > 0)
  {
    SetEyeFocus(x_loc, y_loc);
  }

}

#endif // 0
