#include "FastLED.h"
#include <EEPROM.h>
#include <RotaryEncoder.h>

//---------------------------------------------------------------------------------------------------------------------------
// BBT MicroQuad Race Gate - Opensource Version
//---------------------------------------------------------------------------------------------------------------------------
// It's been a long time coming. This is the code I used for my BBT Tiny Whoop gates for a few years, with some changes to
// make it more user friendly for the open-source world. Thank you to all the support and I hope the open-source world will
// only expand on this work. I'd love to see what everyone comes up with!
//
// I am releasing this code for the Tiny Whoop/Micro Drone/Drone Racing community so that clubs can use it to create their own
// set of racing gates and grow the sport. Therefore, I have licensed the code under GPLv3 (see below). Please, use it, modify
// it, and SHARE your contributions with the community.
//
//---------------------------------------------------------------------------------------------------------------------------
// Functional Details
//---------------------------------------------------------------------------------------------------------------------------
// Details for the electronics and the 3D case design are located on GitHub and Thingiverse.
//
// When the gate is turned on/plugged in, there is a short 2 second delay. That is changeable in the code if desired.
//
// Brightness Control - Twisting the rotary control will display a blue representation of the gate brightness. The lights
// should increase in brightness as the control is turned clockwise until all the lights on the ring are illuminated. Dim
// the brightness by turning counter-clockwise to the desired amount.
//
// Power Save Mode - You can keep the gate plugged in and put it into "power save mode" before a race or during a break. Simply
// turn the brightness control all the way to the left and leave it. Turn the brightness back up when you're ready to fly.
//
// Sequenced Pattern Operation - When the gate starts up, it is automatically in a "sequenced mode", meaning that the available
// patterns displayed on the gate are in order, from the first pattern to the last, then over again. There is a programmed delay
// of 20 seconds between pattern changes.
//
// Fixed Pattern Operation - If you would like to have the gate display one specific pattern or color, push in on the rotary
// selector to advance through the available patterns. Once you decide on a pattern, leave it on that setting and it will remain
// on that pattern/color until it is unplugged or another mode is selected.
//
// Additional mode selections:
// Pressing and holding the control knob will display a pattern on your gate, divided into 4 sections. Holding the button will
// select the next mode and so on.
// Mode 1 - Return to Sequenced Pattern Operation - This will return to the cycling pattern mode. A blue flash will indicate
//          you are in the sequenced pattern mode.
// Mode 2 - Enter Random Pattern Operation - The gate will display random patterns and colors, changing every 20 seconds. A 
//          flash of random colors will indicate you are in the random mode.
// Mode 3 - Save Settings Operation - See below
// Mode 4 - Erase Settings Operation - See below
//
// Saving Settings - Many users of my gates asked me to create a set with only a specific pattern or color, or they wanted
// to mount the gates in an inaccessible location, plug them in from below, and not have to climb up to adjust the brightness
// or pattern every time they reset them. I developed this feature to address that. Here is how it is used:
// 1 - Select the pattern mode or fixed pattern you want the gate to display on power up.
// 2 - Select the brightness you'd like on power up.
// 3 - Hold the control button until the third section is displayed (Mode 3 - Save Settings).
// 4 - The gate will display a red countdown pattern after the button is released.
// 5 - If you wish to interrupt the write of settings, just press the button again and it will not save.
// 6 - Once the countdown has expired without interruption, it will save the settings and give a Green Display to show that 
//     it completed the operation.
//
// Clearing Settings - If you decide that you don't like the saved mode or brightness, the settings can be cleared using the 
// same instructions above, but going to the Mode 4 display. NOTE - The erase function takes some time, so when the display
// appears to freeze at the end of the clear settings mode, don't worry. It will continue in a moment.
//
// If you hold the button longer than Mode 4, it will return to regular operation and ignore the input.
//
//---------------------------------------------------------------------------------------------------------------------------
// The Licensing Blah blah blah
//---------------------------------------------------------------------------------------------------------------------------
// Copyrights:
// Original work Copyright (c) 2017, 2018, 2019 Dean Nicholson via BrightBlueTech(BBT) LLC
// Original work Copyright (c) 2019, 2020, Dean Nicholson via Hydra FPV Drone Entertainment Tech
//
// Attributions:
// Parts based on Demo Reel by Mark Kriegsman, December 2014
// "Marquee fun" (v3) pattern by Marc Miller, May 2016
//
// GPLv3 Note: This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// A copy of the GNU General Public License is available at <https://www.gnu.org/licenses/>
//
// Any queries can be addressed to me directly at dean@hydrafpv.com.
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//---------------------------------------------------------------------------------------------------------------------------

