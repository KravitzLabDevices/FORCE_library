/*
  FORCE v1.0.0
  Fixed ratio example code.  
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        EDIT THESE VARIABLES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int device_number = 2;                                  //give the device a unique number
int ratio = 3;                                          //how many presses before dispensing?
int force_req = 20;                                     //how hard to push?
int hold_time = 350;                                    //how long to hold the force?
int timeout_length = 1;                                 //how long is the timeout between trials?
int dispense_delay = 4;                                 //how many seconds between successful press and dispense?
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        DON'T EDIT THESE LINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Force.h>                                      //Include FORCE library 
String sketch = "FR_N";                                 //unique identifier text for each sketch
Force force(sketch);                                    //start FORCE object

void setup() {
  force.begin();                                        //setup FORCE
  force.req = force_req;                                //set force requirement
  force.dispense_delay = dispense_delay;                //set dispense delay
  force.timeout_length = timeout_length;                //set timeout length
  force.FRC = device_number;                            //set device number (recorded in log file)
}

void loop() {
  force.run();                                          //call force.run() at least once per loop
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      WRITE YOUR BEHAVIORAL CODE BELOW HERE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (force.pressLength > hold_time) {                  //if the force lever is held down for longer than the hold_req
    force.presses++;                                    //keep count of successful presses
    if (force.presses == ratio) {                       //if the number of ratio is met
      force.Dispense(20);                               //open solenoid to dispense (for length of time in ms)
      force.presses = 0;                                //reset presses
    }
    else {
      force.Timeout(timeout_length);                    //timeout (length in seconds)
    }
  }
}
