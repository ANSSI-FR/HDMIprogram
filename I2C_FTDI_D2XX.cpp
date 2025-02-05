#include "stdafx.h"
#include "I2C_FTDI_D2XX.h"

//MPSSE Control Commands
#define MPSSE_CMD_SET_DATA_BITS_LOWBYTE				0x80
#define MPSSE_CMD_SET_DATA_BITS_HIGHBYTE			0x82
#define MPSSE_CMD_GET_DATA_BITS_LOWBYTE				0x81
#define MPSSE_CMD_GET_DATA_BITS_HIGHBYTE			0x83

#define MPSSE_CMD_SEND_IMMEDIATE					0x87
#define MPSSE_CMD_ENABLE_3PHASE_CLOCKING			0x8C
#define MPSSE_CMD_DISABLE_3PHASE_CLOCKING			0x8D
#define MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO			0x9E

//MPSSE Data Command - LSB First
#define MPSSE_CMD_DATA_LSB_FIRST					0x08

//MPSSE Data Commands - bit mode - MSB first
#define MPSSE_CMD_DATA_OUT_BITS_POS_EDGE			0x12
#define MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE			0x13
#define MPSSE_CMD_DATA_IN_BITS_POS_EDGE				0x22
#define MPSSE_CMD_DATA_IN_BITS_NEG_EDGE				0x26
#define MPSSE_CMD_DATA_BITS_IN_POS_OUT_NEG_EDGE		0x33
#define MPSSE_CMD_DATA_BITS_IN_NEG_OUT_POS_EDGE		0x36


//MPSSE Data Commands - byte mode - MSB first
#define MPSSE_CMD_DATA_OUT_BYTES_POS_EDGE			0x10
#define MPSSE_CMD_DATA_OUT_BYTES_NEG_EDGE			0x11
#define MPSSE_CMD_DATA_IN_BYTES_POS_EDGE			0x20
#define MPSSE_CMD_DATA_IN_BYTES_NEG_EDGE			0x24
#define MPSSE_CMD_DATA_BYTES_IN_POS_OUT_NEG_EDGE	0x31
#define MPSSE_CMD_DATA_BYTES_IN_NEG_OUT_POS_EDGE	0x34


//SCL & SDA directions
#define DIRECTION_SCLIN_SDAIN						0x10
#define DIRECTION_SCLOUT_SDAIN						0x11
#define DIRECTION_SCLIN_SDAOUT						0x12
#define DIRECTION_SCLOUT_SDAOUT						0x13

//SCL & SDA values
#define VALUE_SCLLOW_SDALOW							0x00
#define VALUE_SCLHIGH_SDALOW						0x01
#define VALUE_SCLLOW_SDAHIGH						0x02
#define VALUE_SCLHIGH_SDAHIGH						0x03

//Data size in bits
#define DATA_SIZE_8BITS								0x07
#define DATA_SIZE_1BIT								0x00

//ACK/NACK
#define SEND_ACK									0x00
#define SEND_NACK									0x80


I2C_FTDI_D2XX::I2C_FTDI_D2XX()
{
	last_error = NULL;
	d2xx_dll_handle = 0;
	memset(&funlist, 0, sizeof(funlist));
	cableSerial[0] = 0;
	devhandle = NULL;
}

I2C_FTDI_D2XX::~I2C_FTDI_D2XX()
{

}

