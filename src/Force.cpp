#include "Arduino.h"
#include "Force.h"

//SETUP QSPI FLASH
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
File myFile;

/////////////////////////////////////////////////////////////////////////
// Initialize FORCE!
/////////////////////////////////////////////////////////////////////////
Force::Force(String ver) {
  library_version = ver; 
}

/////////////////////////////////////////////////////////////////////////
// RTC Functions 
/////////////////////////////////////////////////////////////////////////
RTC_PCF8523 rtc;

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

/////////////////////////////////////////////////////////////////////////
//     
/////////////////////////////////////////////////////////////////////////
void Force::run() {
  Sense();
  UpdateDisplay();
  WriteToSD();
  DateTime now = rtc.now();
  unixtime  = now.unixtime();
  //SerialOutput();
}

/////////////////////////////////////////////////////////////////////////
// TaskFunctions 
/////////////////////////////////////////////////////////////////////////
void Force::Dispense() {
  dispensing = true;
  trial++;
  Tone();
  float successTime = millis();
  while ((millis() - successTime) < (dispense_delay * 1000)){
    tft.setCursor(85, 44);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Delay:");
    tft.setTextColor(ST7735_WHITE);
    tft.print((-(millis() - successTime - (dispense_delay*1000))/ 1000),1);
    run();
    tft.fillRect(84, 43, 80, 12, ST7735_BLACK); // remove Delay text when timeout is over
    if (grams > 1 or grams2 >1){ //only clear F1 ans F2 values if levers are being pushed
      tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after label
    }
  }
  digitalWrite(SOLENOID, HIGH);
  digitalWrite(A2, HIGH); // A2 will be "reward dispensed" pin
  digitalWrite(13, HIGH); // RED LED
  delay (dispense_amount); //how long to open solenoid?
  digitalWrite(SOLENOID, LOW);
  DateTime now = rtc.now();
  dispenseTime = now.unixtime();
  digitalWrite(A2, LOW);
  digitalWrite(13, LOW); // RED LED
  pressTime = millis();
  pressLength = 0;
  dispensing = false;
}

void Force::Timeout(int timeout_length) {
  dispense_time = millis();
  while ((millis() - dispense_time) < (timeout_length * 1000)){
    tft.setCursor(85, 44);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Timeout:");
    tft.print((-(millis() - dispense_time - (timeout_length*1000))/ 1000),1);
    run();
    tft.fillRect(84, 43, 80, 12, ST7735_BLACK);
    if ((grams > 1.5) or (grams2 > 1.5)) { //reset timeout if either lever pushed
      Timeout(timeout_length); 
      tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after F1 F2 labels
    }
  }
  tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after F1 F2 labels
}

/////////////////////////////////////////////////////////////////////////
// Sound Functions 
/////////////////////////////////////////////////////////////////////////
void Force::Tone() {
  tone(A5, 500, 200);
}

void Force::Click() {
  tone(A5, 800, 8);
}



