#pragma once

/*
This implements the I/O queue interface and delegates the occuring requests to the device
*/
class ATL_NO_VTABLE QueueWrapper :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IQueueCallbackDeviceIoControl
{
public:
	virtual ~QueueWrapper();

	DECLARE_NOT_AGGREGATABLE(QueueWrapper)

	BEGIN_COM_MAP(QueueWrapper)
        COM_INTERFACE_ENTRY(IQueueCallbackDeviceIoControl)
    END_COM_MAP()

	/*
	This creates the queue as done by the COM factory creation system

	Parameters:
		pWdfDevice - A pointer to the CWSSDevice object
		pMyDevice - pointer to the parent device to be used

	Return Values:
		S_OK: The queue was created successfully
	*/
	static HRESULT CreateInstance(__in IWDFDevice* pWdfDevice, VirtualReaderDevice* pMyDevice);

// COM Interface methods
	int state;

	/*
	This is called by the framework if an IOCTL is sent to the device

	Parameters:
		pQueue - pointer to an IO queue
		pRequest - pointer to an IO request
		ControlCode - The IOCTL to process
		InputBufferSizeInBytes - the size of the input buffer
		OutputBufferSizeInBytes - the size of the output buffer
	*/
    STDMETHOD_ (void, OnDeviceIoControl)(
        __in IWDFIoQueue*    pQueue,
        __in IWDFIoRequest*  pRequest,
        __in ULONG           ControlCode,
        SIZE_T          InputBufferSizeInBytes,
        SIZE_T          OutputBufferSizeInBytes
        );

protected:
	QueueWrapper();

private:
	VirtualReaderDevice*          parentDevice; // Parent device object
};

