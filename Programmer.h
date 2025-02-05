#pragma once

class I2C_FTDI_D2XX;

#define NUMBLOCKS 16
//I/O error from the cable
#define ERROR_CABLE -1

//No I2C Device detected at the end of the cable (cable is fine)
#define ERROR_DEVICE -2

//I2C device is write protected
#define ERROR_WP -3

class Programmer
{
private:
	//current file name (UTF-8)
	nfdchar_t* currentFilename;
	//Current EDID content
	unsigned char EDID[256];
	//EDID contains relevant data
	bool validEDID;
	//cable handle
	I2C_FTDI_D2XX *i2c;
	//true if this is the first render() (hack for lazy initialization)
	bool firstStart;
	//if true, the next error is cleared
	bool silentError;
	//app is busy opening (hack for nodal popup)
	int busyOpen;
	//close counter (hack for nodal popup)
	int closeCounter;
	//cable is initialized and ready to be used
	bool cableready;
	//last error message
	char* connectError;
	//EDID is being read
	bool reading;
	//EDID line currently being read
	int readindex;
	//EDID is being written
	bool writing;
	//EDID line currently being written
	int writeindex;
	char EDIDdescription[4096];

	//Reads a block of 16 bytes of the EDID from the cable
	//returns the number of bytes read (nominally 16) or ERROR_CABLE or ERROR_DEVICE
	int readBlock(unsigned int address, unsigned char* block);

	//Writes a block of 16 bytes to the EDID and reads back to check if it has succeeded
	//returns the number of bytes written (nominally 16) or ERROR_CABLE or ERROR_DEVICE or ERROR_WP 
	int writeBlock(unsigned int address, unsigned char* block);
	//initializes the cable
	bool connectCable();
	//closes the cable handle
	void disconnectCable();
	//does some light parsing of the EDID to create a description string
	void decodeEDID();
public:
	//constructor
	Programmer();
	//destructor
	~Programmer();
	//ImGui rendering entry point
	//scale is the GUI scaling factor (>1.0 for high DPI screens)
	//shouldClose indicates the the application must be closed
	//returns true to finish the application
	bool render(float scale, bool shouldClose);
};

