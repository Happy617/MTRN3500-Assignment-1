#include "Galil.h"

#include <sstream>
#include <iomanip>
#include <utility>

constexpr const char* defaultAddress = "192.168.0.120 -d";
constexpr uint8_t maxDigitalBit = 15;
constexpr uint8_t maxNumAnalogChannels = 7;


Galil::Galil() {
    g = nullptr;
    setPoint = 0;
    ControlParameters[0] = 0.0;   // Kp
    ControlParameters[1] = 0.0;   // Ki
    ControlParameters[2] = 0.0;   // Kd
    address = defaultAddress;

    ownFunction = true;
    useSimulator = true;
    lastCommandStatus = G_NO_ERROR;
    lastCommandResponse = {};

    Functions = new EmbeddedFunctions(true);   // ture = simulator

    GReturn status = Functions->GOpen(defaultAddress, &g);
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

    ownFunction = false;
    lastCommandStatus = G_NO_ERROR; // general error code
    lastCommandResponse = {};

    useSimulator = true;

    Functions = Funcs;  

    GReturn status = Functions->GOpen(address, &g);
    if (status != G_NO_ERROR) {
        // error code G_NO_ERROR found in gclib errors.h file
        std::cout << "Failed to open Galil connection. Error code: " << status << "\n";
    }
}

// My overload consturctor to test the copy constructor with
Galil::Galil(EmbeddedFunctions* Funcs, GCStringIn address, bool usingSimulator) {
    g = nullptr;
    setPoint = 0;
    ControlParameters[0] = 0.0;   // Kp
    ControlParameters[1] = 0.0;   // Ki
    ControlParameters[2] = 0.0;   // Kd
    this->address = address;
    
    ownFunction = false;
    lastCommandStatus = G_NO_ERROR; // general error code
    lastCommandResponse = {};

    useSimulator = usingSimulator;

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

    ownFunction = true;

    useSimulator = other.useSimulator;
    lastCommandStatus = other.lastCommandStatus;
    lastCommandResponse = other.lastCommandResponse;

    Functions = new EmbeddedFunctions(useSimulator);
    
    GReturn status = Functions->GOpen(address.c_str(), &g);
    if (status != G_NO_ERROR) {
        // error code G_NO_ERROR found in gclib errors.h file
        std::cout << "Failed to open Galil connection. Error code: " << status << "\n";
    }
}

Galil::~Galil() {
    GReturn status = StopsConnection();
    if (status != G_NO_ERROR) {
        std::cerr << "Failed to close Galil connection. Error code: " << status << "\n";
    }
}

//////////////////////////////////////////// Digital output //////////////////////////////////////////

// Write to all 16 bits of digital output, 1 command to the Galil
void Galil::DigitalOutput(uint16_t value) {
    uint8_t low = value & 0xFF;          // last 8 bits, 0-7 least significant bits
    uint8_t high = (value >> 8) & 0xFF;   // first 8 bits, 8-15

    std::string command = "OP " + std::to_string(low) + "," + std::to_string(high) + ";";

    SendCommand(command);
}

// Write to one byte, either high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
void Galil::DigitalByteOutput(bool bank, uint8_t value) {
    std::string command;

    if (bank == 0) {
        command = "OP " + std::to_string(value) + ";";
    }
    else {
        command = "OP " + std::to_string(0) + "," + std::to_string(value) + ";";
    }

    SendCommand(command);
}

// Write single bit to digital outputs. 'bit' specifies which bit
void Galil::DigitalBitOutput(bool val, uint8_t bit) {
    CheckBit(bit);
    std::string command = "OB " + std::to_string(bit) + "," + std::to_string(val ? 1 : 0) + ";";

    SendCommand(command);
}

//////////////////////////////////////////// Digital Inputs /////////////////////////////////////////

// Return the 16 bits of input data
uint16_t Galil::DigitalInput() {
    uint16_t lowBankValue = DigitalByteInput(0);
    uint16_t highBankValue = DigitalByteInput(1);

    return lowBankValue | highBankValue << 8;
}

