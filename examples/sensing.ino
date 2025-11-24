#define DECODE_NEC
#include <Wire.h>

#include <Adafruit_MLX90393.h>
#include <Adafruit_APDS9960.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
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

#define PITIMESTWO 6.283185307;


Adafruit_NeoPixel_ZeroDMA pixels(NEOPIX_NUM, PIN_NEOPIXEL, NEO_GRB);


Adafruit_APDS9960 apds;
Adafruit_MLX90393 magSns = Adafruit_MLX90393(); //create magnetometer



struct {float x,y,z,norm;}mag;  //to store magnitude [uT]
unsigned long now=0;            //to store actual ms
unsigned long ledBlink=0;       //to store futur ledblink ms
uint32_t printTime = 0;
struct {uint16_t mVusb; uint16_t mVbat; bool usbPwrd; unsigned long mVreadTime;}brdInfo; //to store brp power info


uint8_t buzPwr = 127;            //to store buz magnitude
uint16_t proximity=0;
uint16_t cmax = 4096;            //to store buz magnitude
uint16_t lastC = 0;            //to store buz magnitude


unsigned long lightOn = 1;
unsigned long cint = 0;

unsigned long lastChangeTime = 0;  // Stocke le dernier moment où la direction a changé
unsigned long directionDurationMod1 = 1000;  // Durée en millisecondes
unsigned long directionDurationMod2 = 1000;  // Durée en millisecondes
int currentChoice = -1;  // Stocke le choix de direction actuel
bool isRDM = false;
unsigned long userMods = 0;
unsigned long runner = 0;

int8_t activeBuzPin;

double finit = 0.1;

double coupling = 0.004;


void setup() {

	randomSeed(analogRead(LIGHT_PIN));

	//finit = random(10,50)/10.0;

	finit = 1.2;

	pixels.begin(&sercom3, SERCOM3, SERCOM3_DMAC_ID_TX, PIN_NEOPIXEL, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT);
  pinMode(BUZ_EN_PIN, OUTPUT);
  digitalWrite(BUZ_EN_PIN, LOW);


	
	Wire.begin();
  Serial1.begin(4800); // Init SerialIR 
  Serial.begin(9600); // Init USB serial port

	delay(500);

	Serial1.println(__FILE__);
  Serial.println(__FILE__);

  magSns.begin_I2C(MLX_I2CADD, &Wire); // Init Mag sensor
  IrReceiver.begin(IR_RX_PIN, ENABLE_LED_FEEDBACK);

	analogWriteResolution(8);
	
	
  pinMode(BUZR_PIN, OUTPUT);    //BUZ R PWM PIN as output
	pinMode(BUZL_PIN, OUTPUT);    //BUZ L PWM PIN as output

	
  pinMode(APDS_INT_PIN, INPUT_PULLUP);
  pinMode(APDS_LED_PIN, OUTPUT);
	digitalWrite(APDS_LED_PIN, HIGH);
	
	apds.begin();
	apds.enableColor(true);

  activeBuzPin = -1;
  buzStop(0);

}

