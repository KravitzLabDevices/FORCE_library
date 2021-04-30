/*
  FORCE v1.0.0
*/

#include <Force.h>                                      //Include FORCE library 
String sketch = "FR1";                                  //Unique identifier text for each sketch
Force force(sketch);                                    //Start FORCE object

void setup() {
  Serial.begin(9600);
  force.begin();
  force.req = 20;                                       //force requirement
  force.timeoutLength = 2;                              //timeout in seconds
  force.pressLength = 300;                              //minimum press length in ms
}

void loop() {
  force.run();                                          //call force.run() at least once per loop 
  if (force.grams > force.req) {
    force.Dispense();
    force.Timeout();
  }
}
