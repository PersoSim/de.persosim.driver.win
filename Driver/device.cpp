#include "internal.h"
#include "VirtualSmartCardReader.h"
#include "queue.h"
#include "device.h"
#include "driver.h"
#include <stdio.h>
#include <winscard.h>
#include "memory.h"
#include <Sddl.h>
#include <thread>

#include "persoSimConnect.h"
#include "hexString.h"

#define LUN 0
#define SLEEP_TIME 500

HRESULT
VirtualReaderDevice::CreateInstance(
    __in IWDFDriver *FxDriver,
    __in IWDFDeviceInitialize * FxDeviceInit
    )


{
	inFunc
    CComObject<VirtualReaderDevice>* device = NULL;
    HRESULT hr;

    // Allocate a new instance of the device class.
	hr = CComObject<VirtualReaderDevice>::CreateInstance(&device);

    if (device==NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Initialize the instance.
	device->AddRef();
    FxDeviceInit->SetLockingConstraint(WdfDeviceLevel);

    CComPtr<IUnknown> spCallback;
    hr = device->QueryInterface(IID_IUnknown, (void**)&spCallback);

    CComPtr<IWDFDevice> spIWDFDevice;
    if (SUCCEEDED(hr))
    {
        hr = FxDriver->CreateDevice(FxDeviceInit, spCallback, &spIWDFDevice);
    }

	if (spIWDFDevice->CreateDeviceInterface(&SmartCardReaderGuid,NULL)!=0)
		OutputDebugString(L"CreateDeviceInterface Failed\n");

	SAFE_RELEASE(device);

    return hr;
}

void VirtualReaderDevice::waitForCardPresence(){
	while (!waitInsertIpr.empty()) {
		if (PSIMICCPresent() == PCSC_ICC_PRESENT) {
			while (!waitInsertIpr.empty()) { //complete all pending requests
				IWDFIoRequest* curRequest = waitInsertIpr.front();
				if (curRequest->UnmarkCancelable()==S_OK) {
					curRequest->CompleteWithInformation(STATUS_SUCCESS, 0);
				}
				waitInsertIpr.pop_front();
			}
		}
		Sleep(SLEEP_TIME);
	}
}

void cardPresenceThreadFunction(void* Param){
	//TODO combine presence/absence check methods
	VirtualReaderDevice* waitIpr = (VirtualReaderDevice*)Param;
	waitIpr->waitForCardPresence();
}

void VirtualReaderDevice::IoSmartCardIsPresent(IWDFIoRequest* request, SIZE_T inBufSize, SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[IPRE]IOCTL_SMARTCARD_IS_PRESENT\n");

	if (PSIMICCPresent() == PCSC_ICC_PRESENT) {
		// there's a smart card present, so complete the request
		request->CompleteWithInformation(STATUS_SUCCESS, 0);
	} else {
		// there's no smart card present, so leave the request pending; it will be completed later
		IRequestCallbackCancel *callback;
		QueryInterface(__uuidof(IRequestCallbackCancel), (void**)&callback);
		request->MarkCancelable(callback);
		callback->Release();

		//start a thread to handle the request when the card is inserted
		bool requireNewThread = waitInsertIpr.empty();
		waitInsertIpr.push_back(request);
		if (requireNewThread) {
			DWORD handshakeThreadID;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cardPresenceThreadFunction, (void*)this, 0, &handshakeThreadID);

			wchar_t log[300];
			swprintf(log, L"IsPresent thread started :0x%X\n", handshakeThreadID);
			OutputDebugString(log);
		}

	}
}

void VirtualReaderDevice::IoSmartCardGetState(IWDFIoRequest* request,SIZE_T inBufSize,SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[GSTA]IOCTL_SMARTCARD_GET_STATE\n");
	
	//first check whether card presence has changed
	if (PSIMICCPresent() == PCSC_ICC_PRESENT)
	{
		if ((state == SCARD_UNKNOWN)||(state ==SCARD_ABSENT))
		{
			state = SCARD_PRESENT;
		}
	} else {
		state = SCARD_ABSENT;
	}

	setInt(request, state);
	return;

}