FASTLED_USING_NAMESPACE

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define between(x, a, b)  (((a) <= (x)) && ((x) < (b)))

//---------------------------------------------------------------------------------------------------------------------------
// Normal User adjustable code - Here's where you set up the number of LEDs in your strand, your initial brightness, etc.
//---------------------------------------------------------------------------------------------------------------------------
#define NUM_LEDS           100 // Physical number of LEDs in your string
#define SECONDS_PER_PAT    20  // Number of seconds between pattern change in Sequential and Random modes.
#define INIT_BRIGHTNESS    63  // The brightness your gate will power up with, adjustable between 0 and 255. 
//                                63 is 25%, which works well for all whoop cameras in a dark room.
//---------------------------------------------------------------------------------------------------------------------------

// If you have changed your data pin (LED data output), encoder connections, or button pins, change them here.
#define DATA_PIN              6 //D6
#define BUTTON1_PIN           2 //D2
#define ENCODER_A_PIN        A2 //A2
#define ENCODER_B_PIN        A3 //A3
#define HEARTBEATLED_PIN     13 //D13 - Built-in LED on NANO board. Allows us a visual reference when the button1 interrupt is executed.
#define DEBUG                1

// EEPROM Setup
#define EP_ADDRESS         0
struct ep_contents
{
  byte saved = 0;
  byte brightness = 0;
  byte rotaryPosition = 10;
  byte displayMode = 0;
  byte pattern = 0;
};
typedef struct ep_contents ep_contents;
ep_contents epData;
int actionCancel = 0;

// LED Setup
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB
#define FRAMES_PER_SECOND  360
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette(CRGB::Black);
// Many different possibilities here. These are accessible in colorpalettes.h in the FastLED library.
// Available: CloudColors_p, LavaColors_p, OceanColors_p, ForestColors_p, RainbowColors_p, RainbowStripeColors_p, PartyColors_p, and HeatColors_p
CRGBPalette16 targetPalette(PartyColors_p);

// Pattern Variables
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int lastBrightness = 127;
int actBrightness = 127;

// Rotary Encoder Setup
// Encoder limits
#define ROTARYSTEPS 1
#define ROTARYMIN 0
#define ROTARYMAX 40
RotaryEncoder encoder(ENCODER_A_PIN, ENCODER_B_PIN);
int lastPos = 10; // Last known rotary position.

enum Display_enum {
  SEQ_PAT,
  FIXED_PAT,
  RAND_PAT,
  MODE_PAT,
  WRITE_PAT,
  ERASE_PAT
};

enum Button_enum {
  PRESSED,
  RELEASED
};

// Button states and variables
Button_enum btn1Flag     = RELEASED; // value read from button 1
long btnPressTime; // time the button was pressed down
long btnReleaseTime; // time the button was released
long btnHeldTime; // time the button was held
// Amount of time to hold the select button to go from mode to mode.
// Tap puts display in next pattern and holds it there.
#define SEQ_HOLD_TIME 1500 // Number of ms the button is held to do sequenced mode.
#define RND_HOLD_TIME 3000 // Number of ms the button is held to do random mode.
#define EP_PROG_HOLD_TIME 4500 // Number of ms the button is held to save the present setup to the EEPROM.
#define EP_ERASE_HOLD_TIME 6000 // Number of ms the button is held to clear the EEPROM.

// Mode states and variables
Display_enum displayState        = SEQ_PAT;
Display_enum lastDisplayState    = SEQ_PAT;
Display_enum sendingDisplayState = SEQ_PAT;
Display_enum nextDisplayState    = SEQ_PAT;

