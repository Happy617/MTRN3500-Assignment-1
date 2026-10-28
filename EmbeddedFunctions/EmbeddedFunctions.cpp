#include "EmbeddedFunctions.h"

#include <cstdint>
using namespace System::Text;
using namespace System::IO;

constexpr int dataSize = 2048;
constexpr int timeoutMilliseconds = 500;
constexpr int minPort = 1;
constexpr int maxPort = 65535;

//EmbeddedFunctions::EmbeddedFunctions() {
//	String^ Address = "192.168.0.120";
//	int Port = 23;
//
//
//	TcpClient^ GalilMngHndl = gcnew TcpClient(Address, Port);
//
//	GalilMngHndl->SendBufferSize = 64;
//	GalilMngHndl->ReceiveBufferSize = 2048;
//	GalilMngHndl->SendTimeout = 500;
//	GalilMngHndl->ReceiveTimeout = 500;
//	GalilMngHndl->NoDelay = true;
//
//
//}

EmbeddedFunctions::EmbeddedFunctions() : GalilMngHndl(nullptr), GalilStream(nullptr) {}

EmbeddedFunctions::~EmbeddedFunctions() {
	GClose();
}

void EmbeddedFunctions::GOpen(String^ address, const int port) {
	// Reject bad arguments before touching the existing connection
	if (String::IsNullOrWhiteSpace(address)) {
		throw gcnew ArgumentException("Address must not be empty.", "address");
	}
	if (port < minPort || port > maxPort) {
		throw gcnew ArgumentOutOfRangeException("port", port, "Port must be between 1 and 65535.");
	}

	// Close any existing connection before opening a new one
	GClose();

	// Keep only the IP part so addresses like "192.168.0.120 -d" also work
	String^ IPAddress = address->Trim()->Split(' ')[0];
	try {
		GalilMngHndl = gcnew TcpClient(IPAddress, port);
		GalilMngHndl->SendTimeout = timeoutMilliseconds;
		GalilMngHndl->ReceiveTimeout = timeoutMilliseconds;
		GalilMngHndl->NoDelay = true;
		GalilStream = GalilMngHndl->GetStream();
	}
	catch (SocketException^ e) {
		GClose();
		throw gcnew IOException("Failed to open connection: " + e->Message, e);
	}
}

void EmbeddedFunctions::GClose() {
	if (GalilStream != nullptr) {
		GalilStream->Close();
		GalilStream = nullptr;
	}
	if (GalilMngHndl != nullptr) {
		GalilMngHndl->Close();
		GalilMngHndl = nullptr;
	}
}

String^ EmbeddedFunctions::GCommand(String^ command) {
	if (GalilMngHndl == nullptr || GalilStream == nullptr) {
		throw gcnew InvalidOperationException("Connection is not open.");
	}
	if (command == nullptr) {
		throw gcnew ArgumentNullException("command");
	}

	try {
		// Ensure the command ends with a semicolon
		if (!command->EndsWith(";")) {
			command += ";";
		}

		// Convert the command to bytes and send it
		array<uint8_t>^ sendData = Encoding::ASCII->GetBytes(command + "\r");
		GalilStream->Write(sendData, 0, sendData->Length);

		// Read the response
		array<uint8_t>^ receiveData = gcnew array<uint8_t>(dataSize);
		const int bytesRead = GalilStream->Read(receiveData, 0, receiveData->Length);
		if (bytesRead == 0) {
			throw gcnew IOException("The Galil closed the connection.");
		}

		// Convert the response to a string and return it
		return Encoding::ASCII->GetString(receiveData, 0, bytesRead);
	}
	catch (IOException^ e) {
		throw gcnew IOException("Communication error: " + e->Message, e);
	}
}