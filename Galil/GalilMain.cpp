#include "Galil.h"

#include <thread>
#include <chrono>

int main(void) {

	// For physical Galil
	// EmbeddedFunctions function;

	EmbeddedFunctions function(true);
	Galil g(&function, "192.168.0.120 -d");
	//
	//// Default const
	////Galil g;

	//// Copy con
	 Galil g2 = g;
	 g.StopsConnection();


	//g.DigitalOutput(0);
	//g.DigitalOutput(0b1000000000000000);


	//try {
	//	g.DigitalBitOutput(0, 20);
	//}
	//catch (const std::runtime_error& e) {
	//	std::cout << "Error: " << e.what() << "\n";
	//}
	g2.AnalogOutput(0, 0);

	std::cout << "Was it successful: " << g2.CheckSuccessfulWrite() << "\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//int value = g.DigitalBitInput(3);
	float value = g2.AnalogInput(0);
	std::cout << "Was it successful: " << g2.CheckSuccessfulWrite() << "\n";

	g2.AnalogInputRange(0, 1);
	std::cout << "Was it successful: " << g2.CheckSuccessfulWrite() << "\n";

	value = g2.AnalogInput(0);
	std::cout << "Was it successful: " << g2.CheckSuccessfulWrite() << "\n";

	std::cout << "Pin value: " << value << "\n";
	return 0;
}