#include "EmbeddedFunctions.h"


EmbeddedFunctions::EmbeddedFunctions() {
	String^ Address = "192.168.0.120";
	int Port = 23;


	TcpClient^ GalilMngHndl = gcnew TcpClient(Address, Port);

	GalilMngHndl->SendBufferSize = 64;
	GalilMngHndl->ReceiveBufferSize = 2048;
	GalilMngHndl->SendTimeout = 500;
	GalilMngHndl->ReceiveTimeout = 500;
	GalilMngHndl->NoDelay = true;


}

EmbeddedFunctions::~EmbeddedFunctions() {

}

void EmbeddedFunctions::GOpen(String^ address, const int port) {

}

void EmbeddedFunctions::GClose() {

}

String^ EmbeddedFunctions::GCommand(String^ command) { 
	return "";
}