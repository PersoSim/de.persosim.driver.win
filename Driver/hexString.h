#pragma once

/*
Converts a hexadecimal character into an integer value

Parameters:
	input - the char to convert

Returns:
	the value 0-15 for any of [0123456789ABCDEFabcdef] or 0 if the input is invalid
*/
extern int HexChar2Int(char input);

/*
Writes a hexadecimal nibble into the first element of a char string

Parameters:
	nibble - the hexadecimal nibble to write
	hex - a pointer to the string to write into

Returns:
	1 in case of success, -1 else
*/
extern int HexNibble2Char(char nibble, char* hex);

/*
Writes a hexadecimal byte into a string

Parameters:
	input - the byte to write
	chars - the destination string

Returns:
	the number of chars written
*/
extern int HexByte2Chars(char byte, char* chars);

/*
Converts a hexadecimal byte into a hexadecimal string

Parameters:
	byte - the byte to convert
	chars - a pointer to the destination string

Returns:
	the length of the string without the \0 termination byte
*/
extern int HexByte2String(char byte , char* chars);


/*
Converts an unsigned integer into a hexadecimal string

Parameters:
	input - the integer to convert
	hexString - a pointer to the destination string

Returns:
	the length of the string without the \0 termination byte
*/
extern int HexInt2String(unsigned int input, char* hexString);

/*
Converts a byte array into a hexadecimal string

Parameters:
	bytes - a pointer to the array to be converted
	bytesLength - the length of the input data
	hexString - a pointer to the destination string

Returns:
	the length of the string without the \0 termination byte
*/
extern int HexByteArray2String(const char* bytes, int bytesLength, char* hexString);

/*
Converts a hexadecimal, \0 terminated string into an byte array

Parameters:
	hexString - a pointer to the string to be converted
	bytes - a pointer to the destination array

Returns:
	the length of the bytes array
*/
extern int HexString2CharArray(const char* hexString, char* bytes);

/*
Converts a byte array into a hexadecimal string

Parameters:
	hexString - a pointer to the string to be converted

Returns:
	the integer value
*/
extern int HexString2Int(const char* hexString);
