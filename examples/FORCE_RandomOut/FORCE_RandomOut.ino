/*
  FORCE example code
  by Lex Kravitz and Bridget Matikainen-Ankney

  alexxai@wustl.edu
  May, 2021

  KravitzLabDevices/FORCE_library is licensed under the GNU General Public License v3.0

  Permissions of this strong copyleft license are conditioned on making available complete source code of licensed works
  and modifications, which include larger works using a licensed work, under the same license. Copyright and license
  notices must be preserved. Contributors provide an express grant of patent rights.

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        DON'T EDIT THESE LINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Force.h>                                          //Include FORCE library 
String ver = "v1.2.0_randOut";                              //unique identifier text
Force force(ver);                                           //start FORCE object

void setup() {
  force.begin();                                            //setup FORCE
}

void loop() {
  force.run();                                              //call force.run() at least once per loop

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                      WRITE CUSTOM BEHAVIORAL CODE BELOW HERE
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (force.pressLength > force.hold_time) {                //if the force lever is held down for longer than the hold_req
    //Choose a random number
    randomSeed(millis());                                   //get a new randomSeed here so the sequence isn't the same every time
    force.random_number = random(1, 4);                     //Pick a random number between 1 and 3

    Serial.print("Random Num: ");                           //Print the random number to the Serial Monitor
    Serial.println(force.random_number);

    force.presses++;                                        //add 1 to force.presses

    if (force.random_number == 3) { //if random number is 3...
      force.shock = true;
      force.Tone();
      while ((force.unixtime - force.dispenseTime) < 15) {  //...and it is within 15 seconds of a dispense
        force.run();

        //keep force screen and timers running while waiting around for the 15sec delay
        if (force.lick == false) {                          //if animal licks in this time period:

          /////////////////////////////////////////////////
          // FORCE does this when both conditions are true
          /////////////////////////////////////////////////
          Serial.println("Output triggered!");
          digitalWrite (6, HIGH);
          delay(100);
          digitalWrite (6, LOW);
          break;
        }
      }
    }
    force.Dispense();
    force.shock = false;
    force.Timeout(force.timeout_length);                    //timeout (length in seconds)
  }
}
