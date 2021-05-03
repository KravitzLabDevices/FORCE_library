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
#define DOUT        12    // Load cell1
#define CLK         13    // Load cell1
#define DOUT2       11    // Load cell2
#define CLK2        9     // Load cell2
#define LICKOMETER  18    // Lick-o-meter
#define BEEPER      19    // Beeper
#define SOLENOID    4     // Solenoid
    
void dateTime(uint16_t* date, uint16_t* time);

class Force {
  public:
    Force(String sketch);
    String sketch = "undef";
    String sessiontype = "undef";
            
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

    // --- Load Cells --- //
    HX711 scale;
    HX711 scale2;
    float force_req = 2;      // was F_req. Force required to trigger the solenoid valve
    float calibration_factor = -2182.57639563;
    float scaleChange = 0;    // Load cell 1
    float lastReading = 0;    // Load cell 1
    float outputValue;        // Load cell 1
    float grams;
    float scaleChange2 = 0;   // Load cell 2
    float lastReading2 = 0;   // Load cell 2
    float outputValue2;       // Load cell 2
    float grams2;
    void Sense();
    void Tare();
    void Calibrate();
    bool calibrate_active = false;

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
    void Dispense(int ms = 20);
    unsigned long dispenseTime = 0;

    // --- Lever functions --- //
    unsigned long pressTime = 0;
    unsigned long pressLength = 0;
        
    // --- Trial functions--- //
    void Tone();
    void Click();
    void Timeout(float timeout_length);
    int FRC = 0;          // This is the unique # of the device
    bool lick = false;
    int start_timer = 0;
    int trial = 0;
    int presses = 0;
    int req = 20;
    float timeout_length = 10;
    unsigned long pressStart = 0;
    float dispense_delay = 4;
    int ratio = 1;

    // --- Serial out--- //
    void SerialOutput();

    // --- start up menu--- //
    void start_up_menu();
    bool start_up = true;

};

#endif  // FORCE_H