void VirtualReaderDevice::waitForCardAbsence(){
	//TODO combine presence/absence check methods
	while (!waitRemoveIpr.empty()) {
		if (PSIMICCPresent() != PCSC_ICC_PRESENT) {
			while (!waitRemoveIpr.empty()) { //complete all pending requests
				IWDFIoRequest* curRequest = waitRemoveIpr.front();
				if (curRequest->UnmarkCancelable() == S_OK) {
					curRequest->CompleteWithInformation(STATUS_SUCCESS, 0);
				}
				waitRemoveIpr.pop_front();
			}
		}
		Sleep(SLEEP_TIME);
	}
}

void cardAbsenceThreadFunction(void* Param){
	//TODO combine presence/absence check methods
	VirtualReaderDevice* waitIpr = (VirtualReaderDevice*)Param;
	waitIpr->waitForCardAbsence();
}

void VirtualReaderDevice::IoSmartCardIsAbsent(IWDFIoRequest* request,SIZE_T inBufSize,SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[IABS]IOCTL_SMARTCARD_IS_ABSENT\n");
	
	if (PSIMICCPresent() != PCSC_ICC_PRESENT)
		// there's no smart card present, so complete the request
		request->CompleteWithInformation(STATUS_SUCCESS, 0);
	else {
		// there's a smart card present, so leave the request pending; it will be completed later
		IRequestCallbackCancel *callback;
		QueryInterface(__uuidof(IRequestCallbackCancel),(void**)&callback);
		request->MarkCancelable(callback);
		callback->Release();

		//start a thread to handle the request when the card is removed
		bool requireNewThread = waitRemoveIpr.empty();
		waitRemoveIpr.push_back(request);
		if (requireNewThread) {
			DWORD handshakeThreadID;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cardAbsenceThreadFunction, (void*)this, 0, &handshakeThreadID);
			wchar_t log[300];
			swprintf(log, L"IsAbsent thread started :0x%X\n", handshakeThreadID);
			OutputDebugString(log);
		}

	}
}

void VirtualReaderDevice::IoSmartCardPower(IWDFIoRequest* request,SIZE_T inBufSize,SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[POWR]IOCTL_SMARTCARD_POWER\n");
	DWORD code=getInt(request);

	BYTE ATR[33];
	int ATRsize = 33;
	int pcscResp;
	switch (code)
	{
		case SCARD_COLD_RESET:
			OutputDebugString(L"[POWR]SCARD_COLD_RESET\n");
			PSIMPowerICC(PSIM_POWER_DOWN, (char *)ATR, &ATRsize);
			ATRsize = 33;
			pcscResp = PSIMPowerICC(PSIM_POWER_UP, (char *)ATR, &ATRsize);
			if (pcscResp != PSIM_SUCCESS)
			{
				request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
				return;
			}
			state = SCARD_SPECIFIC;
			break;
		case SCARD_WARM_RESET:
			OutputDebugString(L"[POWR]SCARD_WARM_RESET\n");
			pcscResp = PSIMPowerICC(PSIM_POWER_RESET, (char *)ATR, &ATRsize);
			if (pcscResp != PSIM_SUCCESS)
			{
				request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
				return;
			}
			state = SCARD_SPECIFIC;
			break;
		case SCARD_POWER_DOWN:
			OutputDebugString(L"[POWR]SCARD_POWER_DOWN\n");
			pcscResp = PSIMPowerICC(PSIM_POWER_DOWN, (char *)ATR, &ATRsize);
			if (pcscResp != PSIM_SUCCESS)
			{
				request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
				return;
			}
			state = SCARD_PRESENT;
			break;
		default:
			OutputDebugString(L"[POWR]unknown opcode\n");
			request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
			return;
	}

	//return ATR if available and finish the request
	setBuffer(request, ATR, ATRsize);
	
}

