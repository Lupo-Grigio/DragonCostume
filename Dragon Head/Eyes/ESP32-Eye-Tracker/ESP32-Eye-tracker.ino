#include "esp_camera.h"

// ideally you should grab the BAUD definitions from the project that this one talks to
// through something like #include "../common_comunication_settings.h" or #include "M4_Eyes/globals.h" 
// buuuut you just have to make sure the serial settings (Baud rate 8n1 etc.) are set the same

// Detect Faces in field of view write the location to Hardware Serial 2
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"




// Freemove WROVER most of the GPIO are used by the camera...
// GPIO 32 and 33 are not used by the camera
// GPIO 12  and 15 9  2 3 are not used by camera GPIO 12 is pulled hi
// GPIO 1, is'nt that usually USB? avoiding
// GPIO 2 looks good
// GPIO 14 is used below
// Picking GPIO 9 ... Buuuttt not available as a pin on the package

// the only way you know which pins they are on the package is the silk screen
#define RXPIN 15
#define TXPIN 2

#define DEBUG_BAUD 115200
#define SERIAL2_BAUD 115200 // make sure this matches the rate at which your sender is transmitting



void FaceFinderSetup();
void FindFaces();
bool IsFaceFound();
void GetFaceLocation( int *x, int *y, int *w, int *h);
void requestFaceLocationEvent();

void setup() {
  Serial.begin(DEBUG_BAUD);
  Serial.setDebugOutput(true);

  Serial.print("\nFace Dector for ESP32");
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  FaceFinderSetup();

  Serial.println("Camera Ready! ");


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
  
  Serial.printf("\n ESP32 Face Detector ready to go writing face locations to TX on %d and RX on %d\n",TXPIN,RXPIN);
}

// do not perform time-consuming tasks inside this function,
// do them elsewhere and simply read the data you wish to
// send inside here.
void loop() {
  // put your main code here, to run repeatedly:
  FindFaces();

  WriteFaceLocation();
  // with the demo web server, running face detection gets me like 
  // 2 frames a second. so it does not do much good to check any faster than that
  int framerate = 2;
    delay(1000 / framerate );
}

// function that executes from the main loop of the program

void WriteFaceLocation()
{
  
  int  Face_X = 0;
  int  Face_Y = 0;
  int  Face_H = 0;
  int  Face_W = 0;
  int LocationArray[5]; // Found
                         // X
                         // Y
                         // W
                         // H
  if (IsFaceFound())
  {
    // yes I know I should have just wirtten this function to pass the address of the array
    // but guess what, moving small amounts of data on and off the stack is faster!
    // and the optimizer will do it's job to pack this code real tight
    GetFaceLocation(  &Face_X,
                      &Face_Y,
                      &Face_H,
                      &Face_W );
      
      LocationArray[0] = 1;
      LocationArray[1] = Face_X;
      LocationArray[2] = Face_Y;
      LocationArray[3] = Face_W;
      LocationArray[4] = Face_H;                        
 

      Serial2.printf(" Face found = %i X = %i Y = %i W = %i H = %i \n ",
                    LocationArray[0],
                    LocationArray[1],
                    LocationArray[2],
                    LocationArray[3],
                    LocationArray[4]
      );
      
      Serial.printf(" Wrote to Serial face found = %i X = %i Y = %i W = %i H = %i \n ",
                    LocationArray[0],
                    LocationArray[1],
                    LocationArray[2],
                    LocationArray[3],
                    LocationArray[4]
      );
      Serial.println("\n");
   }

}
