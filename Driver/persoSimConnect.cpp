#include "persoSimConnect.h"
#include "hexString.h"

#include <vector>

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;
HANDLE ghMutex = NULL;

WSADATA wsaData;

bool acceptHandshakeConnections = TRUE;


void PSIMStopHandshakeServer(){
	acceptHandshakeConnections = FALSE;
	//TODO handle occuring errors
	CloseHandle(ghMutex);

	closesocket(ListenSocket);
	WSACleanup();
}

bool isHandshakePerformed(){
	return ClientSocket != INVALID_SOCKET;
}

DWORD handleHandshakeConnections() {
	
	while (acceptHandshakeConnections) {
		// Accept a client socket
		SOCKET tmpSocket = accept(ListenSocket, NULL, NULL);
		if (tmpSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			OutputDebugString(L"The socket could not be created");
			return 1;
		}

		//prepare response buffer
		char response[19];

		//TODO implement handshake protocol changes here

		receive(tmpSocket, response, 19);
		char command[] = "05|00000000";
		transmit(tmpSocket, command);
		receive(tmpSocket, response, 19);

		ClientSocket = tmpSocket;
	}
	return 0;
}

int PSIMStartHandshakeServer(PCSTR port){

	int iResult;

	//init transmit mutex
	ghMutex = CreateMutex(NULL, FALSE, NULL);

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}



	// Create a SOCKET for the server to listen for client connections

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	ClientSocket = INVALID_SOCKET;

	DWORD handshakeThreadID;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleHandshakeConnections, NULL, 0, &handshakeThreadID);

	wchar_t log[300];
	swprintf(log, L"Handshake thread started :0x%X\n", handshakeThreadID);
	OutputDebugString(log);
	return 0;
}

int receive(SOCKET sock, char* response, int respLength) {
	//if no socket connection is available return immediately
	if (sock == INVALID_SOCKET) {
		return PSIM_NO_CONNECTION;
	}

	int offset = 0;
	do {
		offset += recv(sock, response + offset, respLength, 0);
		// if len == 0 after the first loop the connection was closed
	} while (offset > 0 && offset < respLength && response[offset - 1] != '\n');

	if (response[offset - 2] == '\r') {
		offset--;
	}
	if (offset > 0) {
		response[offset - 1] = '\0';
	}
	else {
		printf("Failure during receive (no response received)");
		return PSIM_COMMUNICATION_ERROR;
	}
	printf("Received : %s\n", response);

	return PSIM_SUCCESS;
}



int transmit(SOCKET sock, char* msgBuffer) {
	//if no socket connection is available return immediately
	if (sock == INVALID_SOCKET) {
		return PSIM_NO_CONNECTION;
	}
	int messageLength = (int)strlen(msgBuffer) + 1 + 2;
	char * transmitBuffer = new char[messageLength];


	strcpy(transmitBuffer, msgBuffer);
	strcat(transmitBuffer, "\r\n");

	int iLength, iSendResult;

	iLength = (int) strlen(transmitBuffer);
	iSendResult = send(sock, transmitBuffer, iLength, 0);

	delete[] transmitBuffer;
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		return PSIM_COMMUNICATION_ERROR;
	}

	printf("Bytes sent: %d\n", iSendResult);


	return PSIM_SUCCESS;
}