void VirtualReaderDevice::IoSmartCardSetAttribute(IWDFIoRequest* request,SIZE_T inBufSize,SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[SATT]IOCTL_SMARTCARD_SET_ATTRIBUTE\n");
	
	IWDFMemory *inmem=NULL;
	request->GetInputMemory(&inmem);
	
	SIZE_T size;
	BYTE *data=(BYTE *)inmem->GetDataBuffer(&size);
	

	DWORD minCode=*(DWORD*)(data);
	bool handled=false;
	if (minCode==SCARD_ATTR_DEVICE_IN_USE) {
		OutputDebugString(L"[SATT]SCARD_ATTR_DEVICE_IN_USE\n");
		request->CompleteWithInformation(STATUS_SUCCESS, 0);
		handled=true;
	}
	inmem->Release();

	if (!handled) {
		wchar_t log[300];
		swprintf(log,L"[SATT]ERROR_NOT_SUPPORTED:%08X\n",minCode);
		OutputDebugString(log);
		request->CompleteWithInformation(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED), 0);
	}
}

void VirtualReaderDevice::IoSmartCardTransmit(IWDFIoRequest* request, SIZE_T inBufSize, SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(outBufSize);
	OutputDebugString(L"[TRSM]IOCTL_SMARTCARD_TRANSMIT\n");
	BYTE * APDU = new BYTE[inBufSize];
	int APDUSize;
	getBuffer(request, APDU, &APDUSize);

	int ioRequestHeaderLength = ((SCARD_IO_REQUEST*)APDU)->cbPciLength;

	BYTE * Resp = new BYTE[outBufSize];
	DWORD RespSize = (int)(outBufSize - ioRequestHeaderLength);
	int pcscResp = PSIMTransmitICC(LUN, (char *)APDU + ioRequestHeaderLength, APDUSize - ioRequestHeaderLength, (char *)(Resp + ioRequestHeaderLength), &RespSize);

	delete[] APDU;

	if (pcscResp != PSIM_SUCCESS)
	{
		request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
		delete[] Resp;
		return;
	}

	// There is no additional PCI data and the length field includes the structure length itself
	((SCARD_IO_REQUEST*)Resp)->cbPciLength=sizeof(SCARD_IO_REQUEST); 
	((SCARD_IO_REQUEST*)Resp)->dwProtocol=SCARD_PROTOCOL_T1;

	setBuffer(request, Resp, RespSize + ((SCARD_IO_REQUEST*)Resp)->cbPciLength);
	delete[] Resp;
}

void VirtualReaderDevice::IoSmartCardControl(ULONG controlCode, IWDFIoRequest* request, SIZE_T inBufSize, SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	OutputDebugString(L"[TRSM]IOCTL_SMARTCARD_TRANSMIT\n");
	BYTE * APDU = new BYTE[inBufSize];
	int APDUSize;
	getBuffer(request, APDU, &APDUSize);


	char * params = new char [8 + 1 + APDUSize * 2 + 1 + 8 + 1];

	
	HexInt2String(controlCode, params);
	strcat(params, PSIM_MSG_DIVIDER);
	HexByteArray2String((char *)APDU, APDUSize, params+strlen(params));
	strcat(params, PSIM_MSG_DIVIDER);
	HexInt2String((unsigned int)outBufSize, params + strlen(params));

	delete[] APDU;

	char * Resp = new char[outBufSize * 2 + 1];
	int RespSize = (int)outBufSize;
	
	//	= "42000D48||00000402";
	int pcscResp = exchangePcscFunction(PSIM_MSG_FUNCTION_DEVICE_CONTROL, LUN, params, (char *)Resp, RespSize);

	if (pcscResp != PSIM_SUCCESS)
	{
		request->CompleteWithInformation(STATUS_NO_MEDIA, 0);
		delete[] Resp;
		return;
	}

	//extract response
	BYTE * OutBuffer = new BYTE[outBufSize];
	int outSize;
	if (strlen((char *)Resp) > 9) {
		outSize = HexString2CharArray((char *)&Resp[9], (char *) OutBuffer);
	}
	else {
		outSize = 0;
	}
	delete[] Resp;

	setBuffer(request, OutBuffer, outSize);
	delete[] OutBuffer;
}