int hbState = HIGH;       // State of the Heartbeat LED

void setup() {
  // Setup button 1 for internal pullup on the input - This, plus the cap, allows us to debounce the noisy push switch on the encoder.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(HEARTBEATLED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), button1Action, CHANGE); // Activates on push of encoder dial

  PCICR |= (1 << PCIE1);    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);  // This enables the interrupt for pin 2 and 3 of Port C.

  digitalWrite(HEARTBEATLED_PIN, hbState);

  // if DEBUG, initialize serial communications
  if (DEBUG == 1) {
    Serial.begin(19200);
  }

  delay(2000); // 2 second delay for recovery

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Set the maximum power allowed for the string - (volts, milliamps)
  // For now, limit to 1.5A
  FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);
  
  // Read EEPROM storage and determine if a previous saved/preferred state was stored (epData.saved is 1)
  // Load those values as default if they exist
  EEPROM.get(EP_ADDRESS, epData);
  if (epData.saved == 255) { // 255 = FFh, which is the erased state (no settings saved)
    // set initial master brightness control (just in case) and read in initial brightness control value
    FastLED.setBrightness(INIT_BRIGHTNESS);
    encoder.setPosition(10 / ROTARYSTEPS); // start with the value of 10 which corresponds to 25% brightness.
    displayState = SEQ_PAT; // Start with sequenced pattern
    epData.brightness = INIT_BRIGHTNESS;
    epData.rotaryPosition = encoder.getPosition() * ROTARYSTEPS;
  } else { // If any other value is written, it is not erased and therefore settings are saved.
    // set initial brightness and the previous value stored away
    FastLED.setBrightness(epData.brightness);
    lastBrightness = epData.brightness;
    actBrightness  = epData.brightness;
    encoder.setPosition(epData.rotaryPosition / ROTARYSTEPS);
    displayState = epData.displayMode;
    if(displayState == FIXED_PAT) {
      gCurrentPatternNumber = epData.pattern;
    }
  }
  if(DEBUG==1) {
    char epDataDump[40];
    sprintf(epDataDump, "saved=%d, bri=%d, pos=%d, mod=%d, pat=%d\n",epData.saved, epData.brightness, epData.rotaryPosition, epData.displayMode, epData.pattern);
    Serial.print(epDataDump);
  }
} 

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

// User Changable - You can change the available patterns here
SimplePatternList gPatterns = { gradient_fill, marque_v3, blendme, three_sin_pal, bpm, rainbow, rainbowWithGlitter, confetti, ConstantColorRed, ConstantColorYellow, ConstantColorGreen, ConstantColorCyan, ConstantColorPurple, ConstantColorBlue};

void loop() {
  // get the current physical position and calc the logical position
  int newPos = encoder.getPosition() * ROTARYSTEPS;

  // Keep the rotary position between ROTARYMIN and ROTARYMAX
  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN / ROTARYSTEPS);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) {
    encoder.setPosition(ROTARYMAX / ROTARYSTEPS);
    newPos = ROTARYMAX;
  }
  
  // If the rotary encoder has been turned, immediately go to brightnessControl()
  if (lastPos != newPos) {
    lastPos = newPos;
    brightnessControl();
  }

  // This is an active debug feature. As long as the controller is not hung, the on-board heartbeat LED will
  // toggle with every push of the button. If the LED is not toggling, the controller is hung.
  digitalWrite(HEARTBEATLED_PIN, hbState);
  
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  displayStateMachine();
  
  // Call the current pattern function once, updating the 'leds' array
  // If the mode display is being shown, do not fill with the gPattern display.
  if((displayState == SEQ_PAT) || (displayState == FIXED_PAT) || (displayState == RAND_PAT)) {
    gPatterns[gCurrentPatternNumber]();
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  epData.pattern = gCurrentPatternNumber;

}

void randomPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = random(0, ARRAY_SIZE( gPatterns));
  epData.pattern = gCurrentPatternNumber;
}

