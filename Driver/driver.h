#pragma once

/*
This module contains the core driver callback object. It handles the events
the system sends to the driver.
*/
class ATL_NO_VTABLE VirtualReaderDriver :
    public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<VirtualReaderDriver, &CLSID_VirtualSmartCardReaderDriver>,
    public IDriverEntry
{
public:
	VirtualReaderDriver();

    DECLARE_NO_REGISTRY()
    DECLARE_CLASSFACTORY()
	DECLARE_NOT_AGGREGATABLE(VirtualReaderDriver)

	BEGIN_COM_MAP(VirtualReaderDriver)
        COM_INTERFACE_ENTRY(IDriverEntry)
    END_COM_MAP()

	/*
	Global, device independent initializations are performed in this method

	Parameters:
		pDriver - pointer to an IWDFDriver object
	*/
    STDMETHOD  (OnInitialize)(__in IWDFDriver* pDriver);
	
	/*
	This creates the instance of the device callback object when the framework
	detects a new device.

	Parameters:
		pDriver - pointer to an IWDFDriver object
		pDeviceInit - pointer to a device initialization object

	Returns:
		S_OK: device initialized successfully
	*/
    STDMETHOD  (OnDeviceAdd)(__in IWDFDriver* pDriver, __in IWDFDeviceInitialize* pDeviceInit);
	
	/*
	This method is called by the framework immediately before de-initializing itself.
	WDF resources should be released here.
	*/
    STDMETHOD_ (void, OnDeinitialize)(__in IWDFDriver* pDriver);
};

OBJECT_ENTRY_AUTO(__uuidof(VirtualSmartCardReaderDriver), VirtualReaderDriver)

