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


#define DDS_RESOLUTION 4294967296UL  // 2^32
#define DDS_FREQENCY   1000.0 //Hz

Adafruit_NeoPixel_ZeroDMA pixels(NEOPIX_NUM, PIN_NEOPIXEL, NEO_GRB);


volatile struct{
  uint32_t  accumulator;  // Heart of dds
  uint32_t  increment;    // Tunning freqency increment
  uint8_t   offset;       // Offset added for phase adjust
  uint8_t   index;
	int8_t    sign;           
}phase={.accumulator=0, .increment=0, .offset=0, .sign = 1};

#define HOWMANYCOLORS 6
uint8_t colors[HOWMANYCOLORS][3] = { //Must be a power of 2
  {255,0,0}, // Red
	{255,255,0},  // 
  {0,255,0}, // Green
	{0,255,255},  // Cyan
  {0,0,255}, // Blue
	{255,0,255}, // Magenta
};




Adafruit_APDS9960 apds;
Adafruit_MLX90393 magSns = Adafruit_MLX90393(); //create magnetometer



struct {float x,y,z,norm;}mag;  //to store magnitude [uT]
unsigned long now=0;            //to store actual ms
unsigned long ledBlink=0;       //to store futur ledblink ms
uint32_t printTime = 0;
struct {uint16_t mVusb; uint16_t mVbat; bool usbPwrd; unsigned long mVreadTime;}brdInfo; //to store brp power info


uint8_t buzPwr = 127;            //to store buz magnitude
uint16_t proximity=0;

unsigned long lightOn = 0;
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


void TCC2_Handler() {
  if (TCC2->INTFLAG.bit.OVF) {
    TCC2->INTFLAG.bit.OVF = 1;
    /*
    phase.accumulator += phase.increment;
    uint8_t colorIndex = (phase.accumulator >> 29)%3; // Using 3High bit as color pointer
    pixels.setPixelColor(NEOPIX_TOP, colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]);
    colorIndex += phase.offset; // Adding offset
    colorIndex %= sizeof(colors);
    pixels.setPixelColor(NEOPIX_BOT, colors[colorIndex][0], int(colors[colorIndex][1]*1.365), int(colors[colorIndex][2]/1.05));
    pixels.show();
    */
    phase.accumulator += phase.increment;
    if (phase.accumulator < phase.increment){ //Overflow
			
      phase.index = (phase.index + phase.sign);

			if (phase.index==HOWMANYCOLORS){
				phase.index = 0;

			}
			else if(phase.index>HOWMANYCOLORS){
				phase.index = HOWMANYCOLORS-1;
			}
			//phase.index %= HOWMANYCOLORS;

			pixels.setPixelColor(NEOPIX_BOT, colors[phase.index][0], int(colors[phase.index][1]), int(colors[phase.index][2]));

      uint8_t offsetIndex = (phase.index + phase.offset)%HOWMANYCOLORS;
      pixels.setPixelColor(NEOPIX_TOP, colors[offsetIndex][0], int(colors[offsetIndex][1]), int(colors[offsetIndex][2]));
      pixels.show();
    }
  }
}

float colorFreqence(double freq){  //Return the therorical output freq
  phase.increment = uint32_t((abs(freq)*DDS_RESOLUTION)/DDS_FREQENCY);
	phase.sign = (freq<0) ? -1:1;
  return (float)((phase.increment*DDS_FREQENCY)/DDS_RESOLUTION);
}

void setup() {

	randomSeed(analogRead(A0));

	//finit = random(10,50)/10.0;

	finit = 1.2;

	pixels.begin(&sercom3, SERCOM3, SERCOM3_DMAC_ID_TX, PIN_NEOPIXEL, SPI_PAD_2_SCK_3, PIO_SERCOM_ALT);
  pinMode(BUZ_EN_PIN, OUTPUT);
  digitalWrite(BUZ_EN_PIN, LOW);

  colorFreqence(finit);
  setupTCC2();
	
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
  
  if(now >= brdInfo.mVreadTime){
    brdInfo.mVreadTime = now+SUPPLY_READ_MS;
    readBrdPwr();

  }

	if(now>=printTime){
		Serial1.flush();
		Serial1.printf("%05.4lf\r\n",coupling);
		printTime= now + 5000;
	}
	

	uint16_t r,g,b,c;
	
	apds.getColorData(&r, &g, &b, &c);
	
	r = static_cast<double>(r)/c*255;
	g = static_cast<double>(g)/c*255;
	b = static_cast<double>(b)/c*255;

	r = (r < 45) ? 0 : 255;
	g = (g < 75) ? 0 : 255;
	b = (b < 110) ? 0 : 255;



	uint32_t minDifference = 300000;
	int closestColor = 0;
	
  for (int i = 0; i < 6; i++) {

    uint32_t difference = colorDifference(r,g,b,colors[i][0],colors[i][1],colors[i][2]);

    if (difference < minDifference) {
      minDifference = difference;
      closestColor = i;
    }
  }
	

	
	
	double phdiff = (static_cast<double>(closestColor)-static_cast<double>(phase.index)) / static_cast<double>(HOWMANYCOLORS) * PITIMESTWO;
  finit += coupling * sin(phdiff);
	
	colorFreqence(finit);


	/* Serial.print(closestColor); */
	/* Serial.print(phase.index);
	/* Serial.println(); */
	
	Serial.print(phase.index);
	Serial.print("  ");
	Serial.print(phase.increment);
	Serial.print("  ");
	Serial.print(finit);
	Serial.print("  ");


	/* Serial.println(); */

	Serial.println();
	
  

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
        
        case 0x80: //Button[Vol+] TV NEC(code 1359) from grundig 19935 universal remote
          if(buzPwr+10<128){
            buzPwr = buzPwr + 10;
          }

					coupling = coupling + 0.0005;
						
          Serial.print("buzPwr:");Serial.print(buzPwr);
        break;

        case 0x81: //Button[Vol-] TV NEC(code 1359) from grundig 19935 universal remote
          if(buzPwr-10>50){
            buzPwr = buzPwr - 10;
          }

					coupling = coupling - 0.001;
          
          Serial.print("buzPwr:");
          Serial.print(buzPwr);
          
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
			launchDIFF();
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




uint32_t colorDifference(uint32_t r1,uint32_t g1,uint32_t b1, uint32_t r2, uint32_t g2, uint32_t b2) {

	
  uint32_t diff = (r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2);
  
  return diff;  // Return the squared Euclidean distance
}



void setupTCC2(void){
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3) |
                      GCLK_CLKCTRL_GEN_GCLK0 |
                      GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  TCC2->CTRLA.bit.SWRST = 1;
  while (TCC2->SYNCBUSY.bit.SWRST);

  TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NFRQ;
  while (TCC2->SYNCBUSY.bit.WAVE);

  // 48 MHz / prescaler 8 = 6 MHz; 6 MHz / 6000 = 1 kHz
  TCC2->CTRLA.reg |= TCC_CTRLA_PRESCALER_DIV8;
  TCC2->PER.reg = 6000;
  while (TCC2->SYNCBUSY.bit.PER);

  TCC2->INTENSET.bit.OVF = 1;
  NVIC_EnableIRQ(TCC2_IRQn);

  TCC2->CTRLA.bit.ENABLE = 1;
  while (TCC2->SYNCBUSY.bit.ENABLE);
}
