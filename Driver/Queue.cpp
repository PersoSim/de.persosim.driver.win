#include "internal.h"
#include "Device.h"
#include "Queue.h"


QueueWrapper::QueueWrapper() :
    parentDevice(NULL)
{
	state=0;
}

QueueWrapper::~QueueWrapper()
{
    // Release the reference that was incremented in CreateInstance()
    SAFE_RELEASE(parentDevice);
}

HRESULT QueueWrapper::CreateInstance(__in IWDFDevice*  pWdfDevice, VirtualReaderDevice* pMyDevice)
{
	inFunc
		CComObject<QueueWrapper>* queueInstance = NULL;

    if(NULL == pMyDevice)
    {
        return E_INVALIDARG;
    }

	HRESULT hr = CComObject<QueueWrapper>::CreateInstance(&queueInstance);

    if(SUCCEEDED(hr))
    {
        // AddRef the object
		queueInstance->AddRef();

        // Store the parent device object
		queueInstance->parentDevice = pMyDevice;

        // Increment the reference for the lifetime of the QueueWrapper object.
		queueInstance->parentDevice->AddRef();

        CComPtr<IUnknown> spIUnknown;
		hr = queueInstance->QueryInterface(IID_IUnknown, (void**)&spIUnknown);

        if(SUCCEEDED(hr))
        {
            // Create the framework queue
            CComPtr<IWDFIoQueue> spDefaultQueue;
            hr = pWdfDevice->CreateIoQueue( spIUnknown,
                                            TRUE,                        // DefaultQueue
                                            WdfIoQueueDispatchParallel,  // Parallel queue handling
                                            FALSE,                       // PowerManaged
                                            TRUE,                        // AllowZeroLengthRequests
                                            &spDefaultQueue 
                                            );
            if (FAILED(hr))
            {
				OutputDebugString (L"IoQueue NOT Created\n");
            }
			else
				OutputDebugString (L"IoQueue Created\n");

        }

        // Release the queueInstance pointer when done. Note: UMDF holds a reference to it above
		SAFE_RELEASE(queueInstance);
    }

    return hr;
}

STDMETHODIMP_(void) QueueWrapper::OnDeviceIoControl(
    __in IWDFIoQueue*     pQueue,
    __in IWDFIoRequest*   pRequest,
    __in ULONG            ControlCode,
         SIZE_T           InputBufferSizeInBytes,
         SIZE_T           OutputBufferSizeInBytes
    )
{
	parentDevice->ProcessIoControl(pQueue,pRequest,ControlCode,InputBufferSizeInBytes,OutputBufferSizeInBytes);
    
}