void loop() {

  now = millis();

	/* uint16_t r,g,b,c; */
	
	/* apds.getColorData(&r, &g, &b, &c); */
	
  
  /* Serial.print(" clear: "); */
  /* Serial.println(c); */
	/* Serial.println(cmax); */
	/* Serial.println(); */

	
	/* buzPwr = static_cast<double>(c)/4096*127; */

  
  if(now >= brdInfo.mVreadTime){
    brdInfo.mVreadTime = now+SUPPLY_READ_MS;
    readBrdPwr();

  }


	if(now>=printTime){
		Serial1.flush();
		/* Serial1.printf("%05.1lf\r\n",c); */
		printTime= now + 500;
	}
	

  if (lightOn%2 == 0){
			pixels.setPixelColor(NEOPIX_TOP, pixels.Color(255,255,255));
			pixels.show();
		}
	else{
		pixels.setPixelColor(NEOPIX_TOP, pixels.Color(0,0,0));
		pixels.show();
	}
	


  

  if (IrReceiver.decode()) {
		
    if(IrReceiver.decodedIRData.protocol == NEC){
			
      IrReceiver.printIRResultShort(&Serial);
      Serial.print("\n");
      delay(100);
      switch(IrReceiver.decodedIRData.command){
        

        case 0x11: //Button[2] TV NEC(code 1359) from grundig 19935 universal remote
        
					
          lightOn++;
          lightOn = lightOn %2;
          
        break;

      case 0x13: //Button[4] TV NEC(code 1359) from grundig 19935 universal remote
          Serial.print("mVusb:");Serial1.print(brdInfo.mVusb);
          Serial.print(",mVbat:");Serial1.println(brdInfo.mVbat);
        break;


        case 0x09: //Button[MUTE] TV NEC(code 1359) from grundig 19935 universal remote
          buzStop(0);
          buzPwr = 127;
          
        break;
        
        case 0x4A: //Button[MENU] TV NEC(code 1359) from grundig 19935 universal /* remote */
					uint16_t r,g,b,c;
					apds.getColorData(&r, &g, &b, &c);
          cmax=c;
					
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
					isRDM = true;
          userMods = 2;
          launchSearch();

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
	case 2: 
		if (isRDM) {
			launchSearch();
		}
		break;

	 case 3: 
		if (isRDM) {
			launchBALL();
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

	uint16_t r,g,b,c;
	
	apds.getColorData(&r, &g, &b, &c);
	
	buzPwr = static_cast<double>(c)/cmax*127;
	buzPwr = max(buzPwr,55);
	
	if (now - lastChangeTime >= directionDurationMod1) {

		if (currentChoice<2) {

			lastChangeTime = now; 
      directionDurationMod1 = random(400, 1201);
			
			buzCw(buzPwr, BUZR_PIN);
			buzCw(buzPwr, BUZL_PIN);
			Serial.println("RT: Straight");

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
          Serial.println("RT: To the LEFT");
          break;
  
        case 1: // To the RIGHT
          buzCw(buzPwr, BUZL_PIN);
          buzStop(BUZR_PIN);
          Serial.println("RT: To the RIGHT");
          break;
      }

		}
	  
  }
	
}


void launchSearch() {

	

	
	if (currentChoice == 1){

		if (now - lastChangeTime >= 20) {
			lastChangeTime = now;
			buzCcw(127, BUZL_PIN);
			buzStop(BUZR_PIN);

			uint16_t r,g,b,c;
			
			apds.getColorData(&r, &g, &b, &c);

			if (c>lastC){
				currentChoice = 1;
			}
			else{
				currentChoice = -1;
			}
			lastC = c;

		}
		

		
		

	} 
	else{

		if (now - lastChangeTime >= 20) {
			lastChangeTime = now;
			buzCcw(127, BUZR_PIN);
			buzStop(BUZL_PIN);

			uint16_t r,g,b,c;
			
			apds.getColorData(&r, &g, &b, &c);

			if (c>lastC){
				currentChoice = 1;
			}
			else{
				currentChoice = -1;
			}
			lastC = c;
			
		}

		

	}


	

	

}


void launchBALL() {
  // Vérifier si 'directionDirection' secondes se sont écoulées depuis le dernier changement
  if (now - lastChangeTime >= directionDurationMod1) {
    // Changer la direction aléatoirement toutes les x secondes
    currentChoice = (currentChoice+1) % 2;  // choix aléatoire entre 0 et 1 correspondant au déplacement
    lastChangeTime = now;  // Mettre à jour le temps du dernier changement
    directionDurationMod1 = random(200, 801);


    // Appliquer la direction actuelle (sans changer pendant 3 secondes)
    switch (currentChoice) {
      case 0: // To the LEFT
        buzCcw(110, BUZR_PIN);
        buzStop(BUZL_PIN);
        Serial.println("To the LEFT");
        break;

      case 1: // To the RIGHT
        buzCw(110, BUZL_PIN);
        buzStop(BUZR_PIN);
        Serial.println("To the RIGHT");
        break;
    }
  }
}