int I2C_FTDI_D2XX::link_dll()
{
	d2xx_dll_handle = LoadLibraryA("ftd2xx.dll");
	if (!d2xx_dll_handle) { last_error = u8"Le pilote FTDI D2XX n'est pas installé,\nou bien un câble compatible n'a jamais été branché sur cette machine."; return RET_ERROR; }

	funlist.FT_CreateDeviceInfoList = (FT_CreateDeviceInfoList_t)GetProcAddress(d2xx_dll_handle, "FT_CreateDeviceInfoList");
	if (!funlist.FT_CreateDeviceInfoList) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_GetDeviceInfoList = (FT_GetDeviceInfoList_t)GetProcAddress(d2xx_dll_handle, "FT_GetDeviceInfoList");
	if (!funlist.FT_GetDeviceInfoList) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_Open = (FT_Open_t)GetProcAddress(d2xx_dll_handle, "FT_Open");
	if (!funlist.FT_Open) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_Close = (FT_Close_t)GetProcAddress(d2xx_dll_handle, "FT_Close");
	if (!funlist.FT_Close) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_ResetDevice = (FT_ResetDevice_t)GetProcAddress(d2xx_dll_handle, "FT_ResetDevice");
	if (!funlist.FT_ResetDevice) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_Purge = (FT_Purge_t)GetProcAddress(d2xx_dll_handle, "FT_Purge");
	if (!funlist.FT_Purge) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_SetUSBParameters = (FT_SetUSBParameters_t)GetProcAddress(d2xx_dll_handle, "FT_SetUSBParameters");
	if (!funlist.FT_SetUSBParameters) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_SetChars = (FT_SetChars_t)GetProcAddress(d2xx_dll_handle, "FT_SetChars");
	if (!funlist.FT_SetChars) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_SetTimeouts = (FT_SetTimeouts_t)GetProcAddress(d2xx_dll_handle, "FT_SetTimeouts");
	if (!funlist.FT_SetTimeouts) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_SetLatencyTimer = (FT_SetLatencyTimer_t)GetProcAddress(d2xx_dll_handle, "FT_SetLatencyTimer");
	if (!funlist.FT_SetLatencyTimer) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_SetBitMode = (FT_SetBitMode_t)GetProcAddress(d2xx_dll_handle, "FT_SetBitMode");
	if (!funlist.FT_SetBitMode) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_Write = (FT_Write_t)GetProcAddress(d2xx_dll_handle, "FT_Write");
	if (!funlist.FT_Write) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_Read = (FT_Read_t)GetProcAddress(d2xx_dll_handle, "FT_Read");
	if (!funlist.FT_Read) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }
	funlist.FT_GetQueueStatus = (FT_GetQueueStatus_t)GetProcAddress(d2xx_dll_handle, "FT_GetQueueStatus");
	if (!funlist.FT_GetQueueStatus) { last_error = u8"Le pilote FTDI D2XX n'est pas compatible."; return RET_ERROR; }


	return RET_OK;
}

void I2C_FTDI_D2XX::unlink_dll()
{
	memset(&funlist, 0, sizeof(funlist));
	FreeLibrary(d2xx_dll_handle);
	d2xx_dll_handle = NULL;
}

int I2C_FTDI_D2XX::empty_input_buffer()
{
	unsigned char buffer[4096];
	DWORD toread = 0;
	if (funlist.FT_GetQueueStatus(devhandle, &toread) != FT_OK) return RET_ERROR;
	while (toread>0)
	{
		DWORD returned = 0;
		DWORD blocksize = toread;
		if (blocksize > 4096) blocksize = 4096;
		if (funlist.FT_Read(devhandle, buffer, blocksize, &returned) != FT_OK) return RET_ERROR;
		toread -= returned;
	}
	return RET_OK;
}

int I2C_FTDI_D2XX::loopback_test(bool continuous, unsigned char cmd)
{
	unsigned char buffer[4096];
	DWORD written = 0;
	bool echoed = false;
	int loopcounter = 0;
	DWORD toread = 0;
	DWORD returned = 0;
	bool last_was_FA = false;

	if (!continuous)
	{
		if (funlist.FT_Write(devhandle, &cmd, 1, &written) != FT_OK) return RET_ERROR;
	}
	do
	{
		if (continuous)
		{
			if (funlist.FT_Write(devhandle, &cmd, 1, &written) != FT_OK) return RET_ERROR;
		}
		if (funlist.FT_GetQueueStatus(devhandle, &toread) != FT_OK) return RET_ERROR;
		Sleep(1);
		if (toread>0)
		{
			if (toread > 4096) return RET_ERROR;
			if (funlist.FT_Read(devhandle, buffer, toread, &returned) != FT_OK) return RET_ERROR;
			if (returned > 0)
			{
				for (unsigned int i = 0; i < toread; i++)
				{
					if (buffer[i] == 0xFA)
					{
						last_was_FA = true;
					}
					else
					{
						if (last_was_FA && buffer[i] == cmd)
						{
							echoed = true;
							break;
						}
						last_was_FA = false;
					}
				}
			}
		}
		loopcounter++;
		if (loopcounter>4096) return RET_ERROR;
	} while (!echoed);
	return RET_OK;
}

