/*
  FORCE v1.0.0
  Fixed ratio example code
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        EDIT THESE VARIABLES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int force_req = 20;                                     //how hard to push?
int hold_req = 350;                                     //how long to hold the force?
int timeoutLength = 2;                                  //how long is the timeout between trials?
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Force.h>                                      //Include FORCE library 
String sketch = "FR1";                                  //Unique identifier text for each sketch
Force force(sketch);                                    //Start FORCE object

void setup() {
  force.begin();                                        //setup FORCE
  force.req = force_req;                                //set force requirement
  force.timeoutLength = timeoutLength;                  //timeout (in seconds), defaults to 2s
}

void loop() {
  force.run();                                          //call force.run() at least once per loop
  if (force.pressLength > hold_req) {                   //if the force lever is held down for longer than the hold_req
    force.Dispense(20);                                 //dispense. Default solenoid open length is 20ms, which can be changed, ie: force.Dispense (50);
  }
}