void brightnessControl() {
  int numLedsToLight = 0;

  numLedsToLight = map(lastPos, 0, ROTARYMAX, 0, NUM_LEDS);
  actBrightness = map(lastPos, 0, ROTARYMAX, 0, 255);
  FastLED.setBrightness(actBrightness);
  FastLED.clear();
  for (int led = 0; led < numLedsToLight; led++)
    leds[led] = CRGB::Blue;
  FastLED.show();
  lastBrightness = actBrightness;
  // Delay half a second to allow value to change if user is turning it
  delay(500);
  epData.brightness = lastBrightness;
  epData.rotaryPosition = lastPos;
}

void displayStateMachine() {

  switch(displayState) {

    case SEQ_PAT:
      if (btn1Flag == PRESSED) {
        sendingDisplayState = displayState;
        displayState = MODE_PAT;
      } else {
        EVERY_N_SECONDS( SECONDS_PER_PAT ) {
          nextPattern();
          if (DEBUG == 1) {
            Serial.print("PatternNumber = ");
            Serial.print(gCurrentPatternNumber);
            Serial.print("\n");
          }
        }
        epData.displayMode = displayState;
        displayState = SEQ_PAT;
      }
      break;

    case FIXED_PAT:
      if (btn1Flag == PRESSED) {
        sendingDisplayState = displayState;
        displayState = MODE_PAT;
      } else {
        // Do Nothing, stick with the same pattern.
        epData.displayMode = displayState;
        displayState = FIXED_PAT;
      }
      break;

    case RAND_PAT:
      if (btn1Flag == PRESSED) {
        sendingDisplayState = displayState;
        displayState = MODE_PAT;
      } else {
        EVERY_N_SECONDS( SECONDS_PER_PAT ) {
          randomPattern();
          if (DEBUG == 1) {
            Serial.print("PatternNumber = ");
            Serial.print(gCurrentPatternNumber);
            Serial.print("\n");
          }
        }
        epData.displayMode = displayState;
        displayState = RAND_PAT;
      }
      break;

    case MODE_PAT:
      if ((millis() - btnPressTime) >= EP_ERASE_HOLD_TIME + 2500) { // Give up.
        fadeToBlackBy( leds, NUM_LEDS, 127);
        for (int k = 0; k < NUM_LEDS; k++) {
          leds[k].setRGB(0, 0, 0);
        }
        displayState = sendingDisplayState;
        break;
      } else if ((millis() - btnPressTime) >= EP_ERASE_HOLD_TIME) {
        
//        for (int k = ((75 * NUM_LEDS) / 100); k < ((99 * NUM_LEDS) / 100); k++) {
        for (int k = 0; k < NUM_LEDS; k++) {
          if(between(k, ((75 * NUM_LEDS) / 100), NUM_LEDS-1)) {
            leds[k].setRGB(0, 0, 207);
          } else {
            leds[k].setRGB(0, 0, 0);
          }
        }
        nextDisplayState = ERASE_PAT;
      } else if ((millis() - btnPressTime) >= EP_PROG_HOLD_TIME) {
        for (int k = 0; k < NUM_LEDS; k++) {
          if(between(k, ((50 * NUM_LEDS) / 100), ((75 * NUM_LEDS) / 100))) {
            leds[k].setRGB(0, 0, 207);
          } else {
            leds[k].setRGB(0, 0, 0);
          }
        }
        nextDisplayState = WRITE_PAT;
      } else if ((millis() - btnPressTime) >= RND_HOLD_TIME) {
        for (int k = 0; k < NUM_LEDS; k++) {
          if(between(k, ((25 * NUM_LEDS) / 100), ((50 * NUM_LEDS) / 100))) {
            leds[k].setRGB(0, 0, 207);
          } else {
            leds[k].setRGB(0, 0, 0);
          }
        }
        nextDisplayState = RAND_PAT;
      } else if ((millis() - btnPressTime) >= SEQ_HOLD_TIME) {
        for (int k = 0; k < NUM_LEDS; k++) {
          if(between(k, 0, ((25 * NUM_LEDS) / 100))) {
            leds[k].setRGB(0, 0, 207);
          } else {
            leds[k].setRGB(0, 0, 0);
          }
        }
        nextDisplayState = SEQ_PAT;
      } else {
        fadeToBlackBy( leds, NUM_LEDS, 127);
        for (int k = 0; k < NUM_LEDS; k++) {
          leds[k].setRGB(0, 0, 0);
        }
        nextDisplayState = FIXED_PAT;
      }

      if (btn1Flag == RELEASED) {
        if(nextDisplayState == FIXED_PAT) {
          nextPattern();
        } else if (nextDisplayState == RAND_PAT) {
          randModeDisplay(); //display a random sequence to signifiy mode
          randomPattern();
        } else if (nextDisplayState == SEQ_PAT) {
          seqModeDisplay(); // display a sequence to signify mode
          nextPattern();
        } else if ((nextDisplayState == WRITE_PAT) || (nextDisplayState == ERASE_PAT)) {
          countdownDisplay();
          if(actionCancel == 1) {
            nextDisplayState = sendingDisplayState;
          }
        }
        
        displayState = nextDisplayState;
        
        if (DEBUG == 1) {
          Serial.print("displayState = ");
          Serial.print(displayState);
          Serial.print("\n");
        }
      }
      break;

    case WRITE_PAT:
      epData.saved = 1;
      EEPROM.put(EP_ADDRESS, epData);
      eepromConfirm();
      displayState = sendingDisplayState;
      break;

    case ERASE_PAT:
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 255); // 255=FF which is erased
      }
      eepromConfirm();
      displayState = sendingDisplayState;
      break;
  }
  lastDisplayState = displayState;
}

