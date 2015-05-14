
#include "internal.h"

class VirtualSmartCardDriverModule : public CAtlDllModuleT<VirtualSmartCardDriverModule> {} _AtlModule;

/*
This is the entry point to the DLL.

Parameters:
	instanceHandle  - Handle to the DLL module
	reason   - Indicates why the DLL entry point is being called
	reserved - Additional information based on reason

Returns:
	TRUE  = initialization succeeds
	FALSE = initialization fails
*/

extern "C" BOOL WINAPI DllMain(HINSTANCE    instanceHandle,
                               DWORD        reason,
                               LPVOID       reserved)
{
    (reserved);

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(instanceHandle);

			break;

        case DLL_PROCESS_DETACH:
            _AtlModule.Term();
            break;

        default:
            break;
    }

    // Call the ATL module class so it can initialize
    return _AtlModule.DllMain(reason, reserved); 
}

/*
This method is called by COM to instantiate the driver callback object.

Parameters:
	classIdToGet - Id of the requested object
	interfaceId - The interface the caller wants to use
	pInterface - A location used to store the referenced interface pointer

Returns:
	S_OK: The object was retrieved successfully.
	CLASS_E_CLASSNOTAVAILABLE: The DLL does not support the class
*/
STDAPI DllGetClassObject(__in REFCLSID classIdToGet, __in REFIID interfaceId, __deref_out LPVOID* pInterface)
{
    return _AtlModule.DllGetClassObject(classIdToGet, interfaceId, pInterface);
}

