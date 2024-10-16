#include "config.h"
#include "relais.h"

//// Version 2024 avec framework arduino c++

void setRelay(byte numRelay) {
	if (numRelay <= 4) {
		digitalWrite(relays_pins[numRelay], 1);
		bitWrite(relaysState, numRelay, 1);
		Serial.printf("relay #%d set", numRelay + 1);
	} else {
		Serial.println("setRelay : bad relay number");
	}
}

void resetRelay(byte numRelay) {
	if (numRelay <= 4) {
		digitalWrite(relays_pins[numRelay], 0);
		bitWrite(relaysState, numRelay, 0);
		Serial.printf("relay #%d set to 0", numRelay + 1);
	} else {
		Serial.println("setRelay : bad relay number");
	}
}

void setRelays(byte states) {
	int i;

	for (i=0; i<4; i++) {
		if (bitRead(states, i) == 1) {
			setRelay(i);
		} else {
			resetRelay(i);
		}
	}
}

void initialize_GPIOs(){
  int i;
  for (i = 0; i < 4; i++) {
	pinMode(i, OUTPUT);
	digitalWrite(relays_pins[i], relays_pins[i]);
  }
}

