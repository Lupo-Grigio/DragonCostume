# DragonCostume 
Repository for all code and designs used in my massive dragon head and costume
Dragon Head 
  -Internal LED strips
    Running WLED on an esp32 to power the primary LED Light Strips within the head (Fork minimal changes) (Dev environment is Platform.io in VSCode)
    Several test FASTLED simulations of the head strips coded up in WoKwi - no production code

  -Controllable Animated Eyes
    Animated Eyes powered by an Adafruit Monster M4sk with custom code (fork) (Dev environment is both Arduino and Platform.io)
    Animated eye gaze direction is controlled by face detection done on an ESP32-WROVER-DEV Test framework is based on the ESP32-cam "camera" demo(Dev environment is both Arduino and Platform.io, but issue with Platform.io causes mem management failure in face detection)
    Some example code created for sscanf and parameter passing coded in wowki
    Physical Joystic control of the eyes powered by an ADAFRUIT Joy Feather and Feather wing : In Progress

  -Forward Cameras and Performer HUD
    External visability to performer provided by external cameras powered by ESP32-CAM modules sending their display info to : In Progress
    Heds Up Display Rig has yet to be speced and designed
    
  -Voice Commands
    Voice commands provided by dedicated arduino compatable voice command recognizer TBD
  
  -Voice Tweaker
    Standard voice changer running on an Arduino nano sense 33 (because I've got 2 of them and they have built in microphones)
    
  -Smoke System
    Smoke effect controlled by arduino TBD

  -Physical Sculpture and internal structure
    The Head itself is 3d modeled in blender
    The shell of the Mask is created thorugh pepacura designer https://tamasoft.co.jp/pepakura-en/
    The shell is cut both on a Silhouette Cameo 4 and a generic 60watt RUIDA powered Co2 laser
    Internal LED supports designed in Fusion 360 AND Tinkercad
    Animated Eye Mounts designed in Fusion 360 and Tinkercad
    All printed using PRUSASLICER
    
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

 