int I2C_FTDI_D2XX::open()
{
	if (link_dll()) return RET_ERROR;
	//enumerate devices
	DWORD num_devices;
	if (funlist.FT_CreateDeviceInfoList(&num_devices) != FT_OK) { last_error = u8"Erreur d'énumération de câble."; return RET_ERROR; }
	if (num_devices==0) { last_error = u8"Aucun câble compatible détecté."; return RET_ERROR; }
	//get devices info
	FT_DEVICE_LIST_INFO_NODE* devlist = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * num_devices);
	if (funlist.FT_GetDeviceInfoList(devlist, &num_devices) != FT_OK) { last_error = u8"Erreur d'énumération de câble."; free(devlist); return RET_ERROR; }
	//choose the device
	int selected = -1;
	for (unsigned int i = 0; i < num_devices; i++)
	{
		if (devlist[i].Type == FT_DEVICE_232H && !(devlist[i].Flags & FT_FLAGS_OPENED))
		{
			//we take this one
			selected = i;
			strncpy(cableSerial, devlist[i].SerialNumber, 16);
			cableSerial[16] = 0;
			break;
		}
	}
	free(devlist);
	if (selected  < 0) { last_error = u8"Aucun câble compatible détecté."; return RET_ERROR; }
	//open the chosen device
	if (funlist.FT_Open(selected, &devhandle) != FT_OK) { last_error = u8"Erreur à l'ouverture du périphérique."; return RET_ERROR; }
	//initialization sequence
	if (funlist.FT_ResetDevice(devhandle) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (funlist.FT_Purge(devhandle, 3) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (funlist.FT_SetUSBParameters(devhandle, 65536, 65536) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (funlist.FT_SetChars(devhandle, 0, 0, 0, 0) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (funlist.FT_SetTimeouts(devhandle, 5000, 5000) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (funlist.FT_SetLatencyTimer(devhandle, 255) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//reset MPSSE
	if (funlist.FT_SetBitMode(devhandle, 0, 0) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//enable MPSSE
	if (funlist.FT_SetBitMode(devhandle, 0, 2) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//enable loopback
	unsigned char cmd[10];
	cmd[0] = 0x84;
	DWORD written = 0;
	if (funlist.FT_Write(devhandle, cmd, 1, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//Sync MPSSE
	if (empty_input_buffer() != RET_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (loopback_test(true, 0xAA) != RET_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (loopback_test(false, 0xAB) != RET_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	Sleep(50); //wait for USB
	//set clock frequency
	//enable clock divide
	cmd[0] = 0x8B;
	if (funlist.FT_Write(devhandle, cmd, 1, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//clock set
	cmd[0] = 0x86;
	cmd[1] = 39; //this is good for a 100kHz 3 phase clocking mode
	cmd[2] = 0;
	if (funlist.FT_Write(devhandle, cmd, 3, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	Sleep(20);
	//stop loopback
	cmd[0] = 0x85;
	if (funlist.FT_Write(devhandle, cmd, 1, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	if (empty_input_buffer() != RET_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//set GPIO pin states
	cmd[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	cmd[1] = 0x13;
	cmd[2] = 0x13;
	if (funlist.FT_Write(devhandle, cmd, 3, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };
	//enable 3 phase clocking
	cmd[0] = MPSSE_CMD_ENABLE_3PHASE_CLOCKING;
	if (funlist.FT_Write(devhandle, cmd, 1, &written) != FT_OK) { close(); last_error = u8"Erreur à l'initialisation du périphérique."; return RET_ERROR; };

	///We are done !!!

	last_error = NULL;
	return RET_OK;
}

void I2C_FTDI_D2XX::close()
{
	if (devhandle)
	{
		funlist.FT_Close(devhandle);
		devhandle = NULL;
	}
	cableSerial[0] = 0;
	if (d2xx_dll_handle) unlink_dll();
}

int I2C_FTDI_D2XX::I2C_start()
{
	unsigned char buff[31 * 3];
	int pos = 0;
	for (int i = 0; i < 10; i++)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++; 
		buff[pos] = VALUE_SCLHIGH_SDAHIGH; pos++; 
		buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++; //let the line pulled up
	}
	for (int i = 0; i < 20; i++)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++; 
		buff[pos] = VALUE_SCLHIGH_SDALOW; pos++; 
		buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++; 
	}
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++; 
	buff[pos] = VALUE_SCLLOW_SDALOW; pos++; 
	buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++;
	DWORD written = 0;
	if (funlist.FT_Write(devhandle, buff, pos, &written) != FT_OK) return RET_ERROR;
	if (written!=pos) return RET_ERROR;
	return RET_OK;
}

int I2C_FTDI_D2XX::I2C_stop()
{
	unsigned char buff[31 * 3];
	int pos = 0;
	for (int i = 0; i < 10; i++)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
		buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
		buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++;
	}
	for (int i = 0; i < 10; i++)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
		buff[pos] = VALUE_SCLHIGH_SDALOW; pos++;
		buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++;
	}
	for (int i = 0; i < 10; i++)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
		buff[pos] = VALUE_SCLHIGH_SDAHIGH; pos++;
		buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++; //let the line pulled up
	}
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
	buff[pos] = VALUE_SCLHIGH_SDAHIGH; pos++;
	buff[pos] = DIRECTION_SCLIN_SDAIN; pos++; //tristate SCL & SDA
	DWORD written = 0;
	if (funlist.FT_Write(devhandle, buff, pos, &written) != FT_OK) return RET_ERROR;
	if (written != pos) return RET_ERROR;
	return RET_OK;
}

int I2C_FTDI_D2XX::I2C_writeByteGetAck(unsigned char b, bool* nack)
{
	unsigned char buff[12];
	int pos = 0;
	// set direction
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
	buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
	buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++;
	//write 8 bits
	buff[pos] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE; pos++;
	buff[pos] = DATA_SIZE_8BITS; pos++;
	buff[pos] = b; pos++;
	//SDA to input before reading ACK
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
	buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
	buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++;
	//get ACK
	buff[pos] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE; pos++;
	buff[pos] = DATA_SIZE_1BIT; pos++;
	//do it immediately
	buff[pos] = MPSSE_CMD_SEND_IMMEDIATE; pos++;
	DWORD written = 0;
	if (funlist.FT_Write(devhandle, buff, pos, &written) != FT_OK) return RET_ERROR;
	if (written != pos) return RET_ERROR;
	DWORD toread = 1;
	DWORD returned = 0;
	Sleep(1);
	if (funlist.FT_Read(devhandle, buff, toread, &returned) != FT_OK) return RET_ERROR;
	if (toread != returned) return RET_ERROR;
	*nack = (buff[0] & 1) == 1;
	return RET_OK;
}

int I2C_FTDI_D2XX::I2C_readByteGiveAck(unsigned char *b, bool ack)
{
	unsigned char buff[15];
	int pos = 0;
	// set direction
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
	buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
	buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++;
	//read 8 bits
	buff[pos] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE; pos++;
	buff[pos] = DATA_SIZE_8BITS; pos++;
	if (ack)
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
		buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
		buff[pos] = DIRECTION_SCLOUT_SDAOUT; pos++;

		buff[pos] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE; pos++;
		buff[pos] = DATA_SIZE_1BIT; pos++;
		buff[pos] = SEND_ACK; pos++;
	}
	else
	{
		buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
		buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
		buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++;

		buff[pos] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE; pos++;
		buff[pos] = DATA_SIZE_1BIT; pos++;
		buff[pos] = SEND_NACK; pos++;
	}
	//back to idle
	buff[pos] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE; pos++;
	buff[pos] = VALUE_SCLLOW_SDALOW; pos++;
	buff[pos] = DIRECTION_SCLOUT_SDAIN; pos++;
	//do it immediately
	buff[pos] = MPSSE_CMD_SEND_IMMEDIATE; pos++;

	DWORD written = 0;
	if (funlist.FT_Write(devhandle, buff, pos, &written) != FT_OK) return RET_ERROR;
	if (written != pos) return RET_ERROR;

	DWORD toread = 1;
	DWORD returned = 0;
	Sleep(1);
	if (funlist.FT_Read(devhandle, buff, toread, &returned) != FT_OK) return RET_ERROR;
	if (toread != returned) return RET_ERROR;
	*b = buff[0];
	return RET_OK;

}


int I2C_FTDI_D2XX::I2C_writeDeviceAddress(unsigned char address, bool is_read, bool* ack)
{
	unsigned char ta = address << 1;
	if (is_read) ta = ta | 1;
	return I2C_writeByteGetAck(ta, ack);
}


int I2C_FTDI_D2XX::write(unsigned char deviceAddress, unsigned int sizeToTransfer, unsigned char* buffer, unsigned int* sizeTransferred, unsigned int options)
{
	if (!devhandle || buffer==NULL || sizeTransferred==NULL || deviceAddress>127) return RET_ERROR;
	if (funlist.FT_Purge(devhandle, 3) != FT_OK) return RET_ERROR;
	if (options & I2C_TRANSFER_OPTIONS_START_BIT)
	{
		if (I2C_start() != RET_OK) return RET_ERROR;
	}
	bool nack;
	if (I2C_writeDeviceAddress(deviceAddress, false, &nack) != RET_OK) return RET_ERROR;
	if (!nack) 
	{
		unsigned int i = 0;
		for (i = 0; i < sizeToTransfer; i++)
		{
			nack = false;
			if (I2C_writeByteGetAck(buffer[i], &nack) != RET_OK) break;
			if (nack && (options & I2C_TRANSFER_OPTIONS_BREAK_ON_NACK))
			{
				if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
				{
					if (I2C_stop() != RET_OK) return RET_ERROR;
				}
				return RET_ERROR;
			}
		}
		*sizeTransferred = i;
		if (*sizeTransferred != sizeToTransfer)
		{
			return RET_ERROR;
		}
		else
		{
			if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
			{
				if (I2C_stop() != RET_OK) return RET_ERROR;
			}
		}
	}
	else
	{
		if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
		{
			if (I2C_stop() != RET_OK) return RET_ERROR;
		}
		return RET_DEVICE_ERROR;
	}
	return RET_OK;
}

int I2C_FTDI_D2XX::read(unsigned char deviceAddress, unsigned int sizeToTransfer, unsigned char* buffer, unsigned int* sizeTransferred, unsigned int options)
{
	if (!devhandle || buffer == NULL || sizeTransferred == NULL || deviceAddress > 127) return RET_ERROR;
	if (funlist.FT_Purge(devhandle, 3) != FT_OK) return RET_ERROR;
	if (options & I2C_TRANSFER_OPTIONS_START_BIT)
	{
		if (I2C_start() != RET_OK) return RET_ERROR;
	}
	bool nack;
	if (I2C_writeDeviceAddress(deviceAddress, true, &nack) != RET_OK) return RET_ERROR;
	if (!nack)
	{
		unsigned int i = 0;
		for (i = 0; i < sizeToTransfer; i++)
		{
			if (I2C_readByteGiveAck(&(buffer[i]), (i < (sizeToTransfer - 1)) ? true : ((options & I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE) ? false : true)) != RET_OK) break;
		}
		*sizeTransferred = i;
		if (*sizeTransferred != sizeToTransfer)
		{
			return RET_ERROR;
		}
		else
		{
			if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
			{
				if (I2C_stop() != RET_OK) return RET_ERROR;
			}
		}
	}
	else
	{
		if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
		{
			if (I2C_stop() != RET_OK) return RET_ERROR;
		}
		return RET_DEVICE_ERROR;
	}
	return RET_OK;

}