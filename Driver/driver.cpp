#include "internal.h"
#include "virtualsmartcardreader.h"
#include "driver.h"
#include "device.h"
#include <stdio.h>

VirtualReaderDriver::VirtualReaderDriver()
{
}


HRESULT VirtualReaderDriver::OnDeviceAdd(
    __in IWDFDriver* pDriver,
    __in IWDFDeviceInitialize* pDeviceInit
    )
{
	inFunc
    HRESULT hr = VirtualReaderDevice::CreateInstance(pDriver, pDeviceInit);

    return hr;
}

HRESULT VirtualReaderDriver::OnInitialize(
    __in IWDFDriver* pDriver
    )
{
    UNREFERENCED_PARAMETER(pDriver);
    return S_OK;
}

void VirtualReaderDriver::OnDeinitialize(
    __in IWDFDriver* pDriver
    )
{
    UNREFERENCED_PARAMETER(pDriver);
    return;
}

