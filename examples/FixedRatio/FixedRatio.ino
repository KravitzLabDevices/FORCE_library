/*
  FORCE v1.0.0
*/

#include <Force.h>                                      //Include FORCE library 
String sketch = "FR1";                                  //Unique identifier text for each sketch
Force force(sketch);                                    //Start FORCE object

void setup() {
  force.begin();                                        //setup FORCE device
  force.req = 20;                                       //force requirement
  force.timeoutLength = 2;                              //timeout (in seconds)
}

void loop() {
  force.run();                                          //call force.run() at least once per loop 
  if (force.grams > force.req) {                        //if press force is greater than req
    force.Dispense();                                   //dispense!
  }
}