void VirtualReaderDevice::IoSmartCardGetAttribute(IWDFIoRequest* request,SIZE_T inBufSize,SIZE_T outBufSize) {
	UNREFERENCED_PARAMETER(inBufSize);
	wchar_t log[300];
	DWORD code=getInt(request);
	swprintf(log,L"[GATT]  - code %0X\n",code);
	OutputDebugString(log);

	//TODO Check which of these can be forwarded to by the connector. This depends on timing while the
	//driver is loaded and needs sane defaults if the connector is not availiable.

	switch(code) {
		case SCARD_ATTR_CHARACTERISTICS:
			// 0x00000000 No special characteristics
			OutputDebugString(L"[GATT]SCARD_ATTR_CHARACTERISTICS\n");
			setInt(request,0);
			return;
		case SCARD_ATTR_VENDOR_NAME:
			OutputDebugString(L"[GATT]SCARD_ATTR_VENDOR_NAME\n");
			setString(request,"PersoSim",(int)outBufSize);
			return;
		case SCARD_ATTR_VENDOR_IFD_TYPE:
			OutputDebugString(L"[GATT]SCARD_ATTR_VENDOR_IFD_TYPE\n");
			setString(request,"Virtual Card Reader",(int)outBufSize);
			return;
		case SCARD_ATTR_DEVICE_UNIT:
			OutputDebugString(L"[GATT]SCARD_ATTR_DEVICE_UNIT\n");
			setInt(request,0);
			return;
		case SCARD_ATTR_ATR_STRING:
			OutputDebugString(L"[GATT]SCARD_ATTR_ATR_STRING\n");
			BYTE ATR[33];
			int ATRsize;
			ATRsize = 33;
			PSIMPowerICC(PSIM_POWER_UP, (char *)ATR, &ATRsize);
			setBuffer(request,ATR,ATRsize);
			return;
		case SCARD_ATTR_CURRENT_PROTOCOL_TYPE:
			OutputDebugString(L"[GATT]SCARD_ATTR_CURRENT_PROTOCOL_TYPE\n");
			setInt(request,SCARD_PROTOCOL_T1); // T=1
			return;
		default:
			swprintf(log,L"[GATT]ERROR_NOT_SUPPORTED:%08X\n",code);
			OutputDebugString(log);
	        request->CompleteWithInformation(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED), 0);
	}
}

