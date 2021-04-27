/*
 * Required libraries:
 * - HX711 Arduino Library (v0.7.4) by Bogdan Necula, Andreas Motl
 * - SdFat (v2.0.6) by Bill Greiman
 * - RTClib (v1.13.0) by Adafruit
 * - Adafruit ST7735 and ST7789 Library (v1.7.1) by Adafruit
 * - Adafruit seesaw Library (v1.4.0) by Adafruit
 * - Adafruit Neopixel (v1.7.0) by Adafruit
 * - Adafruit GFX Library (v1.10.7) by Adafruit
 *
*/

#ifndef FORCE_H
#define FORCE_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_miniTFTWing.h>
#include <Adafruit_ST7735.h>
#include <HX711.h>
#include <SdFat.h>
#include "RTClib.h"

// Pin definitions
#define TFT_RST     -1    // TFT display shield
#define TFT_CS      5     // TFT display shield
#define TFT_DC      6     // TFT display shield
#define DOUT        12    // Load cells
#define CLK         13    // Load cells
#define DOUT2       11    // Load cells
#define CLK2        9     // Load cells
#define LICKOMETER  18    // Lick-o-meter
#define BEEPER      19    // Beeper
#define SOLENOID    4     // Solenoid
    
void dateTime(uint16_t* date, uint16_t* time);

class Force {
  public:
    Force(float _force_req, int _press_length, int _dispense_length, int _time_flag);
    
    // --- Basic functions --- //
    void begin();
    void run();
    void check_buttons();
    
    // --- TFT display wing --- //
    Adafruit_miniTFTWing ss;
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    int divideLine = 57;
    int x_speed = 1;
    int x = 0;
    int y;
    int lasty;
    int y2;
    int lasty2;
    void graphLegend();
    void graphData();
    void graphDateTime();
    void UpdateDisplay();

    // --- Neopixel --- //
    Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

    // --- Load Cell --- //
    HX711 scale;
    HX711 scale2;
    float force_req = 2;      // was F_req. Force required to trigger the solenoid valve
    float calibration_factor = -2182.57639563;
    float scaleChange = 0;    // Load cell 1
    float lastReading = 0;    // Load cell 1
    int change;               // Load cell 1
    float outputValue;        // Load cell 1
    float val;                // Load cell 1
    float scaleChange2 = 0;   // Load cell 2
    float lastReading2 = 0;   // Load cell 2
    int change2;              // Load cell 2
    float outputValue2;       // Load cell 2
    float val2;               // Load cell 2
    void Sense();
    void Tare();

    // --- SD File --- //
    SdFat SD;
    File logfile;               // Create file object
    char filename[15];          // Array for file name data logged to named in setup
    const int chipSelect = 10;
    void CreateDataFile();
    void writeHeader();
    void WriteToSD();
    void error(uint8_t errno);
    void getFilename(char *filename);
    void logdata();

    // --- Solenoid functions --- //
    void Dispense();
    int dispense_length = 2000;

    // --- Other functions/variables --- //
    void TaskReq();
    void PlayTone();
    void Timeout();
    void PressLengthReq();
    void SerialOutput();

    int FRC = 0;          // This is the unique # of the device
    bool lick = false;    
    bool mode = false;    // "Mode" variable. Can default to 0. To be changed with buttons on the mini TFT shield.
    int inc = 1;
    // int R = 1;
    // int start_timer_trial = 0;
    // unsigned int time_lapse_trial;
    int trial = 0;
    int trial_flag = 1;
    int time_flag = 3000;             // this is the length of the trial ON, typiclaly 15 sec but can be shorter for troubleshooting
    // int time_flag_full_trial = 30000; //this is the length of time before the trial plus ITI resets, typically 30 sec but can be longer for troubeshooting
    int FR_trials = 0;
    float grams;
    float grams2;
    int start_timer_disp = 0;
    unsigned int time_lapse_disp;
    unsigned int dispense_flag;
    // unsigned int dispense_timer_flag;
    int tone_flag = 0;
    int press_length = 300; // lever has to be held for this long to trigger a reward
    int press_duration_exceeded = 0;
    unsigned long time_lapse_lever_press = 0;
    unsigned long start_timer_lever_press = 0;
    int press_flag = 0;
    // int start_press_timer = 0;
    int start_timer = 0;
    int start_timer_timeout = 0;
    int time_lapse_timeout = 0;
    int timeout_flag = 0;
};

#endif  // FORCE_H