void seqModeDisplay() {
  for (int s = 255; s > 75; s -= 2) {
    for (int t = 0; t < NUM_LEDS; t++) {
      leds[t].setRGB(s, 0, s);
    }
    FastLED.show();
    FastLED.delay(100 / FRAMES_PER_SECOND);
  }
  fadeToBlackBy( leds, NUM_LEDS, 20);
  delay(1000);
} // seqModeDisplay

void randModeDisplay() {
  int randColor = 0;
//  for (int r = 0; r < 3; r++) {
  for (int s = 255; s > 75; s -= 2) {
    for (int t = 0; t < NUM_LEDS; t++) {
      randColor = random(0, 3);
      if (randColor == 0)
        leds[t].setRGB(s, 0, 0);
      if (randColor == 1)
        leds[t].setRGB(0, s, 0);
      if (randColor == 2)
        leds[t].setRGB(0, 0, s);
    }
    FastLED.show();
    FastLED.delay(100 / FRAMES_PER_SECOND);
  }
//  }
  fadeToBlackBy( leds, NUM_LEDS, 20);
  delay(1000);
} // randModeDisplay

void countdownDisplay() {
  for (int s = NUM_LEDS; s > 0; s--) {
    for (int r = 0; r < NUM_LEDS; r++) {
      if (r <= s) {
        leds[r].setRGB(200, 0, 0);
      } else {
        leds[r].setRGB(0, 0, 0);
      }
    }
    if (btn1Flag == PRESSED){
      actionCancel = 1;
      break;
    }
    FastLED.show();
    FastLED.delay(100 / FRAMES_PER_SECOND);
    delay(20);
    actionCancel = 0;
  }
}

void eepromConfirm() {
  for (int s = NUM_LEDS; s > 0; s--) {
    for (int r = 0; r < NUM_LEDS; r++) {
      if (r <= s) {
        leds[r].setRGB(0, 200, 0);
      } else {
        leds[r].setRGB(0, 0, 0);
      }
    }
    FastLED.show();
    FastLED.delay(100 / FRAMES_PER_SECOND);
  }
}


//---------------------------------------------------------------------------------------------------------------------------
// Patterns - Keep associated variables with the patterns for clarity
//---------------------------------------------------------------------------------------------------------------------------

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 120;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void blendme() {
  uint8_t starthue = beatsin8(20, 0, 255);
  uint8_t endhue = beatsin8(35, 0, 255);
  if (starthue < endhue) {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue, 255, 255), CHSV(endhue, 255, 255), FORWARD_HUES); // If we don't have this, the colour fill will flip around
  } else {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue, 255, 255), CHSV(endhue, 255, 255), BACKWARD_HUES);
  }
} // blendme()

