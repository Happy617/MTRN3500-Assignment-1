#include "Galil.h"

int main(void) {

	// For physical Galil
	// EmbeddedFunctions function;

	EmbeddedFunctions function(true);
	Galil g(&function, "192.168.0.120 -d");

	//g.DigitalOutput(0x0001);
	//g.DigitalOutput(0b1000000000000000);

	//g.DigitalByteOutput(1, 3);
	g.DigitalBitOutput(0, 5);

	return 0;
}