int exchangePcscFunction(const char* function, DWORD lun, const char* params, char * response, int respLength) {
	UNREFERENCED_PARAMETER(lun);
	DWORD dwWaitResult = WaitForSingleObject(ghMutex, INFINITE);
	int rv = PSIM_COMMUNICATION_ERROR;

	switch (dwWaitResult)
	{
	case WAIT_ABANDONED: //previous owner died, thus usage of the socket might be usable
	case WAIT_OBJECT_0:
		//mutex available
		__try {
			// build message buffer
			const int minLength = 12; // 2*function + divider + 8*lun + \0
			const int paramLength = (int) strlen(params);
			const int bufferLength = (paramLength) ? minLength + 1 + paramLength : minLength;
			char * msgBuffer = new char[strlen(function) + 1 + strlen(PSIM_MSG_DIVIDER) * 2 + 8 + strlen(params)];
			strcpy(msgBuffer, function);
			strcat(msgBuffer, PSIM_MSG_DIVIDER);

			//ToDo: Use correct LUN here!
			strcat(msgBuffer, "00000000");

			msgBuffer[bufferLength - 1] = '\0';
			if (bufferLength > minLength) {
				strcat(msgBuffer, PSIM_MSG_DIVIDER);
				strcat(msgBuffer, params);
			}

			rv = transmit(ClientSocket, msgBuffer);
			delete[] msgBuffer;

			if (rv != PSIM_SUCCESS) {
				printf("Could not transmit PCSC function to PersoSim Connector");
				//closeReaderConnection(lun);
				break;
			}

			rv = receive(ClientSocket, response, respLength);
			if (rv != PSIM_SUCCESS) {
				printf("Could not receive PCSC response from PersoSim Connector");
				//closeReaderConnection(lun);
				break;
			}

			rv = PSIM_SUCCESS;
		}

		__finally {
			// release mutex
			if (!ReleaseMutex(ghMutex))
			{
				OutputDebugString(L"The mutex governing the socket use was released to often, this should not happen");
			}
		}
		break;
	default:
		return PSIM_COMMUNICATION_ERROR;
	}
	return rv;
}


int extractPcscResponseCode(const char* response) {
	char responseCodeString[9];
	strncpy_s(responseCodeString, response, 8);
	responseCodeString[8] = '\0';

	return HexString2Int(responseCodeString);
}

int PSIMTransmitICC(DWORD Lun, const char *TxBuffer, DWORD TxLength, char *RxBuffer, PDWORD RxLength){

	//prepare params buffer
	char * params = new char[TxLength * 2 + 1 + strlen(PSIM_MSG_DIVIDER) + 8];

	HexByteArray2String(TxBuffer, TxLength, params);
	strcat(params, PSIM_MSG_DIVIDER);
	HexInt2String(*RxLength, &params[TxLength * 2 + 1]);

	//prepare response buffer
	long respBufferSize = *RxLength * 2 + 10;
	char* response = new char[respBufferSize];


	//forward pcsc function to client
	int rv = exchangePcscFunction(PSIM_MSG_FUNCTION_TRANSMIT_TO_ICC, Lun, params, response, respBufferSize);
	delete[] params;
	if (rv != PSIM_SUCCESS) {
		return 1;
	}

	//extract response
	if (strlen(response) > 9) {
		*RxLength = HexString2CharArray(&response[9], RxBuffer);
	}
	else {
		*RxLength = 0;
	}

	//return the response code
	int responseCode = extractPcscResponseCode(response);
	delete[] response;
	return responseCode;

}


int PSIMICCPresent(){

	// Perform IS_ICC_PRESENT:
	DWORD Lun = 0;
	//prepare response buffer
	const char respBufferSizePre = 20;
	char responsePre[respBufferSizePre];

	int rv = exchangePcscFunction(PSIM_MSG_FUNCTION_IS_ICC_PRESENT, Lun, PSIM_MSG_EMPTY_PARAMS, responsePre, respBufferSizePre);
	if (rv != PSIM_SUCCESS) {
		return PSIM_COMMUNICATION_ERROR;
	}
	return extractPcscResponseCode(responsePre);
}


int PSIMPowerICC(int action, char* ATR, int * ATRsize){

	DWORD Lun = 0;

	//prepare params buffer
	char params[18]; // 8(Action) + divider + 8(AtrLength) + '\0'
	HexInt2String(action, params);
	strcat_s(params, PSIM_MSG_DIVIDER);
	HexInt2String(*ATRsize, &params[9]);

	//prepare response buffer
	char respBufferSizeATR = 74;
	char responseATR[74];


	//forward pcsc function to client
	int rv2 = exchangePcscFunction(PSIM_MSG_FUNCTION_POWER_ICC, Lun, params, responseATR, respBufferSizeATR);
	if (rv2 != PSIM_SUCCESS) {
		return PSIM_COMMUNICATION_ERROR;
	}

	//extract ATR
	if (strlen(responseATR) > 9) {
		*ATRsize = HexString2CharArray(&responseATR[9], ATR);
	}
	else {
		*ATRsize = 0;
	}
	return extractPcscResponseCode(responseATR);

}