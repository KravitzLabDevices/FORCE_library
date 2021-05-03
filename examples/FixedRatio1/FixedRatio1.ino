/*
  FORCE v1.0.0
  Fixed ratio example code.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        EDIT THESE VARIABLES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int device_number = 2;                                  //give the device a unique number
int force_req = 20;                                     //how hard to push?
int hold_time = 350;                                    //how long to hold the force?
int dispense_length = 100;                              //how long to open solenoid (in ms)?
float dispense_delay = 4;                               //how many seconds between press and dispense?
float timeout_length = 10;                              //how long is the timeout between trials?
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        DON'T EDIT THESE LINES                                                
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Force.h>                                      //Include FORCE library                                 
String sketch = "FR1";                                  //unique identifier text for each sketch                
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
//                                      WRITE YOUR BEHAVIORAL CODE BELOW THIS LINE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (force.pressLength > hold_time) {                  //if the force lever is held down for longer than the hold_req
    force.Dispense(dispense_length);                    //open solenoid to dispense (for length of time in ms)
  }
}