/////////////////////////////////////////////////////////////////////////
// Begin
/////////////////////////////////////////////////////////////////////////
void Force::begin() {
  Serial.begin(9600);

  if (!ss.begin()) {
    Serial.println("seesaw couldn't be found!");
    while (1);
  }

  // Initialize pins
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(LICKOMETER, INPUT_PULLDOWN);
  pinMode(BEEPER, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(SOLENOID, OUTPUT);
  digitalWrite(SOLENOID, LOW) ;

  // Initialize display
  ss.tftReset();                  // Reset the display
  ss.setBacklight(1);             // Adjust backlight (this doesn't really seem to work unless you do -1 to turn it off)
  tft.initR(INITR_MINI160x80);    // Initialize a ST7735S chip, mini display
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  // Initialize SD
  SdFile::dateTimeCallback(dateTime);
  CreateDataFile();
  writeHeader();

  // Initialize RTC
  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();

  // Initialize neopixel
  pixels.begin();

  //start SPI flash
  Serial.print("flash begin...");
  flash.begin();
  // Open file system on the flash
  if ( !fatfs.begin(&flash) ) {
    Serial.println("Error: filesystem is not existed. Please try SdFat_format example to make one.");
    while (1) yield();
  }
  Serial.println("done.");

  load_settings();

  // Initialize load cells
  analogWriteResolution(12);  // turn on 12 bit resolution
  scale.begin(DOUT, CLK);
  scale2.begin(DOUT2, CLK2);
  scale.tare();
  scale.set_scale(calibration_factor);
  scale2.tare();
  scale2.set_scale(calibration_factor);

  //start up menu
  start_up_menu();
  tft.fillScreen(ST77XX_BLACK);
}

/////////////////////////////////////////////////////////////////////////
// Load from settings.txt on SPI flash
/////////////////////////////////////////////////////////////////////////
void Force::load_settings() {
  Serial.println("*****************************");
  Serial.println("Loading device Settings:");
  //read settings from SPI flash
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    calibrated = true;
    Serial.println ("settings.txt found. Contents:");
    while (myFile.available()) {
      for (int i = 0; i < 12; i++) {
        settings_recalled[i] = myFile.parseInt();
        Serial.println(settings_recalled[i]);
      }
      myFile.read();
      // close the file:
      myFile.close();

      FRC = settings_recalled[0];
      req = settings_recalled[1];
      dispense_amount = settings_recalled[2];
      dispense_delay = settings_recalled[3];
      timeout_length = settings_recalled[4] ;
      ratio = settings_recalled[5];
      hold_time = settings_recalled [6];
      calibration_factor = settings_recalled[7];
      calibration_factor2 = settings_recalled[8];
      PR = settings_recalled[9];
      trials_per_block = settings_recalled[10];
      max_force = settings_recalled[11];
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// Save to settings.txt on SPI flash
/////////////////////////////////////////////////////////////////////////
void Force::save_settings() {
  Serial.println("*****************************");
  Serial.println("Saving device Settings:");
  //open and delete the settings file
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  if (myFile) {
    Serial.print ("settings.txt found, deleting.... ");
    myFile.remove();
    myFile.close();
    Serial.println ("done.");
  }

  settings[0] = FRC;
  settings[1] = req;
  settings[2] = dispense_amount;
  settings[3] = dispense_delay;
  settings[4] = timeout_length;
  settings[5] = ratio;
  settings[6] = hold_time;
  settings[7] = calibration_factor;
  settings[8] = calibration_factor2;
  settings[9] = PR;
  settings[10] = trials_per_block;
  settings [11] = max_force;

  //rewrite settings file
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  Serial.print ("re-creating settings.txt file.");
  if (myFile) {
    for (int i = 0; i < 12; i++) {
      myFile.print(settings[i]);   // These are my settings
      myFile.print(",");   // These are my settings
      Serial.print(".");
    }
    myFile.close();
    Serial.println("done.");
  }
  
  //reopen and read back file on QSPI flash 
  Serial.print("Reading settings.txt back...");
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("opened...contents: ");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
}

/////////////////////////////////////////////////////////////////////////
// reset settings
/////////////////////////////////////////////////////////////////////////
void Force::reset_settings() {
  Serial.println("*****************************");
  Serial.println("Reseting device settings:");
  FRC = 1;
  req = 2;
  dispense_amount = 20;
  dispense_delay = 4;
  timeout_length = 10;
  ratio = 1;
  hold_time = 350;
  calibration_factor = -3300;
  calibration_factor2 = -3300;
  PR = 0;
  trials_per_block = 10;
  max_force = 20;
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(40, 35);
  tft.setTextColor(ST7735_WHITE);
  tft.println("Settings reset");
  start_up_menu();
}

/////////////////////////////////////////////////////////////////////////
// print settings
/////////////////////////////////////////////////////////////////////////
void Force::print_settings() {
  Serial.println("*****************************");
  Serial.println("Printing local device settings:");
  Serial.print("Device#: "); Serial.println(FRC);
  Serial.print("Req: "); Serial.println(req);
  Serial.print("dispense_amount: "); Serial.println(dispense_amount);
  Serial.print("dispense_delay: "); Serial.println(dispense_delay);
  Serial.print("timeout_length: ");  Serial.println(timeout_length);
  Serial.print("ratio: "); Serial.println(ratio);
  Serial.print("hold_time: "); Serial.println(hold_time);
  Serial.print("calibration_factor: "); Serial.println(calibration_factor);
  Serial.print("calibration_factor2: "); Serial.println(calibration_factor2);
  if (PR==0) Serial.println("Fixed Ratio");
  if (PR==1) Serial.println("Prog Ratio");
  Serial.print ("Trials per block: "); Serial.println(trials_per_block);
  Serial.print ("Max force: "); Serial.println(max_force);
  Serial.println(" ");
  
  Serial.print("Reading from SPI flash...");
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("reading contents of settings.txt...");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  }
}

/////////////////////////////////////////////////////////////////////////
// Buttons Functions 
/////////////////////////////////////////////////////////////////////////
void Force::check_buttons() {
  uint32_t buttons = ss.readButtons();
    
  if (! (buttons & TFTWING_BUTTON_A)) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 50)); //Light Neopixel blue
    pixels.show();
  }
  
 if ((! (buttons & TFTWING_BUTTON_A)) and ! (buttons & TFTWING_BUTTON_B)){
    delay (1000);
    uint32_t buttons = ss.readButtons();
    if ((! (buttons & TFTWING_BUTTON_A)) and ! (buttons & TFTWING_BUTTON_B)){
      pixels.setPixelColor(0, pixels.Color(50, 0, 0)); //Light Neopixel red
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 35);  
      tft.setTextColor(ST7735_WHITE);
      tft.println("Solenoid flush");   
      pixels.show();
      digitalWrite(SOLENOID, HIGH);
      tft.setCursor(40, 50);  
      tft.print("5..");
      delay (1000);
      tft.print("4..");
      delay (1000);
      tft.print("3..");
      delay (1000);
      tft.print("2..");
      delay (1000);
      tft.print("1..");
      delay (1000);
      digitalWrite(SOLENOID, LOW);
      tft.fillScreen(ST77XX_BLACK);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// Display Functions 
/////////////////////////////////////////////////////////////////////////
void Force::UpdateDisplay(){
  graphLegend();
  graphData();
  graphDateTime();
}

void Force::graphData() {
  //Calculate datapoints to graph
  lasty = y;
  lasty2 = y2;
  y = map(outputValue, 0, 750, 0, divideLine);  //scale to the screen
  if (y > divideLine) y = divideLine;
  y2 = map(outputValue2, 0, 750, 0, divideLine);  //scale to the screen
  if (y2 > divideLine) y2 = divideLine;

  // Clear display in front of graph
  if (x == 0) {
    tft.fillRect(x, 81 - divideLine, 6, divideLine, ST7735_BLACK); //To remove the first bar
    tft.fillRect(0, 0, 160, divideLine, ST7735_BLACK); //To remove the first bar
  }
  tft.drawLine(x + 7, 78, x + 7, 25, ST7735_RED);
  tft.fillRect(x + 1, 81 - divideLine, 6, divideLine, ST7735_BLACK);

  //Graph data load cell 1:
  tft.drawPixel(x + 1, 79 - y, ST7735_YELLOW);
  tft.drawLine(x, 80 - lasty , x + 1, 80 - y, ST7735_MAGENTA);

  //Graph data load cell 2:
  tft.drawPixel(x + 1, 79 - y2, ST7735_WHITE);
  tft.drawLine(x, 80 - lasty2 , x + 1, 80 - y2, ST7735_CYAN);

  //reset graphing position to left side of screen
  x++;
  if (x == 160) x = 0;
}

void Force::graphDateTime() {
  DateTime now = rtc.now();
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(85, 68);
  if (now.month() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.month(), DEC);
  tft.print('/');
  if (now.day() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.day(), DEC);
  tft.print(' ');
  tft.print(now.hour(), DEC);
  tft.print(':');
  if (now.minute() < 10)
    tft.print('0');      // Trick to add leading zero for formatting
  tft.print(now.minute(), DEC);
}

void Force::graphLegend() {
  // Print force output on F1 and F2
  if (grams > 1 or grams2 >1){ //only clear F1 ans F2 values if levers are being pushed
    tft.fillRect(12, 0, 38, 24, ST7735_BLACK); // clear the text after label
  }
  tft.setCursor(0, 5);  
  tft.setTextColor(ST7735_MAGENTA);
  tft.print("F1: ");   
  tft.println(grams,0);
  tft.setCursor(0, 17); 
  tft.setTextColor(ST7735_CYAN);
  tft.print("F2: ");
  tft.print(grams2,0);
 
  // Print force requirement
  tft.setCursor(45, 5);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Req: ");
  tft.print(req);
  tft.print("g");

  // Print trial 
  tft.setCursor(45, 17);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Trial:");
  if (grams > 1 or grams2 >1){
    tft.fillRect(80, 17, 24, 12, ST7735_BLACK); // clear task data on each trial
  }
  tft.print(trial);

  // Print FR ratio
  tft.setCursor(110, 5);
  tft.setTextColor(ST7735_YELLOW);
  if (PR ==0) tft.print("FR:");
  if (PR ==1) tft.print("PR:");
  tft.print(ratio);

  // Print current press
  tft.setCursor(110, 17);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Press:");
  if (grams > 1 or grams2 >1){
    tft.fillRect(143, 17, 28, 12, ST7735_BLACK); // clear press data on each trial
  }
  tft.print(presses);

  //Indicate licks
  tft.fillRect(0, 27, 40, 12, ST7735_BLACK); // clear the text after label
  if (lick == true) {
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(0, 28);
    tft.print ("Lick");
    digitalWrite(A3, HIGH);
    DateTime now = rtc.now();
    lickTime = now.unixtime();
  }

  if (lick == false) {

    digitalWrite(A3, LOW);

  }
  
  if (calibrated == false){
    tft.setCursor(85, 56);
    tft.print ("Uncalibrated");
  }
}

/////////////////////////////////////////////////////////////////////////
// Logging Functions 
/////////////////////////////////////////////////////////////////////////
void Force::CreateDataFile() {
  //put this next line *Right Before* any file open line:
  SdFile::dateTimeCallback(dateTime);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect, SD_SCK_MHZ(4))) {
     error(1);
  }

  // Name filename in format F###_MMDDYYNN, where MM is month, DD is day, YY is year, and NN is an incrementing number for the number of files initialized each day
  strcpy(filename, "FRC_____________.CSV");  // placeholder filename
  getFilename(filename);

  logfile = SD.open(filename, FILE_WRITE);
  if ( ! logfile ) {
    Serial.println("SD Create error");
    error(2);
  }
}

