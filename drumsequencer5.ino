/*
  header       Byte1         byte2       byte3
  ---------------------------------------------------
  2=MTC       240=STOP,    clocktick
          241=clocktick    counted
  --------------------------------------------------
  8=noteOff   Channel         note       velocity
              (-127)
  ---------------------------------------------------
  9=noteOn    Channel         note       velocity
              (-143)
  ---------------------------------------------------
  11=CC       Channel         CC#        CC Value
              (-175)
  ---------------------------------------------------
  15=clock   tick=248
            Start=250
            Stop=252
*/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>
#include <Adafruit_NeoTrellisM4.h>
Adafruit_ADXL343 accel = Adafruit_ADXL343(12345, &Wire1);
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
#define MIDI_CHANNEL 0                                                        // default channel # is 0

#define TICKS_PER_STEP 6

int tickcount = 0;
int beatcount = 0;
int STEP;                                                                     // variable for "step" selection
int INST;                                                                     // variable for "instrument" selection
const int inst = 8;                                                           // number of instruments
const int steps = 16;                                                         // number of steps
bool pattern[inst][steps] {                                                   // Array for the "inst" step sequencers
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
int veloOn[8] {};
int pixelcolor[inst] {7735946, 3093151, 16616016, 3263577, 16776990, 3093079, 15610826, 15649879}; // color of the instruments
void setup() {
  accel.begin();
  accel.setRange(ADXL343_RANGE_16_G);
  Serial.begin(115200);
  trellis.begin();
  trellis.setBrightness(200);
  trellis.enableUSBMIDI(true);
  trellis.setUSBMIDIchannel(MIDI_CHANNEL);
  for (int i = 0; i <= 15; i++) {
    trellis.setPixelColor(i, pixelcolor[0]);                                  // set pixels 0-15 to first instrument color
  }
  for (int r = 16; r <= 23; r++) {
    trellis.setPixelColor(r, pixelcolor[r - 16]);                             // set pixels 16 - 23 to  instrument color
  }
}

void loop() {
  trellis.tick();
  sensors_event_t sensor;
  accel.getEvent(&sensor);


  if (trellis.isPressed(24)) {
    trellis.controlChange(74, map(sensor.acceleration.x, -10, 10, 0, 127));
  }
  if (trellis.isPressed(25)) {
    trellis.controlChange(75, map(sensor.acceleration.y, 10, -10, 0, 127));
  }


  while (trellis.available()) {                                               // while we hit buttons..
    keypadEvent e = trellis.read();

    for (int m = 16; m <= 23; m++) {
      if (trellis.isPressed(26)) {
        if (trellis.isPressed(m)) {
          veloOn[m - 16] = 0;
        }
        else {
          veloOn[m - 16] = 96;
        }
      }
    }

    for (int i = 16; i <= 23; i++) {                                          // this counts for all the instrument selections
      if (trellis.isPressed(i)) {                                             // if we choose a instrument
        INST = i - 16;                                                        // set the instrument-selector to "pressed button"-16
        for (STEP = 0; STEP < steps; STEP++) {                                // this counts for all choosen steps
          if (pattern[INST][STEP] == HIGH) {                                  // if any of the choosen instrument steps is high
            trellis.setPixelColor(STEP, 7340032);                             // set step led color to red
          }
          else {
            pattern[INST][STEP] = LOW;                                        // if any other step is low
            trellis.setPixelColor(STEP, pixelcolor[INST]);                    // make it the instrument color.
          }
        }
        if (e.bit.EVENT == KEY_JUST_PRESSED) {                                // now to look if any of the step buttons is hit (with the instrument-button,(we´re still in the first for-loop!))
          if (e.bit.KEY < steps) {                                            // check that we only hit on of the step buttons
            STEP = e.bit.KEY;                                                 // set the clicked step-button as the Step-selector
            if (pattern[INST][STEP] == LOW) {                                 // if any of the choosen instrument steps is low
              pattern[INST][STEP] = HIGH;                                     // make it high
              trellis.setPixelColor(STEP, 7340032);                           // make the selected step red
            }
            else {                                                            // if it was already high
              pattern[INST][STEP] = LOW;                                      // make it low
              trellis.setPixelColor(STEP, pixelcolor[INST]);                  // make the selected step to instrument color
            }   trellis.show();
          } // boom
        }   // bamm
      }     // chack
    }       // pow-wee
  }         //  we are done with checking what is pressed and filling the array with active (high) steps

  midiEventPacket_t rx; // now we look at midi
  rx = MidiUSB.read();  // read incoming midi

  if (rx.header == 15 && rx.byte1 == 252) {                                   // clock stop
    tickcount = 0; // reset the tick counter
    beatcount = 0; // reset the beatcounter
  }

  if (rx.header == 15 && rx.byte1 == 250) {                                   // clock start

  }

  if (rx.header == 15 && rx.byte1 == 248) {                                   // clock tick
    tickcount++;
  }

  if (tickcount == 6) {                                                       //every 6th clock ticker
    beatcount++;                                                              //we count the beatcounter + 1
    tickcount = 0;                                                            //and reset the tickcounter
    for (int f = 16; f <= 23; f++) {
      if (trellis.isPressed(27)) {
        if (trellis.isPressed(f)) {
          trellis.noteOn(f + 20, 96);                                         //f + 20 = 36 to 43 midi note
        }     
      }
    }

    if (beatcount == 16) {                                                    //every 16th beatcount
      beatcount = 0;                                                          //we reset the beatcounter, to have 16 steps
    }
   
//    for (int i = 0 ; i < inst ; i++ ) {                                       // loop over each instrument

      if (pattern[0][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(36, veloOn[0]);                                    // play the corresponding midi-note
        trellis.setPixelColor(16, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(36, 0);                                           // if its low, shut up
        trellis.setPixelColor(16, pixelcolor[0]);
      }
      if (pattern[1][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(37, veloOn[1]);                                    // play the corresponding midi-note
        trellis.setPixelColor(17, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(37, 0);                                           // if its low, shut up
        trellis.setPixelColor(17, pixelcolor[1]);
      }
      if (pattern[2][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(38, veloOn[2]);                                    // play the corresponding midi-note
        trellis.setPixelColor(18, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(38, 0);                                           // if its low, shut up
        trellis.setPixelColor(18, pixelcolor[2]);
      }
      if (pattern[3][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(39, veloOn[3]);                                    // play the corresponding midi-note
        trellis.setPixelColor(19, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(39, 0);                                           // if its low, shut up
        trellis.setPixelColor(19, pixelcolor[3]);
      }
      if (pattern[4][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(40, veloOn[4]);                                    // play the corresponding midi-note
        trellis.setPixelColor(20, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(40, 0);                                           // if its low, shut up
        trellis.setPixelColor(20, pixelcolor[4]);
      }
      if (pattern[5][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(41, veloOn[5]);                                    // play the corresponding midi-note
        trellis.setPixelColor(21, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(41, 0);                                           // if its low, shut up
        trellis.setPixelColor(21, pixelcolor[5]);
      }
      if (pattern[6][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(42, veloOn[6]);                                    // play the corresponding midi-note
        trellis.setPixelColor(22, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(42, 0);                                           // if its low, shut up
        trellis.setPixelColor(22, pixelcolor[6]);
      }
      if (pattern[7][beatcount] == HIGH) {                                    // if the instrument is set to high at this point
        trellis.noteOn(43, veloOn[7]);                                    // play the corresponding midi-note
        trellis.setPixelColor(23, 7340032);                               // make the instrument switch red
      } 
      else {
        trellis.noteOff(43, 0);                                           // if its low, shut up
        trellis.setPixelColor(23, pixelcolor[7]);
      }
        // todo: you probably want to replace this with an array to track how long the instrument has been playing and only send note off when both
        // the note is already on AND a duration has elapsed
        // otherwise you are sending note offs immediately, and every step, unnecessarily
      
      trellis.setPixelColor(beatcount - 1, pixelcolor[INST]);                 // set the last played positionmarker to instrument color , here i tried to replace with different if statements with no luck
      trellis.setPixelColor(beatcount, 24577);                                // set positionmarker to green
//    }
    if (beatcount == 0) {                                                     // additional line to handle the annoying 16th step color
      trellis.setPixelColor(15, pixelcolor[INST]);                            // here you have you stupid 16th step
    }
  }
  trellis.show();
  trellis.sendMIDI();                                                         // send any pending MIDI messages
} // end of code. yehaaw
