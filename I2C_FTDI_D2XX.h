#pragma once

//all is well
#define RET_OK 0
//error with the cable
#define RET_ERROR 1
//error with the device at the end of the cable
#define RET_DEVICE_ERROR 2

//from FTDI D2XX Programme's Guide Version 1.5 (FT_000071)


#define FT_STATUS unsigned long
#define FT_OK 0
typedef PVOID FT_HANDLE;
#define FT_DEVICE_232H 8
#define FT_FLAGS_OPENED 1

#define I2C_TRANSFER_OPTIONS_START_BIT			0x1
#define I2C_TRANSFER_OPTIONS_STOP_BIT			0x2
#define I2C_TRANSFER_OPTIONS_BREAK_ON_NACK		0x4
#define I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE		0x8


typedef struct _ft_device_list_info_node 
{ 
	DWORD Flags; 
	DWORD Type; 
	DWORD ID; 
	DWORD LocId; 
	char SerialNumber[17]; 
	char Description[64]; 
	FT_HANDLE ftHandle; 
} FT_DEVICE_LIST_INFO_NODE;

typedef FT_STATUS(__stdcall* FT_CreateDeviceInfoList_t)(LPDWORD lpdwNumDevs);
typedef FT_STATUS(__stdcall* FT_GetDeviceInfoList_t)(FT_DEVICE_LIST_INFO_NODE* pDest, LPDWORD lpdwNumDevs);
typedef FT_STATUS(__stdcall* FT_Open_t)(int iDevice, FT_HANDLE* ftHandle);
typedef FT_STATUS(__stdcall* FT_Close_t)(FT_HANDLE ftHandle);
typedef FT_STATUS(__stdcall* FT_ResetDevice_t)(FT_HANDLE ftHandle);
typedef FT_STATUS(__stdcall* FT_Purge_t)(FT_HANDLE ftHandle, DWORD dwMask);
typedef FT_STATUS(__stdcall* FT_SetUSBParameters_t)(FT_HANDLE ftHandle, DWORD dwInTransferSize, DWORD dwOutTransferSize);
typedef FT_STATUS(__stdcall* FT_SetChars_t)(FT_HANDLE ftHandle, UCHAR uEventCh, UCHAR uEventChEn, UCHAR uErrorCh, UCHAR uErrorChEn);
typedef FT_STATUS(__stdcall* FT_SetTimeouts_t)(FT_HANDLE ftHandle, DWORD dwReadTimeout, DWORD dwWriteTimeout);
typedef FT_STATUS(__stdcall* FT_SetLatencyTimer_t)(FT_HANDLE ftHandle, UCHAR ucTimer);
typedef FT_STATUS(__stdcall* FT_SetBitMode_t)(FT_HANDLE ftHandle, UCHAR ucMask, UCHAR ucMode);
typedef FT_STATUS(__stdcall* FT_Write_t)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpdwBytesWritten);
typedef FT_STATUS(__stdcall* FT_Read_t)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpdwBytesReturned);
typedef FT_STATUS(__stdcall* FT_GetQueueStatus_t)(FT_HANDLE ftHandle, LPDWORD lpdwAmountInRxQueue);

typedef struct
{
	FT_CreateDeviceInfoList_t FT_CreateDeviceInfoList;
	FT_GetDeviceInfoList_t FT_GetDeviceInfoList;
	FT_Open_t FT_Open;
	FT_Close_t FT_Close;
	FT_ResetDevice_t FT_ResetDevice;
	FT_Purge_t FT_Purge;
	FT_SetUSBParameters_t FT_SetUSBParameters;
	FT_SetChars_t FT_SetChars;
	FT_SetTimeouts_t FT_SetTimeouts;
	FT_SetLatencyTimer_t FT_SetLatencyTimer;
	FT_SetBitMode_t FT_SetBitMode;
	FT_Write_t FT_Write;
	FT_Read_t FT_Read;
	FT_GetQueueStatus_t FT_GetQueueStatus;
} funlist_t;

class I2C_FTDI_D2XX
{
private:
	char* last_error;
	HMODULE d2xx_dll_handle;
	funlist_t funlist;
	//serial number of the cable (ASCII)
	char cableSerial[16];
	FT_HANDLE devhandle;

	int link_dll();
	void unlink_dll();
	//empties MPSSE input buffer
	int empty_input_buffer();
	//loopback test
	int loopback_test(bool continuous, unsigned char cmd);
	//I2C functions
	int I2C_start();
	int I2C_stop();
	int I2C_writeDeviceAddress(unsigned char address, bool is_read, bool* nack);
	int I2C_writeByteGetAck(unsigned char b, bool* ack);
	int I2C_readByteGiveAck(unsigned char *b, bool ack);

public:
	I2C_FTDI_D2XX();
	~I2C_FTDI_D2XX();
	int open();
	void close();
	int write(unsigned char deviceAddress, unsigned int sizeToTransfer, unsigned char* buffer, unsigned int* sizeTransferred, unsigned int options);
	int read(unsigned char deviceAddress, unsigned int sizeToTransfer, unsigned char* buffer, unsigned int* sizeTransferred, unsigned int options);
	char* get_last_error() { return last_error; };
	char* get_cable_serial() { return cableSerial; };
};