// Write data header to file of uSD.
void Force::writeHeader() {
  logfile.println("MM:DD:YYYY hh:mm:ss, Seconds, Device_Number, ProgressiveRatio, Grams_req, Hold_time, Ratio, Dispense_amount, Dispense_delay, Timeout, Trials_per_block, Max_force, Trial, Press, Lever1_Grams, Lever2_Grams, Licks, Dispense, Random_Num, Shock_trial");
}

// Print data and time followed by pellet count and motorturns to SD card
void Force::WriteToSD() {
  DateTime now = rtc.now();
  logfile.print(now.month());
  logfile.print("/");
  logfile.print(now.day());
  logfile.print("/");
  logfile.print(now.year());
  logfile.print(" ");
  logfile.print(now.hour());
  logfile.print(":");
  if (now.minute() < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(now.minute());
  logfile.print(":");
  if (now.second() < 10)
    logfile.print('0');      // Trick to add leading zero for formatting
  logfile.print(now.second());
  logfile.print(",");
  
  logfile.print((millis()-start_time)/1000.0000); //print seconds since start
  logfile.print(",");
  
  logfile.print(FRC); // Print device name
  logfile.print(",");
  
  if (PR==1) logfile.print("true"); // Print 
  if (PR==0) logfile.print("false"); // Print 
  logfile.print(",");
  
  logfile.print(req); // Print for requirement
  logfile.print(",");
  
  logfile.print(hold_time); 
  logfile.print(",");
  
  logfile.print(ratio);
  logfile.print(",");
  
  logfile.print(dispense_amount);
  logfile.print(",");
  
  logfile.print(dispense_delay);
  logfile.print(",");
  
  logfile.print(timeout_length);
  logfile.print(",");
  
  logfile.print(trials_per_block);
  logfile.print(",");
  
  logfile.print(max_force);
  logfile.print(",");
 
  logfile.print(trial);
  logfile.print(",");
  
  logfile.print(presses);
  logfile.print(",");
  
  logfile.print(grams);
  logfile.print(",");
  
  logfile.print(grams2);
  logfile.print(",");
  
  logfile.print(lick);
  logfile.print(",");

  logfile.print(dispensing);
  logfile.print(",");

  logfile.print(random_number);
  logfile.print(",");
  
  logfile.println(shock);

  logfile.flush();

  if ( ! logfile ) {
    error(2);
  }
}

/********************************************************
  If any errors are detected with the SD card print on the screen
********************************************************/
void Force::error(uint8_t errno) {
  tft.setCursor(5, 48);
  tft.print("Check SD card");
}

/********************************************************
  This function creates a unique filename for each file that
  starts with "FRC", then the date in MMDDYY,
  then an incrementing number for each new file created on the same date
********************************************************/
void Force::getFilename(char *filename) {
  DateTime now = rtc.now();

  filename[3] = FRC / 100 + '0';
  filename[4] = FRC / 10 + '0';
  filename[5] = FRC % 10 + '0';
  filename[7] = now.month() / 10 + '0';
  filename[8] = now.month() % 10 + '0';
  filename[9] = now.day() / 10 + '0';
  filename[10] = now.day() % 10 + '0';
  filename[11] = (now.year() - 2000) / 10 + '0';
  filename[12] = (now.year() - 2000) % 10 + '0';
  filename[16] = '.';
  filename[17] = 'C';
  filename[18] = 'S';
  filename[19] = 'V';
  for (uint8_t i = 0; i < 100; i++) {
    filename[14] = '0' + i / 10;
    filename[15] = '0' + i % 10;

    if (! SD.exists(filename)) {
      break;
    }
  }
  return;
}

void Force::logdata() {
  WriteToSD();
}

/////////////////////////////////////////////////////////////////////////
// Load cell Functions 
/////////////////////////////////////////////////////////////////////////
void Force::Sense() {
  grams = (scale.get_units());
  grams2 = (scale2.get_units());
  if (grams < 0) grams = 0;
  if (grams2 < 0) grams2 = 0;
  
  if (grams < req){
    pressTime = millis();
    pressLength = 0;
  }
  
  if (grams > req) {
    pressLength = (millis() - pressTime);
  }
  
    
  outputValue = map(grams, 0, 200, 0, 4095);
  outputValue2 = map(grams2, 0, 200, 0, 4095);
 
  if (outputValue > 4000) outputValue = 4000;
  if (outputValue < 1) outputValue = 0;
  if (outputValue2 > 4000) outputValue2 = 4000;
  if (outputValue2 < 1) outputValue2 = 0;

  analogWrite(A0, outputValue2);
  analogWrite(A1, outputValue);
  
  scaleChange += abs(outputValue - lastReading);
  scaleChange2 += abs(outputValue2 - lastReading2);

  lastReading = outputValue;
  lastReading2 = outputValue2;
  
  //control pixel color based on load cells 
  pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  pixels.show();

  lick = digitalRead(18) == HIGH;
  Tare();
  check_buttons();
}

void Force::Tare() {
  if (millis() - start_timer > 5000)  {
    if (scaleChange < 1000) {  // this sets sensitivity for delaying taring
      pixels.setPixelColor(0, pixels.Color(0, 10, 10));
      pixels.show();
      scale.tare();
    }
    if (scaleChange2 < 1000) {
      pixels.setPixelColor(0, pixels.Color(10, 10, 0));
      pixels.show();
      scale2.tare();
    }
    start_timer = millis();
    scaleChange  = 0;
    scaleChange2  = 0;
  }
}

/////////////////////////////////////////////////////////////////////////
// Serial Output
/////////////////////////////////////////////////////////////////////////
void Force::SerialOutput() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print ("  ");
  Serial.print("   Version: ");
  Serial.print(library_version);
  Serial.print("   Trial: ");
  Serial.print(trial);
  Serial.print("   Force1: ");
  Serial.print(grams,0);
  Serial.print("   Force2: ");
  Serial.println(grams2,0);
}

