#include "Galil.h"

int main(void) {

	EmbeddedFunctions function(true);
	Galil g(&function, "192.168.0.120 -d");

	return 0;
}