// Read entire high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
uint8_t Galil::DigitalByteInput(bool bank) {
    std::string command = "TI " + std::to_string(bank) + ";";

    std::string response = SendCommand(command);

    return static_cast<uint8_t>(atoi(response.c_str()));
}

// read one bit of input
bool Galil::DigitalBitInput(uint8_t bit) {
    CheckBit(bit);
    std::string command = "MG @IN[" + std::to_string(bit) + "];";

    std::string response = SendCommand(command);

    return atof(response.c_str()) != 0;
}

////////////////////////////// Check Successful Write ///////////////////////////////////////////////////////////////////////

bool Galil::CheckSuccessfulWrite() {
    if (lastCommandStatus != G_NO_ERROR) return false;

    // if response contains a '?' thers an error
    if (lastCommandResponse.find('?') != std::string::npos) return false;

    // if the last non-whitespace character is a colon its a successful write
    const size_t lastCharPos = lastCommandResponse.find_last_not_of(" \r\n");
    if (lastCharPos == std::string::npos || lastCommandResponse[lastCharPos] != ':') return false;

    return true;
}

//////////////////////////////////////// Analogue Input and Output ///////////////////////////////////

// Read Analog channel and return voltage
float Galil::AnalogInput(uint8_t channel) {
    CheckAnoChannel(channel);
    std::string command = "MG @AN[" + std::to_string(channel) + "]";

    std::string response = SendCommand(command);

    return atof(response.c_str());
}

// Write to any channel of the Galil, send voltages as
// 2 decimal place in the command string
void Galil::AnalogOutput(uint8_t channel, double voltage) {
    CheckAnoChannel(channel);

    std::stringstream command;
    command << "AO " << (int)channel << "," << std::fixed << std::setprecision(2) << voltage << ";";

    SendCommand(command.str());
}

// Configure the range of the input channel with
// the desired range code
// From AQ documentation and forum #54:
// Range value: 1 = +- 5V, 2 = +- 10V, 3 = 0-5V, 4 = 0-10V
void Galil::AnalogInputRange(uint8_t channel, uint8_t range) {
    CheckAnoChannel(channel);

    if (range < 1 || range > 4) {
        throw std::runtime_error("Analog input range out of range, range code must be 1...4.");
    }

    std::string command = "AQ " + std::to_string(channel) + "," + std::to_string(range) + ";";
    SendCommand(command);
}

///////////////////////////////////////////////// Encoder //////////////////////////////////////////
// Manually Set the motor encoder value to zero (encoder channel 0)
void Galil::WriteEncoder() {
    // keep whatever value at channel 1 still same if we ever have another encoder channel
    SendCommand("WE 0;");
}

// Read from motor Encoder (encoder channel 0)
int Galil::ReadEncoder() {
    const std::string command = "QE 0;";
    std::string response = SendCommand(command);

    return static_cast<int>(atoi(response.c_str()));
}

///////////////////////////////////////////////// Control Functions //////////////////////////////

// Set the desired setpoint for control loops, counts or counts/sec
void Galil::setSetPoint(int s) {
    setPoint = s;
}

// Gets the current setpoint stored in the class
double Galil::getSetPoint() {
    return static_cast<double>(setPoint);
}

// Set the proportional gain of the controller used in controlLoop() of Position/SpeedControl
void Galil::setKp(double gain) {
    ControlParameters[0] = gain;
}

// Gets the current proportional gain stored in the class
double Galil::getKp() {
    return ControlParameters[0];
}

// Set the integral gain of the controller used in controlLoop()  of Position/SpeedControl
void Galil::setKi(double gain) {
    ControlParameters[1] = gain;
}

// Gets the current integral gain stored in the class
double Galil::getKi() {
    return ControlParameters[1];
}

// Set the derivative gain of the controller used in controlLoop()  of Position/SpeedControl
void Galil::setKd(double gain) {
    ControlParameters[2] = gain;
}

