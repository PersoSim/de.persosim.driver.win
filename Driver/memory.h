#pragma once
#include "internal.h"


/*
This method extracts a buffer of a given length from an I/O request

Parameters:
	pRequest - pointer to the request to extract data from
	buffer - pointer to the byte buffer to be used as a target
	bufferLen - length of data to extract

Returns:
	true if the operation was successfull, false otherwise
*/
bool getBuffer(IWDFIoRequest* pRequest,BYTE *buffer,int *bufferLen);

/*
This method sets a given string on an I/O request object

Parameters:
	pRequest - pointer to the request to copy the string into
	result - pointer to the string to be used as source
	outSize - the maximum size to copy
*/
void setString(IWDFIoRequest* pRequest, char *result, int outSize);

/*
This method sets a given buffer on an I/O request object

Parameters:
	pRequest - pointer to the request to copy the data into
	buffer - pointer to the byte buffer to be used as source
	inSize - length of data to copy
*/
void setBuffer(IWDFIoRequest* pRequest, BYTE *buffer, int inSize);

/*
This method sets a given DWORD on an I/O request object

Parameters:
	pRequest - pointer to the request to copy the string into
	value - the value to be set
*/
void setInt(IWDFIoRequest* pRequest, DWORD value);

/*
This method extracts an integer from an I/O request

Parameters:
	pRequest - pointer to the request to extract data from

Returns:
	the value or 0xffffffff if the operation failed
*/
DWORD getInt(IWDFIoRequest* pRequest);