void VirtualReaderDevice::ProcessIoControl(__in IWDFIoQueue*     queue,
                                    __in IWDFIoRequest*   request,
                                    __in ULONG            controlCode,
                                         SIZE_T           inBufSize,
                                         SIZE_T           outBufSize)
{
	inFunc
	UNREFERENCED_PARAMETER(queue);
	wchar_t log[300];
	swprintf(log,L"[IOCT]IOCTL %08X - In %i Out %i\n",controlCode,inBufSize,outBufSize);
	OutputDebugString(log);

	if (controlCode==IOCTL_SMARTCARD_GET_ATTRIBUTE) {
		IoSmartCardGetAttribute(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode==IOCTL_SMARTCARD_IS_PRESENT) {
		IoSmartCardIsPresent(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode==IOCTL_SMARTCARD_GET_STATE) {
		IoSmartCardGetState(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode==IOCTL_SMARTCARD_IS_ABSENT) {
		IoSmartCardIsAbsent(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode==IOCTL_SMARTCARD_POWER) {
		IoSmartCardPower(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode == IOCTL_SMARTCARD_SET_ATTRIBUTE) {
		IoSmartCardSetAttribute(request, inBufSize, outBufSize);
		return;
	}
	else if (controlCode==IOCTL_SMARTCARD_TRANSMIT) {
		IoSmartCardTransmit(request,inBufSize,outBufSize);
		return;
	}
	else if (controlCode == PCSC_SCARD_CONTROL) {
		IoSmartCardControl(0x42000D48, request, inBufSize, outBufSize);
		return;
	}
	else {
		IoSmartCardControl(controlCode, request, inBufSize, outBufSize);
		return;
	}
	swprintf(log,L"[IOCT]ERROR_NOT_SUPPORTED:%08X\n",controlCode);
	OutputDebugString(log);
    request->CompleteWithInformation(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED), 0);

    return;
}
BOOL CreateMyDACL(SECURITY_ATTRIBUTES * pSA)
{
     TCHAR * szSD = TEXT("D:")       // Discretionary ACL
        TEXT("(D;OICI;GA;;;BG)")     // Deny access to 
                                     // built-in guests
        TEXT("(D;OICI;GA;;;AN)")     // Deny access to 
                                     // anonymous logon	
        TEXT("(A;OICI;GRGWGX;;;AU)") // Allow 
                                     // read/write/execute 
                                     // to authenticated 
                                     // users
        TEXT("(A;OICI;GA;;;BA)");    // Allow full control 
                                     // to administrators

    if (NULL == pSA)
        return FALSE;

     return ConvertStringSecurityDescriptorToSecurityDescriptor(
                szSD,
                SDDL_REVISION_1,
                &(pSA->lpSecurityDescriptor),
                NULL);
}

HRESULT VirtualReaderDevice::OnPrepareHardware(
        __in IWDFDevice* pPreparedWdfDevice
        )
{
	inFunc

	OutputDebugString(L"OnPrepareHardware\n");
	// Store the IWDFDevice pointer
	wdfDevice = pPreparedWdfDevice;

    // Configure the default IO Queue
	HRESULT hr = QueueWrapper::CreateInstance(wdfDevice, this);

	// start the handshake server
	PSIMStartHandshakeServer(DEFAULT_PSIM_PORT);

    return hr;
}

HRESULT VirtualReaderDevice::OnReleaseHardware(
        __in IWDFDevice* releasedWdfDevice
        )
{
	inFunc
		UNREFERENCED_PARAMETER(releasedWdfDevice);

	// Release the IWDFDevice handle, if it matches
	if (releasedWdfDevice == wdfDevice.p)
    {
        wdfDevice.Release();
    }

    return S_OK;
}

HRESULT VirtualReaderDevice::OnD0Entry(IN IWDFDevice*  pWdfDevice,IN WDF_POWER_DEVICE_STATE  previousState) {
	UNREFERENCED_PARAMETER(pWdfDevice);
	UNREFERENCED_PARAMETER(previousState);
	OutputDebugString(L"OnD0Entry\n");
	return S_OK;
}

HRESULT VirtualReaderDevice::OnD0Exit(IN IWDFDevice*  pWdfDevice,IN WDF_POWER_DEVICE_STATE  newState) {
	UNREFERENCED_PARAMETER(pWdfDevice);
	UNREFERENCED_PARAMETER(newState);
	OutputDebugString(L"OnD0Exit\n");
	return S_OK;
}

void VirtualReaderDevice::shutDown(){
	// stop the handshake server
	PSIMStopHandshakeServer();
}

HRESULT VirtualReaderDevice::OnQueryRemove(IN IWDFDevice*  pWdfDevice) {
	UNREFERENCED_PARAMETER(pWdfDevice);
	OutputDebugString(L"OnQueryRemove\n");
	shutDown();
	return S_OK;
}

HRESULT VirtualReaderDevice::OnQueryStop(IN IWDFDevice*  pWdfDevice) {
	UNREFERENCED_PARAMETER(pWdfDevice);
	OutputDebugString(L"OnQueryStop\n");
	shutDown();
	return S_OK;
}

void VirtualReaderDevice::OnSurpriseRemoval(IN IWDFDevice*  pWdfDevice) {
	UNREFERENCED_PARAMETER(pWdfDevice);
	OutputDebugString(L"OnSurpriseRemoval\n");
	shutDown();
}

STDMETHODIMP_ (void) VirtualReaderDevice::OnCancel(IN IWDFIoRequest*  pWdfRequest) {
	OutputDebugString(L"OnCancel Request\n");

	bool inWaitRemoveList = (std::find(waitRemoveIpr.begin(), waitRemoveIpr.end(), pWdfRequest) != waitRemoveIpr.end());
	bool inWaitInsertList = (std::find(waitInsertIpr.begin(), waitInsertIpr.end(), pWdfRequest) != waitInsertIpr.end());

	if (inWaitRemoveList) {
		OutputDebugString(L"Cancel Remove\n");
		waitRemoveIpr.remove(pWdfRequest);
	}
	else if (inWaitInsertList) {
		OutputDebugString(L"Cancel Insert\n");
		waitInsertIpr.remove(pWdfRequest);
	}
	else {
		OutputDebugString(L"Request to cancel an unknown request => ignore \n");
		return;
	}
	pWdfRequest->UnmarkCancelable();
	pWdfRequest->CompleteWithInformation(HRESULT_FROM_WIN32(ERROR_CANCELLED), 0);
}
