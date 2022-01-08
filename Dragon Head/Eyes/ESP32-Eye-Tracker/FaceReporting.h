// the only way you know which pins they are on the package is the silk screen



// ideally you should grab the BAUD definitions from the project that this one talks to
// through something like #include "../common_comunication_settings.h" or #include "M4_Eyes/globals.h" 
// buuuut you just have to make sure the serial settings (Baud rate 8n1 etc.) are set the same

#define DEBUG_BAUD 115200
#define SERIAL2_BAUD 115200 // make sure this matches the rate at which your sender is transmitting

#define DEBUG_FACE         // write face locations to serial out
#undef DEBUG_FRAMERATE     // define to turn on writing framerate info to USB for every frame.

// Freemove WROVER most of the GPIO are used by the camera...
// GPIO 32 and 33 are not used by the camera
// GPIO 12  and 15 9  2 3 are not used by camera GPIO 12 is pulled hi
// GPIO 1, is'nt that usually USB? avoiding
// GPIO 2 looks good
// GPIO 14 is used below
// Picking GPIO 9 ... Buuuttt not available as a pin on the package
#define RXPIN 15
#define TXPIN 2

// NOTE: This code Assumes the X and Y co-ordinates are Positive ranges
// in that the camera is layed out int quadrent 1 where X and Y values are > 0
typedef struct {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
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

} FaceLocationStruct;

static const float WRangeFudge = 0.2; // these guesses based on testing
static const float HRangeFudge = 0.4; // these guesses based on testing
static const char FaceLocationReportFormat[] = " Face found = %i X = %i Y = %i W = %i H = %i \n ";
static const char FaceLocationRangeReportFormat[] = " Face found = %i X = %i Y = %i W = %i H = %i W_Min = %i H_Min = %i W_Max = %i H_Max = %i\n ";

void FaceReportingSetup();
void SetFaceFoundAt( int x, int y, int w, int h);
void SetNoFaceFound();
void FindFaces();
bool IsFaceFound();
void WriteFaceLocation();
