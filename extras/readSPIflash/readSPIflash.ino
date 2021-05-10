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

int time_out = 0;
int settings[8];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(1); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Initializing Filesystem on external flash...");

  // Init external flash
  flash.begin();

  // Open file system on the flash
  if ( !fatfs.begin(&flash) ) {
    Serial.println("Error: filesystem is not existed. Please try SdFat_format example to make one.");
    while (1) yield();
  }

  // open the settings file for reading:
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("Reading settings.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      for (int i = 0; i < 8; i++) {
        settings[i] = myFile.parseInt();
        Serial.println(settings[i]);
      }
      // close the file:
      myFile.close();
      time_out = settings[1];
      Serial.print ("Post-reading timeout: ");
      Serial.println(time_out);
    }
  }
}

void loop() {
  // nothing happens after setup
}
