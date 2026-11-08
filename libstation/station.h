#pragma once
#include <Windows.h>

#define ENV_VAR_MONIKER (L"ActiveStation")
#define SETUP_TIMEOUT (3000)
#define DLL_PATH_SIZE MAX_PATH

#define CUSTOM_ERROR_CODE(code) MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0200 + (code) )

#define STATION_E_SETUP_TIMEOUT CUSTOM_ERROR_CODE(1)

EXTERN_C void CALLBACK StationInject(HWND, HINSTANCE, LPSTR, int);
STDAPI StationEstablish(HANDLE hStationSetup);
STDAPI StationGetObject(LPUNKNOWN *ppStation);