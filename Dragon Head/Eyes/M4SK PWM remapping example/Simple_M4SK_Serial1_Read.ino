
Uart Serial1( &sercom3, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;
void SERCOM3_0_Handler() { Serial1.IrqHandler(); }
void SERCOM3_1_Handler() { Serial1.IrqHandler(); }
void SERCOM3_2_Handler() { Serial1.IrqHandler(); }
void SERCOM3_3_Handler() { Serial1.IrqHandler(); }

#define BAUDRATE_1 9600 // (put this in the config file)
#define BAUDRATE_DEBUG 115200
void setup() {
  
  Serial.begin(BAUDRATE_DEBUG);
  while (!Serial)
     delay(10);
  Serial.println("\nHARD Serial1 Reader for Monster M4SK");
  
  // define pin modes for tx, rx:
  // pinMode(RXPIN, INPUT);
  // pinMode(TXPIN, OUTPUT);

  
  // Serial.printf("\n Beginning Hardware serial port 2 TX on pin %d, RX on pin %d\n",RXPIN,TXPIN);
  
  Serial.printf("\n Beginning Hardware serial port 1 TX and RX are %d %d\n",PIN_SERIAL1_TX,PIN_SERIAL1_RX);
  // If I knew what those pins where defined as....
  
  //Serial1.begin(9600,SERIAL_8N1, RXPIN, TXPIN);
  // alas throw caution to the wind...
  Serial1.begin(9600);
  
  while (!Serial1)
  {
    Serial.print(".");
  }
  Serial.println("\n Serial1 started successfully at %d Baud and ready to read");
}

void loop() {
   if (Serial1.available()>0)
  {
    char c = Serial1.read();
    Serial.print(c);
  }
 delay(10);
}
