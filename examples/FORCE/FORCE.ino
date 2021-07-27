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
#include <Force.h>                                      //Include FORCE library 
String ver = "v1.2.0";                                  //unique identifier text
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
      force.Tone();                                           //tone signals pending reward
      //This code block runs only if it's a progressive ratio session
      if (force.PR == true) {                                 //if it's a progressive ratio session
        if ((force.trial % force.trials_per_block) == 0) {    //if the number of trials_per_block is reached 
          force.req = force.req + 2;                          //increase force requirement 
          if (force.req > force.max_force) force.req = 2;     //if force.req breaks max_force, reset to 2g
        }
      }
      
      force.Dispense();                                       //dispense reward
      force.Timeout(force.timeout_length);                    //timeout (length in seconds)
      force.presses = 0;                                      //reset presses
    }
    else {
      force.Timeout(force.timeout_length);                    //timeout (length in seconds)
    }
  }
}
