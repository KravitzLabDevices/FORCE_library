/********************************************************
  Clock setting function for PCF8523 on the Adalogger Feather.  This will set the RTC to the current computer
  date/time and the device will maintain that time as long as a battery is installed.  The coin cell in
  the slot on the breakout board should last at least 5 years.

  Note: If the coin cell is removed from the RTC board it will lose the time.  Flash this code again to reset it.

  This project code includes code from:
  *** Adafruit, who made the hardware breakout boards and associated code ***

  This project is released under the terms of the Creative Commons - Attribution - ShareAlike 3.0 license:
  human readable: https://creativecommons.org/licenses/by-sa/3.0/
  legal wording: https://creativecommons.org/licenses/by-sa/3.0/legalcode
  Copyright (c) 2018 Lex Kravitz


********************************************************/

/********************************************************
  Include libraries
********************************************************/
#include <Wire.h>
#include "RTClib.h"

/********************************************************
  Setup RTC object
********************************************************/
RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  Serial.begin(57600);  //open serial monitor so you can see time output on computer via USB

  /********************************************************
      Set RTC to computer date and time
    ********************************************************/
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");  //This will trigger if RTC cannot be found
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");  //This will trigger if RTC was not set - it should show up the first time this code is run, but not again
    // following line sets the RTC to the date & time this sketch was compiled
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2018, 8, 24, 11, 20, 0));
}

void loop () {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future (now + TimeSpan(7, 12, 30, 6));

  Serial.print(" now + 7d + 30s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();

  Serial.println();
  delay(3000);
}