// Gets the current derivative gain stored in the class
double Galil::getKd() {
    return ControlParameters[2];
}

////////////////////////////////////////////// Operator Overload ///////////////////////////////
std::ostream& operator<<(std::ostream& output, Galil& galil) {
    char info[G_SMALL_BUFFER] = "";
    char version[G_SMALL_BUFFER] = "";

    GReturn infoStatus = G_GCLIB_ERROR;
    GReturn versionStatus = G_GCLIB_ERROR;

    if (galil.Functions != nullptr) {
        if (galil.g != nullptr) {
            infoStatus = galil.Functions->GInfo(galil.g, info, sizeof(info));
        }
        versionStatus = galil.Functions->GVersion(version, sizeof(version));
    }

    if (infoStatus == G_NO_ERROR) {
        output << info << "\n\n";
    } else {
        output << "Galil info error: " << infoStatus << ")\n\n";
    }

    if (versionStatus == G_NO_ERROR) {
        output << version << "\n\n";
    } else {
        output << "Galil version error: " << versionStatus << ")\n\n";
    }

    return output;
}

Galil& Galil::operator=(const Galil& other) {
    if (this == &other) {
        return *this;   // self-assignment
    }

    Galil temp(other);   // copy constructor opens a new connection

    GReturn status = StopsConnection();
    if (status != G_NO_ERROR) {
        std::cerr << "Failed to close Galil connection. Error code: " << status << "\n";
    }

    std::swap(Functions, temp.Functions);
    std::swap(g, temp.g);
    std::swap(ControlParameters, temp.ControlParameters);
    std::swap(setPoint, temp.setPoint);
    std::swap(address, temp.address);
    std::swap(ownFunction, temp.ownFunction);
    std::swap(useSimulator, temp.useSimulator);
    std::swap(lastCommandStatus, temp.lastCommandStatus);
    std::swap(lastCommandResponse, temp.lastCommandResponse);

    return *this;
    // temp is automatically destroyed so old connection also closed
}

////////////////////////////////////////////////////////////////
// My own helper functions
///////////////////////////////////

std::string Galil::SendCommand(const std::string& command) {
    if (g == nullptr) {
        throw std::runtime_error("Galil is not connected.");
    }

    // buffer size defined from gclib.h
    char buffer[G_SMALL_BUFFER] = {};
    // needs c_str for command as it a standard c string that needs a null-terminated string, char*
    GReturn status = Functions->GCommand(g, command.c_str(), buffer, sizeof(buffer), nullptr);

    lastCommandStatus = status;
    lastCommandResponse = std::string(buffer);

    if (status != G_NO_ERROR) {
        std::cerr << "Failed to send command to Galil: " << command << " Error code: " << status << "\n";
    }

    return lastCommandResponse;
}

// Throws std::runtime_error if bit is not a valid digital I/O bit (0-15)
// from forum #75 that throwing a runtime error is fine instead of std::cerr
void Galil::CheckBit(uint8_t bit) {
    if (bit > maxDigitalBit) {
        throw std::runtime_error("Digital bit must be between 0 and 15.");
    }
}

void Galil::CheckAnoChannel(uint8_t channel) {
    if (channel > maxNumAnalogChannels) {
        throw std::runtime_error("Anologu channel must be between 0 and 7.");
    }
}

GReturn Galil::StopsConnection() {
    GReturn status = G_NO_ERROR;

    // Only close if there is actually an open connection
    if (Functions != nullptr && g != nullptr) {
        status = Functions->GClose(g);
        if (status != G_NO_ERROR) {
            std::cerr << "Failed to close Galil connection. Error code: " << status << "\n";
        }
        else {
            std::cout << "Closed\n";
        }
    }
    g = nullptr;   // mark as disconnected

    if (ownFunction && Functions != nullptr) {
        delete Functions;
    }

    Functions = nullptr;
    return status;
}