# DragonCostume 
Repository for all code and designs used in my massive dragon head and costume

/Dragon Head 

  -/Lights 
    Internal LED strips
    Running WLED on an esp32 to power the primary LED Light Strips within the head (Fork minimal changes) (Dev environment is Platform.io in VSCode)
    The strip "Map" or definitions are created from a google sheets spreadsheet, The regions are created using several pivot tables

    -/WLED
      clone of WLED source from https://github.com/Aircoookie/WLED I _think_ this directory should be a fork.. but maybe if I actually make changes. this tweak is to get WLED to work on one of @romkey's discoball ESP32-dev boards. 

    -/WoWki Simulations
      Several test FASTLED simulations of the head strips coded up in WoKwi - no production code
      makes a convient(Where is my spell checker?) way to simulate the LED strips.

  -/Eyes Controllable Animated Eyes
    Animated Eyes powered by an Adafruit Monster M4sk with custom code (fork) (Dev environment is both Arduino and Platform.io)
    Animated eye gaze direction is controlled by face detection done on an ESP32-WROVER-DEV Test framework is based on the ESP32-cam "camera" demo(Dev environment is both Arduino and Platform.io, but issue with Platform.io causes mem management failure in face detection)
    Some example code created for sscanf and parameter passing coded in wowki
    Physical Joystic control of the eyes powered by an ADAFRUIT Joy Feather and Feather wing : In Progress

      -/M4_Eyes Based off of TonyLHansen's merge request into Adafruit's main. His code includes some extra eye controls and code for user.cpp files getting config info from the config file. Will be handy for me to set the BAUD rate between controller and the mask. https://github.com/adafruit/Adafruit_Learning_System_Guides/pull/915/commits/2b4827c8276a8fa8b2f16dbccf2c00a39fa4a631
    
      -/ESP32-Eye-Tracker
        Based on the ESP32-Cam example, Wifi removed, with web server assocated code removed. This Just captures video and does face detection, if it finds a face it writes it out to serial2
        Face detection running in the DEMO gets about 2 frames a second
        Face detection running in the Eye Tracker ...

      -/ESP32-Camera-Eye-tracker-DEMO
      Cloned from the ESP32-CAM/example and fixed via this commentary https://github.com/espressif/arduino-esp32/issues/5128. This directory contains the mirror code for M4_Eyes, this is the "controller" part. This does facial detection, and sends tracking data out serial port. This code is written for the Freenove ESP32-WROVER-DEV module with camera. https://www.amazon.com/Freenove-ESP32-WROVER-Bluetooth-Compatible-Tutorials/dp/B09BC5CNHM?ref_=ast_sto_dp This demo code runs a webserver and streams video, and contains VERBOSE debugging info written to the USB serial port. Face location data is written to serial 2 which is configured on pins 2 and 15, the format of the message and which pin is TX and RX are in the code. 

      -/Prototype Case
      3d printable case for the face tracking eye demo. Designed to hold the ESP32-CAM module and the Adafruit Monster M4sk. Originally designed in tinkercad by importing the case designed by adafruit for the M4sk. Tinkercad link:https://www.tinkercad.com/things/l33OCsHAKPb-eye-camera-mask-case also can be found on thingaverse: https://www.thingiverse.com/thing:5188566

      -/M4SK PWM remapping example
      Sample code for the Adafruit M4SK that just remaps the PWM port as Serial2 reads from it and writes to the USB serial monitor

      -/ESP32-CAM second serial sample
      A tiny project containing an example of setting up an ESP32-CAM to write data to pins 15 and 2, also a bit of beginner examples of passing pointers and structs. Developed and simulated on Wokwi. See the Wokwi-project.txt for link
  
  
  -Forward Cameras and Performer HUD
    External visability to performer provided by external cameras powered by ESP32-CAM modules sending their display info to : In Progress
    Heds Up Display Rig has yet to be speced and designed
    
  -Voice Commands
    Voice commands provided by dedicated arduino compatable voice command recognizer TBD
  
  -Voice Tweaker
    Standard voice changer running on an Arduino nano sense 33 (because I've got 2 of them and they have built in microphones)
    
  -Smoke System
    Smoke effect controlled by arduino TBD

  -/Physical 
    Sculpture and internal structure
    The Head itself is 3d modeled in blender
    The shell of the Mask is created thorugh pepacura designer https://tamasoft.co.jp/pepakura-en/
    The shell is cut both on a Silhouette Cameo 4 and a generic 60watt RUIDA powered Co2 laser
    Internal LED supports designed in Fusion 360 AND Tinkercad
    Animated Eye Mounts designed in Fusion 360 and Tinkercad
    All printed using PRUSASLICER

    -/Blender Models
      this is the start of the workflow, Just as it says, the blender source for the shell.
    
    -/Pepakura
      source files for the Shell generated using pepakura designer https://tamasoft.co.jp/pepakura-en/

    -/Laser
      RLD and DXF files for the shell cut paths

    -Internal LED supports
    The 3dprinted internal struts for the LEDs where designed in Fusion 360, then modified in tinkercad to ad direction registration and numbering then pulled into PrusaSlicer to prep for 3d printing. 



  -Cosmetic wrapping
    Reflective pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    
Top Hat
    LED strip and ElWire controlled by TBD (Likely an ESP32 running WLED to synch with the dragon head
    Reflective pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    
Swagger Stick
    Bulb Internal LEDs controlled by TBD (likely an ESP32 runing WLED to synch with the dragon head
    physical controls on the stick and white LEDs in the base, as well as discoball magnetic attachment to base. 
    
Jacket
    Reflective pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    "Scales" and supports for the reflective patterns designed in Fusion360 and tinkercad
    "Scales" and supports Sliced and printed using PrusaSlicer
    Jacket Internal lights powered by an ESP32 running WLED
    
Vest
    Vest pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    "Scales" and supports for the reflective patterns designed in Fusion360 and tinkercad
    "Scales" and supports Sliced and printed using PrusaSlicer
    
Broken Heart
    broken heart pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    Internal lighting controlled by an arduino nano running simple FASTLED animations
    
Pants
    Pants pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    "Scales" and supports for the reflective patterns designed in Fusion360 and tinkercad
    "Scales" and supports Sliced and printed using PrusaSlicer

Gloves
    Glove pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    "Scales" and supports for the reflective patterns designed in Fusion360 and tinkercad
    "Scales" and supports Sliced and printed using PrusaSlicer
    Button controls integrated into the gloves managed by an arduino nano and sent to various devices

Shoes
    Vest pattern created in Shilouette studio and cut on a Shilouette CAMEO 4
    "Scales" and supports for the reflective patterns designed in Fusion360 and tinkercad
    "Scales" and supports Sliced and printed using PrusaSlicer

 

