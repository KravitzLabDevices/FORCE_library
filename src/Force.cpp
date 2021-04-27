#include "Arduino.h"
#include "Force.h"

/////////////////////////////////////////////////////////////////////////
// Initialize FORCE!
/////////////////////////////////////////////////////////////////////////
Force::Force(float _force_req, int _press_length, int _dispense_length, int _time_flag) {
  force_req = _force_req;
  press_length = _press_length;
  dispense_length = _dispense_length;
  time_flag = _time_flag;
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
// Begin and Run
/////////////////////////////////////////////////////////////////////////
void Force::begin() {
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
  graphLegend();

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

  // Initialize load cells
  analogWriteResolution(12);  // turn on 12 bit resolution
  scale.begin(DOUT, CLK);
  scale2.begin(DOUT2, CLK2);
  scale.tare();
  scale.set_scale(calibration_factor);
  scale2.tare();
  scale2.set_scale(calibration_factor);
}

void Force::run() {
  Sense();

  //Task (combine into one or move task to Arduino example)
  TaskReq();
  PressLengthReq();
  PlayTone();
  Dispense();
  Timeout();

  UpdateDisplay();
  SerialOutput();
  WriteToSD();
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
  
  if (! (buttons & TFTWING_BUTTON_B)) {
    pixels.setPixelColor(0, pixels.Color(50, 0, 0)); //Light Neopixel red
    pixels.show();
    digitalWrite(SOLENOID, HIGH);
    delay (5000);
    digitalWrite(SOLENOID, LOW);
  }
}

/////////////////////////////////////////////////////////////////////////
// Display Functions 
/////////////////////////////////////////////////////////////////////////
void Force::UpdateDisplay(){
  graphData();
  graphDateTime();
  graphLegend();
}

void Force::graphData() {
  //Calculate datapoints to graph
  lasty = y;
  lasty2 = y2;
  y = map(outputValue, 0, 4095, 0, divideLine);
  if (y > divideLine) y = divideLine;
  y2 = map(outputValue2, 0, 4095, 0, divideLine);
  if (y2 > divideLine) y2 = divideLine;

  // Clear display in front of graph
  if (x == 0) tft.fillRect(x, 81 - divideLine, 6, divideLine, ST7735_BLACK); //To remove the first bar
  tft.fillRect(x + 1, 81 - divideLine, 6, divideLine, ST7735_BLACK);

  //Graph data load cell 1:
  tft.drawPixel(x + 1, 79 - y, ST7735_YELLOW);
  tft.drawLine(x, 80 - lasty , x + 1, 80 - y, ST7735_MAGENTA);

  //Graph data load cell 2:
  tft.drawPixel(x + 1, 79 - y2, ST7735_WHITE);
  tft.drawLine(x, 80 - lasty2 , x + 1, 80 - y2, ST7735_CYAN);

  //reset graphing position on screen
  x++;
  if (x == 160) x = 0;
}

void Force::graphDateTime() {
  DateTime now = rtc.now();
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(85, 68);
  tft.print(now.month(), DEC);
  tft.print('/');
  tft.print(now.day(), DEC);
  tft.print(' ');
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
}

void Force::graphLegend() {
  // Print legend
  //  tft.fillRect(0, 0, 20, 25, ST7735_BLACK);
  tft.setCursor(0, 5);  tft.setTextColor(ST7735_MAGENTA);
  tft.println("F1");   tft.setTextColor(ST7735_CYAN);
  tft.println("F2");

  tft.setCursor(30, 5);
  tft.setTextColor(ST7735_YELLOW);

  tft.print("Mode:");
  // tft.print(mode);
  if (mode == 1) {
    tft.fillRect(57, 5, 15, 8, ST7735_BLACK);
    tft.print("PR ");
  }

  if (mode == 0) {
    tft.fillRect(57, 5, 15, 8, ST7735_BLACK);
    tft.print("FR ");
  }

  tft.print(" f_req: ");
  tft.fillRect(120, 5, 40, 8, ST7735_BLACK); // clear the text after label
  tft.print(force_req, 1);

  tft.setCursor(30, 15);
  if (mode == 1) {
    tft.fillRect(30, 15, 120, 8, ST7735_BLACK); // clear the text after "trial:"
    tft.print("Inc n:");
    tft.print(inc);
  }

  if (mode == 0) {
    tft.fillRect(30, 15, 120, 8, ST7735_BLACK); // clear the text after "trial:"
    tft.print("Trial n:");
    tft.print(FR_trials);
  }

  tft.print(" g1:");
  tft.print(grams, 1);

  //Visualize licks
  tft.setTextColor(ST7735_WHITE);
  tft.fillRect(125, 25, 12, 12, ST7735_BLACK); // clear the text after "trial:"
  tft.setCursor(125, 25);
  tft.print (' ');
  if (lick == true) {
    tft.print ("L");
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
  logfile.println("MM:DD:YYYY hh:mm:ss, Device_Number, Force_requirement, Millis, Seconds, ActiveGrams, InactiveGrams, Licks, Notes");
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
  logfile.print(FRC); // Print device name
  logfile.print(",");
  logfile.print(force_req); // Print for requirement
  logfile.print(",");
  logfile.print(millis()); //print millis since start
  logfile.print(",");
  logfile.print(millis()/1000.0000); //print seconds since start
  logfile.print(",");
  logfile.print(grams);
  logfile.print(",");
  logfile.print(grams2);
  logfile.print(",");  
  logfile.print(lick);
  logfile.print(",");   
  logfile.println("notes column");
  logfile.flush();

  if ( ! logfile ) {
    error(2);
  }
}

/********************************************************
  If any errors are detected with the SD card print on the screen
********************************************************/
void Force::error(uint8_t errno) {
  tft.setCursor(5, 68);
  switch (errno) {
    case 1:
      Serial.println("SD initialization error");
      tft.print("SD init error");
      break;
    default:
      tft.print("Card error "); tft.print(errno);
  }
  
  
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

void Force::Sense() {
  val = (scale.get_units());
  val2 = (scale2.get_units());
  outputValue = map(val, 0, 200, 0, 4095);
  outputValue2 = map(val2, 0, 200, 0, 4095);
 
  if (outputValue > 4000) outputValue = 4000;
  if (outputValue < 1) outputValue = 0;
  if (outputValue2 > 4000) outputValue2 = 4000;
  if (outputValue2 < 1) outputValue2 = 0;

  analogWrite(A0, outputValue2);
  analogWrite(A1, outputValue);
  
  change = (outputValue - lastReading);
  change2 = (outputValue2 - lastReading2);

  scaleChange += abs(change);
  scaleChange2 += abs(change2);

  lastReading = outputValue;
  lastReading2 = outputValue2;
  
  //control pixel color based on load cells 
  pixels.setPixelColor(0, pixels.Color(0, outputValue / 100, outputValue2 / 100)); 
  pixels.show();

  lick = digitalRead(18);
  Tare();
  check_buttons();
}

/////////////////////////////////////////////////////////////////////////
// TaskFunctions 
/////////////////////////////////////////////////////////////////////////
void Force::TaskReq() {
  grams = outputValue / 7.5;
  grams2 = outputValue2 / 7.5;
  /////////////////////////////////
  // mode 0, FR1
  /////////////////////////////////

  if (mode == 0 && trial == 1 && timeout_flag != 1) {
    //force_req = 2;
    if (grams > force_req) { //maybe put a stipulation in where this can't continuously be called, or re-zeroed, etc??///////////////////////////
      press_flag = 1;
    }
    if (press_duration_exceeded == 1) {
      dispense_flag = 1;
      tone_flag = 1;
      start_timer_disp = millis();
      start_timer_timeout = millis();

      timeout_flag = 1;
      FR_trials++;
      press_flag = 0;
      press_duration_exceeded = 0;
    }
  }
}

void Force::PlayTone() {
  if (tone_flag == 1) {
    tone(A5, 500, 200);
    tone_flag = 0;
  }
}

void Force::Dispense() {
  if (dispense_flag == 1) {
    press_duration_exceeded = 0;
    unsigned int time_lapse_disp = millis() - start_timer_disp;

    if (time_lapse_disp > 2000) {
      Serial.println("Dispensing!");
      digitalWrite(SOLENOID, HIGH);
      digitalWrite(A2, HIGH); // A2 will be "reward dispensed" pin
    }

    if (time_lapse_disp > (2000+dispense_length)) {
      digitalWrite(SOLENOID, LOW);
      digitalWrite(A2, LOW);

      start_timer_disp = millis(); // Reset for new trial
      dispense_flag = 0;
    }
  }
}

void Force::PressLengthReq() {
  time_lapse_lever_press = millis() - start_timer_lever_press;
  if (grams < force_req) {
    press_flag = 0;
    start_timer_lever_press = millis(); //reset
  }
  if (press_flag == 1) {
    if (time_lapse_lever_press > press_length) {
      press_duration_exceeded = 1;
    }
  }
}

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
  Serial.print(" Force1: ");
  Serial.print(grams);
  Serial.print(" Force2: ");
  Serial.println(grams2);
}

void Force::Timeout() {
  if (timeout_flag == 1) {
    time_lapse_timeout = millis() - start_timer_timeout;
    trial_flag=0;

    // reset timeout timer
    if (time_lapse_timeout > time_flag) {
      start_timer_timeout = millis(); //reset for new trial
      timeout_flag = 0;
      trial_flag=1;
    }
  }
}

