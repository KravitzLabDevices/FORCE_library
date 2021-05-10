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

Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
File myFile;

int settings_recalled[8];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(1); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing Filesystem on external flash...");

  // Init external flash
  flash.begin();
  // Open file system on the flash
  if ( !fatfs.begin(&flash) ) {
    Serial.println("Error: filesystem is not existed. Please try SdFat_format example to make one.");
    while (1) yield();
  }
  Serial.println("initialization done.");

  // open the file and delete it
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  if (myFile) {
    myFile.remove();
  }

  //reopen file
  myFile = fatfs.open("settings.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to settings.txt...");
    Serial.println("1, 2, 3, 4, 5, 6, 7, 8");
    myFile.println("1, 2, 3, 4, 5, 6, 7, 8");
    // close the file:
    myFile.close();
    Serial.println("...done.");
  }

  // re-open the file for reading:
  myFile = fatfs.open("settings.txt");
  if (myFile) {
    Serial.println("Opened settings.txt.");

    // read from the file until there's nothing else in it:
    Serial.println("Contents of settings.txt: ");
    while (myFile.available()) {
      for (int i = 0; i < 8; i++) {
        settings_recalled[i] = myFile.parseInt();
        Serial.print(i);
        Serial.print(": ");
        Serial.println(settings_recalled[i]);
      }
      break;
    }
    // close the file:
    myFile.close();
    Serial.println("settings_recalled:");
    for (int i = 0; i < 8; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(settings_recalled[i]);
    }
  }
}

void loop() {
  // nothing happens after setup
}
