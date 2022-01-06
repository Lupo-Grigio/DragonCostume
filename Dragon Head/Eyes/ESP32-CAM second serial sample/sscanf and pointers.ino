
// Do you need to read data from a serial port?
// Do you need to parce various values from a text string?
// you do NOT have to strictly format your input stream to get valuable data

String IncomingString;

// example of handeling serial input and the fact
// that the buffer might be emptied before you actually 
// reach the end of your "message"
bool HandelSerialInput()
{
  char c;
  bool ret = false;
  if(Serial1.available()>0) // hey there is something there
  {
    while(Serial1.available()>0)
    {
          IncomingString += (char)Serial1.read();
    }
    
    if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
    {
      FaceLocationStruct FaceAt;
      ret = true;
      Serial.print("Full Message : ");
      Serial.println(IncomingString);
      
      ParceIncomingString(IncomingString,  &FaceAt);
      Serial.printf(" Parsed data looks like :%i,%i,%i,%i,%i\n",
                            FaceAt.IS,
                            FaceAt.X,
                            FaceAt.Y,
                            FaceAt.W,
                            FaceAt.H  
                    );
      ret = true;
      IncomingString.remove(1); // empty the string
      moveEyesRandomly = false; // stop random eye movement TODO: Time this to prevent jerking
      LookAt( &FaceAt );
    }
    else
    {
      Serial.print(IncomingString);
      Serial.println(" Reached end of buffer before EOM");
    }
  }
  return(ret); 

}

typedef struct {
  int IS; // != 1 face not found or data tx/rx muck up co=ords are invalid
  int X; // X co-ord of face
  int Y; // Y co-ord of face
  int W; // Width of field
  int H; // Height of field
} FaceLocationStruct;

bool HandelSerialInput();
void ParceIncomingString( String str, FaceLocationStruct* fat );
void LookAt(FaceLocationStruct* fat);


bool HandelSerialInput()
{
char c;
bool ret = false;

  if(IncomingString.endsWith("\n") || IncomingString.endsWith("\n "))
  {
    FaceLocationStruct FaceAt;

    Serial.print("Full Message : ");
    Serial.println(IncomingString);
    
    ParceIncomingString(IncomingString,  &FaceAt);
    Serial.printf(" Parsed data looks like :%i,%i,%i,%i,%i\n",
                          FaceAt.IS,
                          FaceAt.X,
                          FaceAt.Y,
                          FaceAt.W,
                          FaceAt.H  
                  );
    ret = true;
    IncomingString.remove(1); // empty the string
    bool moveEyesRandomly = false;
    LookAt( &FaceAt );
  }
  else
  {
    Serial.print(IncomingString);
    Serial.println(" Reached end of buffer before EOM");
  }
  return(ret); 

}

void ParceIncomingString( String str, FaceLocationStruct* fat )
{
  str.trim(); // remove leading and trailing whitespace
  
  /*%i signed integer
    %u unsigned integer
    %d Decimal value
    %f float value (+-nums.nums)
    %c single char
    %6c 6 chars
    %s string ... watch out for this one it is powerful but could bite you in places you did'nt know you had

 
  sscanf(str.c_str()," Face found = %i X = %i Y = %i W = %i H = %i \n", &f,&x,&y,&w,&h);
  Serial.printf(" scanf returned %i X = %i Y = %i W = %i H = %i \n", f,x,y,w,h);
  */

  sscanf(str.c_str()," Face found = %i X = %i Y = %i W = %i H = %i \n", 
                      &fat->IS,
                      &fat->X,
                      &fat->Y,
                      &fat->W,
                      &fat->H
                      );
  Serial.printf(" Pointer Playground returned %i X = %i Y = %i W = %i H = %i \n", 
                      fat->IS,
                      fat->X,
                      fat->Y,
                      fat->W,
                      fat->H
  );

}

#define DISPLAY_SIZE 240

void LookAt(FaceLocationStruct* fat)
{
  float eyeTargetX = (float)constrain(
                map(fat->X, 1, fat->W, 1, DISPLAY_SIZE),    // where would X be if it was in a range of DISPLAY_SIZE
                1,DISPLAY_SIZE);
  float eyeTargetY = (float)- constrain(                                 // see commentary in rendering code for why -y
                map(fat->Y, 1, fat->H, 1, DISPLAY_SIZE),    // where would X be if it was in a range of DISPLAY_SIZE
                1,DISPLAY_SIZE);
  Serial.printf("Mapped values are %f , %f \n", eyeTargetX, eyeTargetY);

}

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200); // Any baud rate should work


  IncomingString += "Face found = 1 X = 131 Y = 131 W = 240 H = 320 \n";

  int f,x,y,w,h = 0;

  char fs[50],xs[20],ys[20],ws[20],hs[20];

  Serial.printf("Scanning :%s:\n",IncomingString.c_str());

  sscanf(IncomingString.c_str(),"Face found = %i X = %i Y = %i W = %i H = %i \n", &f,&x,&y,&w,&h);

  Serial.printf(" scanf returned %i X = %i Y = %i W = %i H = %i \n", f,x,y,w,h);

  sscanf(IncomingString.c_str(),"Face found = %d X = %d Y = %d W = %d H = %d\n", &f,&x,&y,&w,&h);

  Serial.printf(" second method returned %d X = %d Y = %d W = %d H = %d \n", f,x,y,w,h);

  sscanf(IncomingString.c_str(),"Face found = %s X = %s Y = %s W = %s H = %s\n", &fs, &xs, &ys, &ws, &hs);

  Serial.printf(" thirdnd method returned %s X = %s Y = %s W = %s H = %s \n", fs,xs,ys,ws,hs);



}

void loop() {
  HandelSerialInput();
  // put your main code here, to run repeatedly:

}
