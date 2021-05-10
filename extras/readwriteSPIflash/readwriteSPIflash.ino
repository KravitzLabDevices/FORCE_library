/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

//SETUP SPI FLASH
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
File myFile;

int settings[8];
settings[0] = 2;
settings[1] = 20;
settings[2] = 200;
settings[3] = 2000;
settings[4] = 20000;
settings[5] = 200000;
settings[6] = 2000000;
settings[7] = 20000000;

int settings_recalled[8];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  //print settings
  Serial.println("Settings:");
  for (int i = 0; i < 8; i++) {
    Serial.println(settings[i]);
  }
  Serial.println(" ");

  //print settings_recalled
  Serial.println("Settings_recalled:");
  for (int i = 0; i < 8; i++) {
    Serial.println(settings_recalled[i]);
  }
  Serial.println(" ");

  //initialize SPI flash
  Serial.print("Initializing Filesystem on external flash...");
  // Init external flash
  flash.begin();
  // Open file system on the flash
  if ( !fatfs.begin(&flash) ) {
    Serial.println("Error: filesystem is not existed. Please try SdFat_format example to make one.");
    while (1) yield();
  }
  Serial.println("done.");

  //open and delete the settings file
  Serial.print("removing settings.txt....");
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  myFile.remove();
  Serial.println("done.");

  //re-create the settings file
  Serial.print("creating settings.txt....");
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  while (myFile.available()) {
    Serial.write(myFile.read());
  }
  // if the file opened okay, write to it:

  if (myFile) {
    Serial.println("done.");
    Serial.print("Writing to settings.txt...");
    //write settings to myFile
    for (int i = 0; i < 8; i++) {
      myFile.print(settings[i]);   // These are my settings
      myFile.print(",");   // These are my settings
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening settings.txt");
  }

  Serial.print("Re-opening settings.txt....");
  // re-open the file for reading:
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("done.");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      for (int i = 0; i < 8; i++) {
        settings_recalled[i] = myFile.parseInt();
      }
      myFile.read());
    }
    // close the file:
    myFile.close();

    Serial.println("Settings:");
    for (int i = 0; i < 8; i++) {
      Serial.println(settings[i]);
    }
    Serial.println(" ");

    Serial.println("Settings_recalled:");
    for (int i = 0; i < 8; i++) {
      Serial.println(settings_recalled[i]);
    }
    Serial.println(" ");
  }
}

void loop() {
  // nothing happens after setup
}
