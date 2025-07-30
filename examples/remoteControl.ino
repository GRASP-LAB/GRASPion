#define DECODE_NEC
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MLX90393.h>
#include <Adafruit_APDS9960.h>
#include <IRremote.hpp>

#define NEOPIX_PIN  11    //Neopixel Arduino pinName
#define NEOPIX_NUM  2     //Number on neopix 0=BOTTOM 1=TOP
#define NEOPIX_BOT  0     //Bottom led address in neopîx strip
#define NEOPIX_TOP  1     //Top led address in neopîx strip
#define LIGHT_PIN   A0    //Light Sensor Arduino pinName
#define MLX_I2CADD  0x10  //MLX90393xLQ-ABA-011 I2C address
#define IR_RX_PIN   7
#define BUZ_EN_PIN  12    //buzzers drivers enable pin
#define BUZR_PIN    2
#define BUZL_PIN    3
#define SUPPLY_READ_MS  2000 //ms bettween read powr info from the board
#define BLINK_MS        200 //Bink LED time [ms]
#define APDS_INT_PIN  8
#define APDS_LED_PIN  10


Adafruit_APDS9960 apds;

Adafruit_NeoPixel pixels(NEOPIX_NUM, NEOPIX_PIN, NEO_GRB + NEO_KHZ800); //create neopixels strip
Adafruit_MLX90393 magSns = Adafruit_MLX90393(); //create magnetometer

struct {float x,y,z,norm;}mag;  //to store magnitude [uT]
unsigned long now=0;            //to store actual ms
unsigned long ledBlink=0;       //to store futur ledblink ms
struct {uint16_t mVusb; uint16_t mVbat; bool usbPwrd; unsigned long mVreadTime;}brdInfo; //to store brp power info

uint8_t buzPwr = 127;            //to store buz magnitude


unsigned long lastChangeTime = 0;  // Stocke le dernier moment où la direction a changé
unsigned long directionDurationMod1 = 1000;  // Durée en millisecondes
unsigned long directionDurationMod2 = 1000;  // Durée en millisecondes

int currentChoice = -1;  // Stocke le choix de direction actuel
bool isRDM = false;
unsigned long userMods = 0;
unsigned long runner = 0;

int8_t activeBuzPin;


void setup() {

	
	Wire.begin();
  Serial1.begin(4800); // Init SerialIR 
  Serial.begin(9600); // Init USB serial port4
	
	delay(500);

	Serial1.println(__FILE__);
  Serial.println(__FILE__);

	pixels.begin(); // Init NeoPixel strip object (REQUIRED)
  magSns.begin_I2C(MLX_I2CADD, &Wire); // Init Mag sensor
  IrReceiver.begin(IR_RX_PIN, ENABLE_LED_FEEDBACK);

	pinMode(BUZ_EN_PIN, OUTPUT);    //BUZ EN PIN as output
  digitalWrite(BUZ_EN_PIN, LOW);  //Disable Buz

	analogWriteResolution(8);
	
  pinMode(BUZR_PIN, OUTPUT);    //BUZ R PWM PIN as output
	pinMode(BUZL_PIN, OUTPUT);    //BUZ L PWM PIN as output

	
	randomSeed(analogRead(A0));
	

  activeBuzPin = -1;
  buzStop(0);


}

