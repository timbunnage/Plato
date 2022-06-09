#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define DATA_PIN    22
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];

#define NUM_SENSORS 4

#define BRIGHTNESS          220
#define FRAMES_PER_SECOND   60


const int LOWEST_INS = 10;            // Lowest led Intensity
int ledIns[NUM_LEDS];         // LED Intensities range [LOWEST_INS-100]

const int snsrCentre[NUM_SENSORS] = {3,9,15,21};  // Location of sensors compared to leds, idx corresponds with sensors array
#define SENSE_RANGE 3             // range of sensors either side



const int sensors[NUM_SENSORS] = { A5, A4, A1, A3};         // Stores sensor pins
const int CENTRE_SENSOR = A0;                               // Store pin of sensor in centre of plate

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void setup()
{
  delay(2000);
  
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  for (int i=0; i<NUM_SENSORS; i++) {pinMode(sensors[i], INPUT);}
  pinMode(CENTRE_SENSOR, INPUT);


  // 20% led intensity
  for (int i=0; i<NUM_LEDS; i++) { 
    ledIns[i] = LOWEST_INS; 
  }
  Serial.println(ledIns[0]);
  Serial.println(ledIns[1]);
  Serial.println(ledIns[2]);
  
}

int timer = 20;

void loop()
{
    // reset led intensities
  for (int i=0; i<NUM_LEDS; i++) { ledIns[i] = LOWEST_INS+100; }
  
  int val;
  int ctr;
  int centreVal;

  // for each sensor

  int fps_multiplier = 1;

  // leds control their own section
  for (int i=0; i < NUM_SENSORS; i++) {
    val = getLedVal(sensors[i]);
    // if sensing something
    
    if (val > 120) {
      // set led intensities around index range
      ctr = snsrCentre[i];
      for (int idx = ctr-SENSE_RANGE; idx <= ctr+SENSE_RANGE; idx++) {
        
        // further leds further away range have different intensities

//        ledIns[(idx+NUM_LEDS)%NUM_LEDS] = 0;
        ledIns[(idx+NUM_LEDS)%NUM_LEDS] -= (int)max((long)100, map(abs(ctr-idx), 0, SENSE_RANGE, 100, 0)); 

      }
    }
  }
  
  confetti(); 

  // centre sensor does some different cool stuff
  centreVal = getLedVal(CENTRE_SENSOR);
  if (centreVal < 80) {
    fps_multiplier = 2;
    sinelon();
  }

  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/(fps_multiplier*FRAMES_PER_SECOND)); 
//  FastLED.delay(1000/FRAMES_PER_SECOND); 

  gHue++;   // slowly cycle the "base color" through the rainbow
}


void confetti() 
{
  // constantly increase brightness to ensure colours don't go black
  for (int i=0; i<NUM_LEDS; i++) { 
    leds[i] += CHSV( 0, 0, 2);
  }

  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = random16(NUM_LEDS);
  // multiply brightness by led intensity
  leds[pos] = CHSV(gHue+ random8(64), 200, (2 * ledIns[pos])); 
}


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
//  fadeToBlackBy( leds, NUM_LEDS, 20);
  for (int i=0; i<NUM_LEDS; i++) {
    ledIns[i] =  max(LOWEST_INS, (ledIns[i] * 8)/10);
  }
  int pos = beatsin16( 24, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}


int getLedVal(int qrd_pin) {
  int proximityADC = analogRead(qrd_pin);
  float proximityV = (float)proximityADC * 5.0 / 1023.0;
  int ledVal = 255 - map(proximityV, 2, 20, 0, 255);  // 0-20 is mapped to 0-255
  return ledVal;
}
