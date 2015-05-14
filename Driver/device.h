
#include <winscard.h>
#include <list>

#pragma once
/*
This module contains the implementation of the driver device callback object.
It implements the the needed PNP interfaces to get callbacks after setup. This
module is used in the Windows User-Mode Driver Framework (WUDF).
*/
class ATL_NO_VTABLE VirtualReaderDevice :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IPnpCallbackHardware,
	public IPnpCallback,
	public IRequestCallbackCancel
{
public:
	~VirtualReaderDevice() {}

	DECLARE_NOT_AGGREGATABLE(VirtualReaderDevice)

	BEGIN_COM_MAP(VirtualReaderDevice)
        COM_INTERFACE_ENTRY(IPnpCallbackHardware)
        COM_INTERFACE_ENTRY(IPnpCallback)
		COM_INTERFACE_ENTRY(IRequestCallbackCancel)
    END_COM_MAP()

	VirtualReaderDevice(VOID)
    {
        wdfDevice = NULL;
		state = SCARD_ABSENT;
    }

public:
	/*
	This method creates and initializs an instance of the virtual smart card reader driver's
	device callback object

	Parameters:
		FxDriver - a location to store the referenced pointer to the device object
		FxDeviceInit - a pointer to the settings for the device

	Returns:
		HRESULT
	*/
    static
    HRESULT CreateInstance(
        __in IWDFDriver *FxDriver,
        __in IWDFDeviceInitialize *FxDeviceInit
        );

	void  ProcessIoControl(__in IWDFIoQueue*     pQueue,
                                    __in IWDFIoRequest*   pRequest,
                                    __in ULONG            ControlCode,
                                         SIZE_T           InputBufferSizeInBytes,
                                         SIZE_T           OutputBufferSizeInBytes);

	/*
	Called by the driver framework to prepare the device

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object representing the device

	Returns:
		S_OK: success
	*/
    STDMETHOD_ (HRESULT, OnPrepareHardware)(__in IWDFDevice* pWdfDevice);

	/*
	Called by the driver framework to release the device

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device

	Returns:
		S_OK
	*/
    STDMETHOD_ (HRESULT, OnReleaseHardware)(__in IWDFDevice* pWdfDevice);

	/*
	This method is called by the UMDF when the device enters the working state (device fully functional)

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device
		previousState - the state the device had previous to entering the working state

	Returns:
		S_OK
	*/
	STDMETHOD_(HRESULT, OnD0Entry)(IN IWDFDevice*  pWdfDevice, IN WDF_POWER_DEVICE_STATE  previousState);

	/*
	This method is called by the UMDF when the device leaves the working state (device fully functional)

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device
		newState - the state the device will have after to leaving the working state

	Returns:
		S_OK
	*/
	STDMETHOD_(HRESULT, OnD0Exit)(IN IWDFDevice*  pWdfDevice, IN WDF_POWER_DEVICE_STATE  newState);

	/*
	This method is called by the UMDF in case of the removal PNP event.
	It performs a driver shutdown.

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device

	Returns:
		S_OK
	*/
	STDMETHOD_(HRESULT, OnQueryRemove)(IN IWDFDevice*  pWdfDevice);

	/*
	This method is called by the UMDF in case of the stop PNP event.
	It performs a driver shutdown.

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device

	Returns:
		S_OK
	*/
	STDMETHOD_(HRESULT, OnQueryStop)(IN IWDFDevice*  pWdfDevice);

	/*
	This method is called by the UMDF in case of a not anticipated device removal.
	It performs a driver shutdown.

	Parameters:
		pWdfDevice - pointer to an IWDFDevice object for the device
	*/
    STDMETHOD_ (void, OnSurpriseRemoval)(IN IWDFDevice*  pWdfDevice);

	/*
	This is called by the UMDF in case of a canceled I/O request and performs
	cleanup of the given request.

	Parameters:
		pWdfRequest - pointer to an IWDFIoRequest object that has been canceled
	*/
    STDMETHOD_ (void, OnCancel)(IN IWDFIoRequest*  pWdfRequest);

	/*
	This method blocks while no card is present and handles the pending IoRequests.
	*/
	void waitForCardPresence();

	/*
	This method blocks until the card is absent. It also handles the pending requests.
	*/
	void waitForCardAbsence();
private:

	CComPtr<IWDFDevice>     wdfDevice;
	std::list<IWDFIoRequest*> waitRemoveIpr, waitInsertIpr;
	int state;

	/*
	This implements the IOCTL_SMARTCARD_GET_ATTRIBUTE call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardGetAttribute(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_IS_PRESENT call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardIsPresent(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_GET_STATE call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardGetState(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_IS_ABSENT call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardIsAbsent(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_POWER call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardPower(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_SET_ATTRIBUTE call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardSetAttribute(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the IOCTL_SMARTCARD_TRANSMIT call

	Parameters:
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardTransmit(IWDFIoRequest* pRequest,SIZE_T inBufSize,SIZE_T outBufSize);

	/*
	This implements the PCSC SCARD_CONTROL call

	Parameters:
		ControlCode - the control code to send
		pRequest - the I/O request to respond to
		inBufSize - the requests input buffer size
		outBufSize - the requests output buffer size
	*/
	void IoSmartCardControl(ULONG ControlCode, IWDFIoRequest* pRequest, SIZE_T inBufSize, SIZE_T outBufSize);

	/*
	Shutdown of the device driver
	*/
	void shutDown();
};

