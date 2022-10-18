#include "LocDefines.h"
#include <streams.h>
#include <initguid.h>
#include "LocGuids.h"
#include "RegMacros.h"
#include "TSPushFileSource.h"

//------------------------------------------------------------------------
// Global variables
//
HINSTANCE hDll;

//------------------------------------------------------------------------
// Filter registration template
//
static const AMOVIESETUP_MEDIATYPE OutputPinTypes[] =
{
    MTYPES(MEDIASUBTYPE_MPEG2_TRANSPORT)
};

static const AMOVIESETUP_PIN Pins[] =
{
    DEFINEPIN(Output, TRUE, OutputPinTypes)
};

static const AMOVIESETUP_FILTER TSPushFileSource[] =
{
    DEFINEFILTER(CLSID_TSPUSHFILESOURCE, TSPUSHFILESOURCE_NAMEL, TSPUSHFILESOURCE_MERIT, Pins)
};

CFactoryTemplate g_Templates[] =
{
    DEFINETEMPLATE(TSPUSHFILESOURCE_NAMEL, CLSID_TSPUSHFILESOURCE, CTSPushFileSource::CreateInstance, TSPushFileSource)
};

int g_cTemplates = SIZEOFARRAY(g_Templates);

//------------------------------------------------------------------------
// DLL entry point
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            hDll = hinstDLL;
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return DllEntryPoint(hinstDLL, fdwReason, lpReserved);
}

//------------------------------------------------------------------------
// Filter registration
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}
