/*
FORCE v1.0.0
*/
#include <Force.h>                                      //Include FORCE library 
String sketch = "FR1";                                  //Unique identifier text for each sketch
Force force(sketch);                                    //Start FORCE object

void setup() {
  Serial.begin(9600);
  force.begin();
}

void loop() {
  force.run();
  if (force.grams > 50) {
    force.Dispense();
    force.Timeout();
  }
}
