#include <FastLED.h>
#include <math.h>
#include <algorithm>
#include <array>

#define LED_PIN     25
#define POT_PIN     4
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    24
#define LEDS_PER_SECTION 4
#define NUM_SENSORS 6

#define COLOUR_COUNT 6
#define PULSE 60
#define PULSE_VAR 200
#define PULSE_TIME 10

#define POT_RANGE   455
#define POT_MAX     4095
#define BRIGHTNESS  255

class LEDToggle {
  public:
    int input;
    int inputState = 0;     // 0 or 1
    int outputState = 0;    // 0 - COLOUR_COUNT-1
    
    int LEDUpdate();

    LEDToggle(int inputPin) {
      input = inputPin;
      pinMode(input, INPUT);
    }
};

int LEDToggle::LEDUpdate() {
  if (digitalRead(this->input) == LOW && this->inputState == HIGH) {    // i.e. change in state
    this->outputState++;
    if (outputState >= COLOUR_COUNT) {
      outputState = 0;
    }
  }
  
  this->inputState = digitalRead(this->input);
  
  return this->outputState;
}

int tick = 0;
CRGB leds[NUM_LEDS];
int sensors[NUM_SENSORS] = { 33, 14, 36, 32, 27, 15 };
int colours[COLOUR_COUNT][3] = { {0, 0, 0}, {210, 1, 0}, {255, 255, 255}, {254, 180, 11}, {221, 73, 145}, {217, 88, 34} };

std::array<LEDToggle, NUM_SENSORS> toggles = { LEDToggle(sensors[0]), LEDToggle(sensors[1]), LEDToggle(sensors[2]), LEDToggle(sensors[3]), LEDToggle(sensors[4]), LEDToggle(sensors[5]) }; // c++???

int potentiometer = 0;
double brightness = 0.0;


void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  Serial.begin(115200);

  for (int i = 0; i < NUM_LEDS; i++) { // set all lights to off
    leds[i] = CRGB(0,0,0);
  }

  for (int i = 0; i < NUM_SENSORS; i++) { // create toggles for all sensors
    toggles[i] = LEDToggle(sensors[i]);
  }
}

void loop() {
  tick++;
  
  potentiometer = analogRead(POT_PIN) - (POT_MAX - POT_RANGE);                  // constrain the potentiometer output to an achievable range (without snapping wires)
  if (tick % 15 == 0) {                                                         // update at slower speed to reduce flicker
    brightness = (double)std::max((potentiometer / (double)POT_RANGE), 0.0);          
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    int target = toggles[i].LEDUpdate();
    
    for (int j = 0; j < LEDS_PER_SECTION; j++) {        // light LEDs
      int index = LEDS_PER_SECTION * i + j;             // index of target LED in the ring (the jth LED belonging to the ith sensor)

      if (target == 0) {
        leds[index] = CRGB(0, 0, 0);
      }

      else {
        // each LED pulsates with PULSE amplitude and PULSE_VAR offset from its neighbours
        int r = (int) (brightness * (colours[target][0] + (PULSE * sin((tick + (PULSE_VAR * index)) / PULSE_TIME))));
        r = std::max(std::min(r, 255), 0);              // lock to 0-255      
        int g = (int) (brightness * (colours[target][1] + (PULSE * sin((tick + (PULSE_VAR * index)) / PULSE_TIME))));
        g = std::max(std::min(g, 255), 0);              // lock to 0-255    
        int b = (int) (brightness * (colours[target][2] + (PULSE * sin((tick + (PULSE_VAR * index)) / PULSE_TIME))));
        b = std::max(std::min(b, 255), 0);              // lock to 0-255    
        
        leds[index] = CRGB(r, g ,b);                    // apply
      }
    }
  }

  FastLED.show();
  FastLED.delay(10);
}