// Variables for three_sin/three_sin_pal
uint8_t thisdelay = 20;                                       // A delay value for the sequence(s)
int     wave1 = 0;                                            // Current phase is calculated.
int     wave2 = 0;
int     wave3 = 0;
uint8_t mul1 = 7;                                            // Frequency, thus the distance between waves
uint8_t mul2 = 6;
uint8_t mul3 = 5;

void three_sin_pal() {
  ChangeMe();

  uint8_t maxChanges = 24;

  EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
    nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);
    three_sin();                                              // Improved method of using non-blocking delay
  }
}

void three_sin() {

  wave1 += beatsin8(10, -4, 4);
  wave2 += beatsin8(15, -2, 2);
  wave3 += beatsin8(12, -3, 3);

  for (int k = 0; k < NUM_LEDS; k++) {
    uint8_t tmp = sin8(mul1 * k + wave1) + sin8(mul1 * k + wave2) + sin8(mul1 * k + wave3);
    leds[k] = ColorFromPalette(currentPalette, tmp, 255);
  }

} // three_sin()

void ChangeMe() {

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if ( lastSecond != secondHand) {
    lastSecond = secondHand;
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB u = CHSV( HUE_BLUE, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;

    switch (secondHand) {
      case  0: targetPalette = RainbowColors_p; break;
      case  5: targetPalette = CRGBPalette16( u, u, b, b, p, p, b, b, u, u, b, b, p, p, b, b); break;
      case 10: targetPalette = OceanColors_p; break;
      case 15: targetPalette = CloudColors_p; break;
      case 20: targetPalette = LavaColors_p; break;
      case 25: targetPalette = ForestColors_p; break;
      case 30: targetPalette = PartyColors_p; break;
      case 35: targetPalette = CRGBPalette16( b, b, b, w, b, b, b, w, b, b, b, w, b, b, b, w); break;
      case 40: targetPalette = CRGBPalette16( u, u, u, w, u, u, u, w, u, u, u, w, u, u, u, w); break;
      case 45: targetPalette = CRGBPalette16( u, p, u, w, p, u, u, w, u, g, u, w, u, p, u, w); break;
      case 50: targetPalette = CloudColors_p; break;
      case 55: targetPalette = CRGBPalette16( u, u, u, w, u, u, p, p, u, p, p, p, u, p, p, w); break;
      case 60: break;
    }
  }
} // ChangeMe()

// draws a line that fades between 2 random colors
// TODO:  Add logic to rotate the starting point
void gradient_fill() {

  //  uint8_t hue1 = 60;
  //  uint8_t hue2 = random8(255);
  uint8_t hue1 = random8(255);
  uint8_t hue2 = hue1 + random8(30, 61);

  for ( int i = 0; i < NUM_LEDS; i++) {
    //fill_gradient (leds, 0, CHSV(0, 255, 255), i, CHSV(96, 255, 255), SHORTEST_HUES);
    fill_gradient (leds, 0, CHSV(hue1, 255, 255), i, CHSV(hue2, 255, 255), SHORTEST_HUES);
    delay(15);
    //    FastLED.show();
    //    FastLED.clear();
  }
}

// gradually fill up the strip with random colors
void randomColorFill(uint8_t wait) {
  fadeToBlackBy( leds, NUM_LEDS, 20);
  //  clearStrip();

  for (uint16_t i = 0; i < NUM_LEDS; i++) { // iterate over every LED of the strip
    int r = random(63, 255); // generate a random color
    int g = random(63, 255);
    int b = random(63, 255);

    for (uint16_t j = 0; j < NUM_LEDS - i; j++) { // iterate over every LED of the strip, that hasn't lit up yet
      leds[j - 1] = CHSV(0, 0, 0); // turn previous LED off
      leds[j] = CHSV(r, g, b); // turn current LED on
      FastLED.show(); // apply the colors
      delay(15);
    }
  }
}
//***************************************************************
// Marquee fun (v3)
//  Pixel position down the strip comes from this formula:
//      pos = spacing * (i-1) + spacing
//  i starts at 0 and is incremented by +1 up to NUM_LEDS/spacing.
//
// Marc Miller, May 2016
//***************************************************************
uint8_t spacing = 20;      // Sets pixel spacing. [Use 2 or greater]
uint8_t width = 5;        // Can increase the number of pixels (width) of the chase. [1 or greater]
int8_t advance;           // Stores the advance amount.
uint8_t color;            // Stores a hue color.
boolean fadingTail = 1;   // Add fading tail? [1=true, 0=false]
uint8_t fadeRate = 70;   // How fast to fade out tail. [0-255]
int8_t delta = 1;         // Sets forward or backwards direction amount. (Can be negative.)
#define MARQ_DEBUG  0

void marque_v3() {

  int16_t pos;

  for (uint8_t i = 0; i < (NUM_LEDS / spacing); i++) {
    for (uint8_t w = 0; w < width; w++) {
      pos = (spacing * (i - 1) + spacing + advance + w) % NUM_LEDS;
      color = gHue;

      leds[pos] = CHSV(color, 255, 255);
    }

    if (MARQ_DEBUG == 1) { // Print out lit pixels if DEBUG is true.
      Serial.print(" "); Serial.print(pos);
    }
    delay(10);
  }
  if (MARQ_DEBUG == 1) {
    Serial.println(" ");
  }
  //  FastLED.show();

  // Fade out tail or set back to black for next loop around.
  if (fadingTail == 1) {
    fadeToBlackBy(leds, NUM_LEDS, fadeRate);
  } else {
    for (uint8_t i = 0; i < (NUM_LEDS / spacing); i++) {
      for (uint8_t w = 0; w < width; w++) {
        pos = (spacing * (i - 1) + spacing + advance + w) % NUM_LEDS;
        leds[pos] = CRGB::Black;
      }
    }
  }

  // Advance pixel position down strip, and rollover if needed.
  advance = (advance + delta + NUM_LEDS) % NUM_LEDS;
}

void ConstantColorGreen () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::Green;
  }
}

