/*
	Left lever is active, when mouse pushes it with >2g force for >300ms it triggers dispense for 200ms, and then starts a 30s timeout
*/

#include <Force.h>

#define FORCE_REQ_g         2
#define PRESS_LENGTH_ms     300
#define DISPENSE_LENGTH_ms  200
#define TIMEOUT_ms          30000

Force feeder(FORCE_REQ_g, PRESS_LENGTH_ms, DISPENSE_LENGTH_ms, TIMEOUT_ms);

void setup() {
  Serial.begin(9600);
  feeder.begin();
}

void loop() {
  feeder.run();
}
