#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PSIM_PORT "5678"

//return values
#define PSIM_SUCCESS				0
#define PSIM_NO_CONNECTION			1
#define PSIM_COMMUNICATION_ERROR	2

//PCSC named values
#define PCSC_SUCCESS				0
#define PCSC_ICC_PRESENT			0x0267
#define PCSC_SCARD_CONTROL			0x00313520


//frequently used parts of PSIM connector messages
#define PSIM_MSG_DIVIDER "|"
#define PSIM_MSG_EMPTY_PARAMS ""

//PCSC function identifiers used in connector messages

#define PSIM_MSG_HANDSHAKE_ICC_HELLO "00000001"
#define PSIM_MSG_HANDSHAKE_ICC_STOP  "00000002"
#define PSIM_MSG_HANDSHAKE_ICC_ERROR "00000003"
#define PSIM_MSG_HANDSHAKE_ICC_DONE  "00000004"
#define PSIM_MSG_HANDSHAKE_IFD_HELLO "00000005"
#define PSIM_MSG_HANDSHAKE_IFD_ERROR "00000006"


#define PSIM_MSG_FUNCTION_DEVICE_CONTROL          "10"
#define PSIM_MSG_FUNCTION_DEVICE_LIST_DEVICES     "11"
#define PSIM_MSG_FUNCTION_GET_CAPABILITIES        "12"
#define PSIM_MSG_FUNCTION_SET_CAPABILITIES        "13"
#define PSIM_MSG_FUNCTION_POWER_ICC               "14"
#define PSIM_MSG_FUNCTION_TRANSMIT_TO_ICC         "15"
#define PSIM_MSG_FUNCTION_IS_ICC_PRESENT          "16"
#define PSIM_MSG_FUNCTION_IS_ICC_ABSENT           "17"
#define PSIM_MSG_FUNCTION_SWALLOW_ICC             "18"
#define PSIM_MSG_FUNCTION_SET_PROTOCOL_PARAMETERS "19"
#define PSIM_MSG_FUNCTION_LIST_INTERFACES         "1A"
#define PSIM_MSG_FUNCTION_LIST_CONTEXTS           "1B"
#define PSIM_MSG_FUNCTION_IS_CONTEXT_SUPPORTED    "1C"
#define PSIM_MSG_FUNCTION_GET_IFDSP               "1D"
#define PSIM_MSG_FUNCTION_EJECT_ICC               "1E"

#define PSIM_POWER_UP		0x1F4
#define PSIM_POWER_DOWN		0x1F5
#define PSIM_POWER_RESET	0x1F6




/*
Start and fork of the handshake server, listening on the provided port number.

Parameters:
	port - the port to be used for the server

Returns:
	either PSIM_SUCCESS or PSIM_COMMUNICATION_ERROR
*/
int PSIMStartHandshakeServer(PCSTR port);

/*
Stops the server and cleans up used handles
*/
void PSIMStopHandshakeServer();

/*
Returns:
	true if the socket connection is availiable
*/
bool isHandshakePerformed();

/*
Transmit data to the simulator

Parameters:
	Lun - the lun of the slot to use
	TxBuffer - a pointer to the string to send via the socket
	TxLength - the input data length
	RxBuffer - a pointer to the receiving buffer
	RxLength - the length of the receiving buffer

Returns:
	the PCSC response code as integer or 1 in case of transmission error
*/
int PSIMTransmitICC(DWORD Lun, const char *TxBuffer, DWORD TxLength, char *RxBuffer, PDWORD RxLength);

/*
Checks if the simulator is reachable

Returns:
	the PCSC response code as integer
*/
int PSIMICCPresent();

/*
Executes a power operation on the simulator

Parameters:
	action - the PCSC power action
	ATR - a pointer to the receiving buffer to store the ATR
	ATRsize - a pointer the size of the ATR

Returns:
	the PCSC response code as integer or PSIM_COMMUNICATION_ERROR if the
	socket communication fails
*/
int PSIMPowerICC(int action, char* ATR, int * ATRsize);

/*
Transmit a string through a socket

Parameters:
	sock - the socket to use
	msgBuffer - the data to send

Returns:
	PSIM_SUCCESS, PSIM_NO_CONNECTION or PSIM_COMMUNICATION_ERROR
*/
int transmit(SOCKET sock, char * msgBuffer);

/*
Receive data from a socket

Parameters:
	sock - the socket ot use
	response - a pointer to the receiving buffer
	respLength - the length of the expected response

Returns:
	PSIM_SUCCESS, PSIM_NO_CONNECTION or PSIM_COMMUNICATION_ERROR
*/
int receive(SOCKET sock, char * response, int respLength);

/*
Execute a PCSC function on the simulator and receive a hexadecimal string

Parameters:
	function - PCSC function to send
	lun - the lun of the simulator to use
	params - parameters for the PCSC call
	response - a pointer to the receiving buffer
	respLength - the expected length of the response

Returns:
	PSIM_SUCCESS or PSIM_COMMUNICATION_ERROR
*/
int exchangePcscFunction(const char* function, DWORD lun, const char* params, char * response, int respLength);

/*
Extracts the response code from the first 8 characters

Parameters:
	response - the string to use

Returns:
	the response code value
*/
int extractPcscResponseCode(const char* response);
