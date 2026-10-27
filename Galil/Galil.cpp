#include "Galil.h"

#define DEFAULT_ADDRESS "192.168.0.120 -d"

Galil::Galil() {
    g = nullptr;
    setPoint = 0;
    ControlParameters[0] = 0.0;   // Kp
    ControlParameters[1] = 0.0;   // Ki
    ControlParameters[2] = 0.0;   // Kd
    address = DEFAULT_ADDRESS;

    Functions = new EmbeddedFunctions(true);   // ture = simulator

    GReturn status = Functions->GOpen(DEFAULT_ADDRESS, &g);
    if (status != G_NO_ERROR) {
        // error code G_NO_ERROR found in gclib errors.h file
        std::cout << "Failed to open Galil connection. Error code: " << status << "\n";
    }
}

Galil::Galil(EmbeddedFunctions* Funcs, GCStringIn address) {
    g = nullptr;
    setPoint = 0;
    ControlParameters[0] = 0.0;   // Kp
    ControlParameters[1] = 0.0;   // Ki
    ControlParameters[2] = 0.0;   // Kd
    this->address = address;

    Functions = Funcs;  

    GReturn status = Functions->GOpen(address, &g);
    if (status != G_NO_ERROR) {
        // error code G_NO_ERROR found in gclib errors.h file
        std::cout << "Failed to open Galil connection. Error code: " << status << "\n";
    }
}

// This syntax means its of type Galil reference/memory address
Galil::Galil(const Galil& other) {
    g = nullptr;
    setPoint = other.setPoint;
    ControlParameters[0] = other.ControlParameters[0];   // Kp
    ControlParameters[1] = other.ControlParameters[1];   // Ki
    ControlParameters[2] = other.ControlParameters[2];   // Kd
    address = other.address;

    Functions = new EmbeddedFunctions(true);

    GReturn status = Functions->GOpen(DEFAULT_ADDRESS, &g);
    if (status != G_NO_ERROR) {
        // error code G_NO_ERROR found in gclib errors.h file
        std::cout << "Failed to open Galil connection. Error code: " << status << "\n";
    }
}

Galil::~Galil() {
    GReturn status = Functions->GClose(g);
    if (status != G_NO_ERROR) {
        std::cout << "Failed to close Galil connection. Error code: " << status << "\n";
    }

    Functions = nullptr;
}

void Galil::DigitalOutput(uint16_t value) {
    uint8_t low = value & 0xFF;          // last 8 bits, 0-7 least significant bits
    uint8_t high = (value >> 8) & 0xFF;   // first 8 bits, 8-15

    std::string command = "OP " + std::to_string(low) + "," + std::to_string(high) + ";";
    //std::string command = "OP " + std::to_string(low) + ";";

    char buffer[128];
    // needs c_str for command as it a standard c string that needs a bull-terminated string, char*
    Functions->GCommand(g, command.c_str(), buffer, sizeof(buffer), nullptr);
}

void Galil::DigitalByteOutput(bool bank, uint8_t value) {
    std::string command;

    if (bank == 0) {
        command = "OP " + std::to_string(value) + ";";
    }
    else {
        command = "OP " + std::to_string(0) + "," + std::to_string(value) + ";";
    }

    char buffer[128];
    Functions->GCommand(g, command.c_str(), buffer, sizeof(buffer), nullptr);
}

void Galil::DigitalBitOutput(bool val, uint8_t bit) {
    std::string command = "OB " + std::to_string(bit) + "," + std::to_string(val ? 1 : 0) + ";";

    char buffer[128];
    Functions->GCommand(g, command.c_str(), buffer, sizeof(buffer), nullptr);
}