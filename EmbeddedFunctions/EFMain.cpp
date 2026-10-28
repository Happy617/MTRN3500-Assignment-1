#include "EmbeddedFunctions.h"

int main() {
	EmbeddedFunctions^ ef = gcnew EmbeddedFunctions();

	try {
		ef->GOpen("127.0.0.1 -d", 26000);
		Console::WriteLine("Opened");

		Console::WriteLine("Response: " + ef->GCommand("TI 0"));

		ef->GClose();
		Console::WriteLine("Closed");
	}
	catch (Exception^ e) {
		Console::WriteLine("Error: " + e->Message);
	}

	//   netstat -ano | findstr ESTABLISHED to find port

	Console::ReadLine();   // keeps the window open
	return 0;
}