void loop() {

  now = millis();
  
  if(now >= brdInfo.mVreadTime){
    brdInfo.mVreadTime = now+SUPPLY_READ_MS;
    readBrdPwr();
  }


  if (IrReceiver.decode()) {
    if(IrReceiver.decodedIRData.protocol == NEC){
      IrReceiver.printIRResultShort(&Serial);
      Serial.print("\n");
			Serial.print(buzPwr);
      delay(100);
      switch(IrReceiver.decodedIRData.command){
        
			case 0x10: //Button[1] TV NEC(code 1359) from grundig 19935 universal remote
				buzPwr = 127;
				
        break;
				
			case 0x11: //Button[2] TV NEC(code 1359) from grundig 19935 universal remote
				
				buzPwr = 121;
        
        break;
				
			case 0x12: //Button[3] TV NEC(code 1359) from grundig 19935 universal remote
				
				buzPwr = 115;
				
				break;
				
      case 0x13: //Button[4] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 109;
				
        break;
        
			case 0x14: //Button[5] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 103;
				
        break;

			case 0x15: //Button[6] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 97;
				
        break;

			case 0x16: //Button[7] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 91;
				
        break;

			case 0x17: //Button[8] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 85;
				
        break;

			case 0x18: //Button[9] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 79;
				
        break;

		 case 0x1A: //Button[0] TV NEC(code 1359) from grundig 19935 universal remote

				buzPwr = 72;
				
        break;

			case 0x09: //Button[MUTE] TV NEC(code 1359) from grundig 19935 universal remote
          
        break;
        
        case 0x4A: //Button[MENU] TV NEC(code 1359) from grundig 19935 universal remote
					break;

        case 0x00: //Button[UP ARROW] TV NEC(code 1359) from grundig 19935 universal remote
						buzStop(0);
						buzCw(buzPwr, BUZR_PIN);
						buzCw(buzPwr, BUZL_PIN);

        break;
        
        case 0x01: //Button[DOWN ARROW] TV NEC(code 1359) from grundig 19935 universal remote
          buzStop(0);
          isRDM = false;
          activeBuzPin = -1;
        break;
        
        case 0x02: //Button[RIGHT ARROW] TV NEC(code 1359) from grundig 19935 universal remote
          buzStop(0);
          buzCw(buzPwr, BUZL_PIN); 
        break;
        
        case 0x03: //Button[LEFT ARROW] TV NEC(code 1359) from grundig 19935 universal remote
          buzStop(0);
          buzCw(buzPwr, BUZR_PIN);
        break;

        case 0x58: //Button[SLEEP] TV NEC(code 1359) from grundig 19935 universal remote
          
        break;
        
        case 0x0F: //Button[DISP] TV NEC(code 1359) from grundig 19935 universal remote

        break;
        
        case 0x0A: //Button[AV] TV NEC(code 1359) from grundig 19935 universal remote
          isRDM = true;
          userMods = 1;
          launchRT();
        break;
        
      }
    }
    IrReceiver.resume();
  }

  switch (userMods) {
	case 1: 
		if (isRDM) {
			launchRT();
		}
		break;



  }





}

void readBrdPwr(){
  char ret[5];
  static bool serialBegun = false;
  Wire.requestFrom(0x36, sizeof(ret));
  uint8_t i =0;
  while(Wire.available()){
    ret[i++] = Wire.read();
  }
  if(i==5){
    brdInfo.mVusb = (ret[0]<<8) + ret[1];
    brdInfo.mVbat = (ret[2]<<8) + ret[3];
    brdInfo.usbPwrd = (brdInfo.mVusb >= 4500) ? true :false;
  }

  if(brdInfo.usbPwrd){
    if(!serialBegun){
      Serial.begin(115200);
      serialBegun = true;
    }
  }else{
    if(serialBegun){
      Serial.end();
      serialBegun = false;
    }
  }
}

void buzStop(uint8_t pin){
  uint8_t pwr = 127;
  if(!pin){
    analogWrite(BUZR_PIN, pwr);
    analogWrite(BUZL_PIN, pwr); 
    digitalWrite(BUZ_EN_PIN, LOW);
  }else{
    analogWrite(pin, pwr);
  }
}

void buzCw(uint8_t val, uint8_t pin){
  uint8_t pwr;
  digitalWrite(BUZ_EN_PIN, HIGH);
  pwr = 127 + min(abs(val),128);
  analogWrite(pin, pwr); //(from 127->255 [0%->100%])
}

void buzCcw(uint8_t val, uint8_t pin){
  uint8_t pwr;
  digitalWrite(BUZ_EN_PIN, HIGH);
  pwr = 127 - min(abs(val),127); 
  analogWrite(pin, pwr); // (from 127 to 0 [0%->-100%])
}



void launchRT() {
  // Vérifier si 'directionDirection' secondes se sont écoulées depuis le dernier changement
  		
	if (now - lastChangeTime >= directionDurationMod1) {

		if (currentChoice<2) {

			lastChangeTime = now; 
      directionDurationMod1 = random(400, 1201);
			
			buzCw(buzPwr, BUZR_PIN);
			buzCw(buzPwr, BUZL_PIN);
			//Serial.println("RT: Straight");

			currentChoice = 2;
			
		}

		else{
			
      currentChoice = random(0, 2);  
      lastChangeTime = now; 
      directionDurationMod1 = random(400, 1201);
      
      switch (currentChoice) {
        case 0: // To the LEFT
          buzCw(buzPwr, BUZR_PIN);
          buzStop(BUZL_PIN);
          //Serial.println("RT: To the LEFT");
          break;
  
        case 1: // To the RIGHT
          buzCw(buzPwr, BUZL_PIN);
          buzStop(BUZR_PIN);
          //Serial.println("RT: To the RIGHT");
          break;
      }

		}
	  
  }
	

}




