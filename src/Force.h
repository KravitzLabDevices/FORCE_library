/*
 * Required libraries:
 * - HX711 Arduino Library (v0.7.4) by Bogdan Necula, Andreas Motl
 * - SdFat - Adafruit Fork (v1.2.3) by Bill Greiman
 * - RTClib (v1.13.0) by Adafruit
 * - Adafruit ST7735 and ST7789 Library (v1.7.2) by Adafruit
 * - Adafruit seesaw Library (v1.4.2) by Adafruit
 * - Adafruit Neopixel (v1.8.0) by Adafruit
 * - Adafruit GFX Library (v1.10.7) by Adafruit
 * - Adafruit Adafruit_SPIFlash (v3.4.1) by Adafruit
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
#include "Adafruit_SPIFlash.h"

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
    Force(String ver);
    String ver = "undef";
    String library_version = "undef";
            
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
    float calibration_factor = -3300;
    float calibration_factor2 = -3300;
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
    bool calibrated = false;

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
    unsigned long unixtime = 0;


    // --- Solenoid functions --- //
    float dispense_time = 0;
    int dispense_amount = 20;
    void Dispense();

    // --- Lever functions --- //
    unsigned long pressTime = 0;
    unsigned long pressLength = 0;
    int dispense_delay = 4;

    // --- Trial functions--- //
    int FRC = 1;          // This is the unique # of the device
    bool PR = false;
    void Tone();
    void Click();
    void Timeout(int timeout_length);
    int timeout_length = 10;
    bool lick = false;
    int start_timer = 0;
    int trial = 0;
    int presses = 0;
    int req = 2;
    unsigned long pressStart = 0;
    int ratio = 1;
    int hold_time = 350;
    int trials_per_block = 10;
    int max_force = 20; 
    unsigned long start_time = 0;
    unsigned long lickTime = 0;
    unsigned long dispenseTime = 0;
    int random_number = 0;
    bool shock = false;
    bool dispensing = false;

    // --- Serial out--- //
    void SerialOutput();

    // --- start up menu--- //
    void start_up_menu();
    bool start_up = true;
    void save_settings();
    
    // --- save and load settings from QSPI flash --- //
    int settings[9];  //array for saving settings on QSPI flash
    int settings_recalled[9]; //arrat for reading from QSPI flash
    void load_settings();
    void reset_settings();
    void print_settings();
};

#endif  // FORCE_H
