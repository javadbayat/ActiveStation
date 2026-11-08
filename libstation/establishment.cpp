#define _CRT_SECURE_NO_WARNINGS

#include "station.h"

#define GetLastErrorAsHR (HRESULT_FROM_WIN32(GetLastError()))

STDAPI StationEstablish(HANDLE hStationSetup)
{
	HRESULT hr = S_OK;
	
	hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	if (SUCCEEDED(hr))
	{
		LPBC pBindContext = NULL;
		hr = CreateBindCtx( 0, &pBindContext );
		if (SUCCEEDED(hr))
		{
			LPUNKNOWN pStation = NULL;
			hr = StationGetObject( &pStation );
			if (SUCCEEDED(hr))
			{
				LPMONIKER pObjRefMon = NULL;
				hr = CreateObjrefMoniker( pStation, &pObjRefMon );
				if (SUCCEEDED(hr))
				{
					LPOLESTR pszObjRef = NULL;
					hr = pObjRefMon->GetDisplayName( pBindContext, NULL, &pszObjRef );
					if (SUCCEEDED(hr))
					{
						if (SetEnvironmentVariableW( ENV_VAR_MONIKER, pszObjRef ))
						{
							if (!SetEvent( hStationSetup ))
								hr = GetLastErrorAsHR;
						}
						else
							hr = GetLastErrorAsHR;
						
						CoTaskMemFree( pszObjRef );
					}
					
					pObjRefMon->Release();
				}

				if (FAILED(hr))
					pStation->Release();
			}
			
			pBindContext->Release();
		}
		
		CloseHandle( hStationSetup );

		if (SUCCEEDED(hr))
			Sleep( INFINITE );

		CoUninitialize();
	}

	return hr;
}