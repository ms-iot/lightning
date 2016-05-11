#include "pch.h"


#pragma warning( push )
// Ignore compiler warning about Win32-style DLLMain when compiled with /ZW flag
#pragma warning( disable : 4447 )
BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#pragma warning( pop )
