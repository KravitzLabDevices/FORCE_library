/*
FORCE example code
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        DON'T EDIT THESE LINES
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Force.h>                                      //Include FORCE library 
String ver = "v1.1.0";                                  //unique identifier text
Force force(ver);                                       //start FORCE object

void setup() {
  force.begin();                                        //setup FORCE
}

void loop() {
  force.run();                                          //call force.run() at least once per loop

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                      WRITE CUSTOM BEHAVIORAL CODE BELOW HERE
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (force.pressLength > force.hold_time) {                  //if the force lever is held down for longer than the hold_req
    force.presses++;                                          //keep count of successful presses
    if (force.presses == force.ratio) {                       //if the ratio for number of presses is met
      if (force.PR == true) {                                 //if it's a progressive ratio session
        if ((force.trial % force.trials_per_block) == 0) {    //if the number of trials_per_block is reached 
          force.req = force.req + 2;                          //inrease force requirement 
          if (force.req > force.max_force) force.req = 2;     // if force.req breaks max_force, reset to 2g
        }
      }
      force.Dispense();                                       //dispense reward
      force.presses = 0;                                      //reset presses
    }
    else {
      force.Timeout(force.timeout_length);                    //timeout (length in seconds)
    }
  }
}
