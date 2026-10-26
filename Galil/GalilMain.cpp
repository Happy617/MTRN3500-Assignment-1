#include "Galil.h"

int main(void) {

	// For physical Galil
	// EmbeddedFunctions function;

	EmbeddedFunctions function(true);
	Galil g(&function, "192.168.0.120 -d");

	g.DigitalOutput(0x0000);

	return 0;
}