//void ConstantColorRed (int PulseBrightness=NULL) {
void ConstantColorRed () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::Red;
  }
}

//void ConstantColorYellow (int PulseBrightness=NULL) {
void ConstantColorYellow () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::Yellow;
  }
}

void ConstantColorBlue () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::Blue;
  }
}

void ConstantColorPink () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::LightCoral;
  }
}

void ConstantColorPurple () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::BlueViolet;
  }
}

void ConstantColorCyan () {
  for (int k = 0; k < NUM_LEDS; k++) {
    leds[k] = CRGB::Cyan;
  }
}
//---------------------------------------------------------------------------------------------------------------------------
// End of Patterns
//---------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------------
// Interrupt Service Routines
//---------------------------------------------------------------------------------------------------------------------------
// ISR for Button press
void button1Action() {

  // Look at the definition section at the beginning of this document for button operation.
  Button_enum btn1 = digitalRead(BUTTON1_PIN);

  if (btn1 == PRESSED) {
    if (DEBUG == 1) {
      Serial.print("Button1 Pressed\n");
    }
    btnPressTime = millis();
    btn1Flag = PRESSED;
  } else if (btn1 == RELEASED) {
    if (DEBUG == 1) {
      Serial.print("Button1 Released\n");
    }
    hbState = !hbState; // Watchdog LED state
    btnReleaseTime = millis();
    btnHeldTime = btnReleaseTime - btnPressTime;
    btn1Flag = RELEASED;
  }
}

// The Interrupt Service Routine for Pin Change Interrupt 1
// This routine will only be called on any signal change on A2 and A3: exactly where we need to check.
ISR(PCINT1_vect) {
  encoder.tick(); // just call tick() to check the state.
}
//---------------------------------------------------------------------------------------------------------------------------
// END of ISRs
//---------------------------------------------------------------------------------------------------------------------------