/////////////////////////////////////////////////////////////////////////
// Calibration function
/////////////////////////////////////////////////////////////////////////
void Force::Calibrate(){
  bool lever1 = true;
  tft.fillScreen(ST77XX_BLACK);
  scale.tare();
  scale2.tare();
  while (calibrate_active==true){
    uint32_t buttons = ss.readButtons();
    if (! (buttons & TFTWING_BUTTON_A)) lever1 = true;
    if (! (buttons & TFTWING_BUTTON_B)) lever1 = false;
    
    float calibrate_timer = millis();
    tft.setCursor(40, 15);  
    tft.setTextColor(ST7735_WHITE);
    tft.println("Calibrate levers:");   
    grams = (scale.get_units());
    grams2 = (scale2.get_units());
    tft.setCursor(40, 30);  
    tft.print("Lever 1:");   
    tft.println(grams,1);   
    tft.setCursor(40, 45);  
    tft.print("Lever 2:");   
    tft.println(grams2,1);   
    delay (100);
    if (lever1 == true){
        tft.fillRect(0, 30, 160, 12, ST7735_BLUE); // highlight active bar
        tft.fillRect(0, 43, 160, 12, ST7735_BLACK); // highlight active bar

        if (! (buttons & TFTWING_BUTTON_UP)) { 
          calibration_factor += 100;
          scale.set_scale(calibration_factor);
        }
        
        if (! (buttons & TFTWING_BUTTON_DOWN)) { 
          calibration_factor -= 100;
          scale.set_scale(calibration_factor);
        }
    }
    
    if (lever1 == false){
        tft.fillRect(0, 30, 160, 12, ST7735_BLACK); // highlight active bar
        tft.fillRect(0, 43, 160, 12, ST7735_BLUE); // highlight active bar
        if (! (buttons & TFTWING_BUTTON_UP)) { 
          calibration_factor += 100;
          scale2.set_scale(calibration_factor2);
        }
        
        if (! (buttons & TFTWING_BUTTON_DOWN)) { 
          calibration_factor -= 100;
          scale2.set_scale(calibration_factor2);
        }
    }

    if (! (buttons & TFTWING_BUTTON_SELECT)) {
      Click();
      start_timer = millis();
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 15);  
      tft.println("Remove weights");
      calibrated = true;
      delay (2000);
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 15);  
      tft.println("Calibrated!");
      delay (1000);
      start_up_menu();
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// FORCE menu
/////////////////////////////////////////////////////////////////////////
void Force::start_up_menu() {
  calibrate_active = false;
  print_settings();
  float start_timer = millis();
  int option = 0;
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
  while (start_up == true) {
    int page = 1;
    //////////////////////////////////////////////
    ////////////////////// PAGE 1 ////////////////
    //////////////////////////////////////////////
    while (page == 1) {
      if ((millis() - start_timer) > 10000) start_up = false; //after 10 seconds of start up menu, start session
      uint32_t buttons = ss.readButtons();
      tft.setCursor(40, 5);
      tft.setTextColor(ST7735_MAGENTA);
      tft.println("FR Menu");

      tft.setCursor(0, 20);
      tft.setTextColor(ST7735_CYAN);

      //option 0
      tft.print("device #:       ");
      tft.println(FRC);
      if (option == 0) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          FRC ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          FRC --;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 1
      tft.print("ratio:          ");
      tft.println(ratio);
      if (option == 1) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          ratio ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          ratio --;
          if (ratio < 0) ratio = 0;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 2
      tft.print("force_req:      ");
      tft.print(req);
      tft.println(" g");
      if (option == 2) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          req ++;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          req --;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 3
      tft.print("hold_time:      ");
      tft.print(hold_time);
      tft.println(" ms");
      if (option == 3) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          Click();
          hold_time += 10;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          Click();
          hold_time -= 10;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 4
      tft.print("dispense_delay: ");
      tft.print(dispense_delay);
      tft.println(" s");
      if (option == 4) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          dispense_delay += 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          dispense_delay -= 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
            
      //option 5
      tft.print("timeout:        ");
      tft.print(timeout_length);
      tft.println(" s");
      if (option == 5) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          Click();
          timeout_length += 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          Click();
          timeout_length -= 1;
          delay (250);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }


      //button up
      if (! (buttons & TFTWING_BUTTON_UP)) {
        option --;
        start_timer = millis();
        Click();
        if (option < 0) {
          tft.fillScreen(ST77XX_BLACK);
          option = 11;
          page = 2;
        }
        tft.fillRect(0, ((option+1) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
        tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        delay (150);
        save_settings();
        print_settings();
      }

      //button down
      if (! (buttons & TFTWING_BUTTON_DOWN)) {
        option ++;
        start_timer = millis();
        Click();
        tft.fillRect(0, ((option-1) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
        tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        delay (150);
        if (option > 5) {
          tft.fillScreen(ST77XX_BLACK);
          page = 2;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        save_settings();
        print_settings();
      }

      //button select
      if (! (buttons & TFTWING_BUTTON_SELECT)) {
        delay (500);
        uint32_t buttons = ss.readButtons();
        if (! (buttons & TFTWING_BUTTON_SELECT)) {
            Click();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(40, 35);
            tft.setTextColor(ST7735_WHITE);
            save_settings();
            print_settings();
            tft.println("Starting FORCE!");
            delay (250);
            start_time = millis();
            start_up = false;
            page = 0;
         }
      }
    }
    
    //////////////////////////////////////////////
    ////////////////////// PAGE 2 ////////////////
    //////////////////////////////////////////////
    tft.fillScreen(ST77XX_BLACK);
    tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
    print_settings();
    while (page == 2) {
      if ((millis() - start_timer) > 10000) start_up = false; //after 10 seconds of start up menu, start session
      uint32_t buttons = ss.readButtons();
      tft.setCursor(40, 5);
      tft.setTextColor(ST7735_MAGENTA);
      tft.println("PR Menu");

      tft.setCursor(0, 20);
      tft.setTextColor(ST7735_CYAN);

      //option 6
      tft.print("Prog ratio: ");
      if (PR == 0) tft.println("off");
      if (PR == 1) tft.println("on");
      if (option == 6) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          PR = 1;
          ratio = 1; //all PR sessions will have the FR ratio of 1
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          
        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          PR = 0;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 7
      tft.print("Trials per block: ");
      tft.println(trials_per_block);
      if (option == 7) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          trials_per_block ++;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          trials_per_block --;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 8
      tft.print("Max force: ");
      tft.print (max_force);
      tft.println(" g");
      if (option == 8) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          max_force ++;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          max_force--;
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 9
      tft.println(" ");
      if (option == 9) {
        if (! (buttons & TFTWING_BUTTON_RIGHT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, (((option - 6) - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar

        }
        if (! (buttons & TFTWING_BUTTON_LEFT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }

      //option 10
      tft.setTextColor(ST7735_RED);
      tft.println("Calibrate FORCE");
      if (option == 10) {
        if (! (buttons & TFTWING_BUTTON_RIGHT) or ! (buttons & TFTWING_BUTTON_SELECT)) {
          start_timer = millis();
          Tone();
          delay (250);
          calibrate_active = true;
          Calibrate();
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
      }
      
      //option 11
      tft.setTextColor(ST7735_RED);
      tft.println("Reset settings");
      if (option == 11) {
        if (! (buttons & TFTWING_BUTTON_RIGHT) or ! (buttons & TFTWING_BUTTON_SELECT)) {
          start_timer = millis();
          delay (250);
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          reset_settings();
        }
      }

      //button up
      if (! (buttons & TFTWING_BUTTON_UP)) {
        option --;
        start_timer = millis();
        Click();
        if ((option <= 11) and (option > 5)){
          tft.fillRect(0, ((option - 5) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        delay (150);
        if (option < 6) {
          tft.fillScreen(ST77XX_BLACK);
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          page = 1;
        }
        save_settings();
        print_settings();
      }

      //button down
      if (! (buttons & TFTWING_BUTTON_DOWN)) {
        option ++;
        start_timer = millis();
        Click();
        if (option <=11){
          tft.fillRect(0, ((option - 7) * 8) + 19, 160, 9, ST7735_BLACK); // erase current bar
          tft.fillRect(0, ((option - 6) * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
        }
        delay (150);
        if (option > 11) {
          tft.fillScreen(ST77XX_BLACK);
          option = 0;
          tft.fillRect(0, (option * 8) + 19, 160, 9, ST7735_BLUE); // highlight active bar
          page = 1;
        }
        save_settings();
        print_settings();
      }

      //button select
      if (! (buttons & TFTWING_BUTTON_SELECT)) {
        delay (500);
        uint32_t buttons = ss.readButtons();
        if (! (buttons & TFTWING_BUTTON_SELECT)) {
            Click();
            tft.fillScreen(ST77XX_BLACK);
            tft.setCursor(40, 35);
            tft.setTextColor(ST7735_WHITE);
            save_settings();
            print_settings();
            tft.println("Starting FORCE!");
            delay (250);
            start_time = millis();
            start_up = false;
            page = 0;
         }
      }
    }
    
  }
}
