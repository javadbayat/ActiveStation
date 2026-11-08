#define _CRT_SECURE_NO_WARNINGS

#include "station.h"
#include <TlHelp32.h>
#include <tchar.h>
#include <stdlib.h>

#define GetLastErrorAsHR (HRESULT_FROM_WIN32(GetLastError()))

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

HRESULT EnableSeDebugPrivilege()
{
	HRESULT hr = S_OK;
	HANDLE hAccessToken = NULL;
	LUID luidPrivilege;
	TOKEN_PRIVILEGES tp;

	if (OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hAccessToken ))
	{
		if (LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luidPrivilege ))
		{
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luidPrivilege;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges( hAccessToken, FALSE, &tp, 0, NULL, NULL ))
				hr = GetLastErrorAsHR;
		}
		else
			hr = GetLastErrorAsHR;

		CloseHandle( hAccessToken );
	}
	else
		hr = GetLastErrorAsHR;

	return hr;
}

HRESULT GetParentProcessID(LPDWORD pdwResult)
{
    HRESULT hr = S_OK;
    DWORD dwCurrentID = GetCurrentProcessId();
    PROCESSENTRY32 pe;
    HANDLE hSnapshot;
    DWORD dwLastError;

	*pdwResult = 0;

    hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        ZeroMemory( &pe, sizeof(pe) );
        pe.dwSize = sizeof(pe);

        if (Process32First( hSnapshot, &pe ))
        {
            while (pe.th32ProcessID != dwCurrentID)
            {
                if (!Process32Next( hSnapshot, &pe ))
                {
                    dwLastError = GetLastError();
                    if (dwLastError == ERROR_NO_MORE_FILES)
                        hr = __HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                    else
                        hr = __HRESULT_FROM_WIN32(dwLastError);

                    break;
                }
            }

            if (SUCCEEDED(hr))
				*pdwResult = pe.th32ParentProcessID;
        }
        else
            hr = GetLastErrorAsHR;

		CloseHandle( hSnapshot );
    }
    else
        hr = GetLastErrorAsHR;

    return hr;
}

HRESULT GetModuleBaseAddress(DWORD dwProcessID, LPTSTR pszModuleName, PDWORD_PTR pdwResult)
{
	HRESULT hr = S_OK;
    HANDLE hSnapshot;
	MODULEENTRY32 me;
    DWORD dwLastError;

	*pdwResult = 0;

	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwProcessID );
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
        ZeroMemory( &me, sizeof(me) );
        me.dwSize = sizeof(me);

        if (Module32First( hSnapshot, &me ))
        {
            while (_tcsicmp( pszModuleName, me.szExePath ))
            {
                if (!Module32Next( hSnapshot, &me ))
                {
                    dwLastError = GetLastError();
                    if (dwLastError == ERROR_NO_MORE_FILES)
                        hr = __HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                    else
                        hr = __HRESULT_FROM_WIN32(dwLastError);

                    break;
                }
            }

            if (SUCCEEDED(hr))
				*pdwResult = DWORD_PTR( me.modBaseAddr );
        }
        else
            hr = GetLastErrorAsHR;

		CloseHandle( hSnapshot );
	}
	else
		hr = GetLastErrorAsHR;

	return hr;
}

EXTERN_C void CALLBACK StationInject(HWND, HINSTANCE hSelf, LPSTR, int)
{
	HRESULT hr = S_OK;
	HANDLE hStationSetup[2] = { NULL };

	hSelf = HINSTANCE( &__ImageBase );

	hStationSetup[1] = CreateEvent( NULL, TRUE, FALSE, NULL );
	if (hStationSetup[1])
	{
		TCHAR szDllFullPath[DLL_PATH_SIZE];
		if (DWORD dwPathLength = GetModuleFileName( hSelf, szDllFullPath, DLL_PATH_SIZE ))
		{
			hr = EnableSeDebugPrivilege();
			if (SUCCEEDED(hr))
			{
				DWORD dwParentProcessID = 0;
				hr = GetParentProcessID( &dwParentProcessID );
				if (SUCCEEDED(hr))
				{
					HANDLE hParentProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE, FALSE, dwParentProcessID );
					if (hParentProcess)
					{
						LPVOID pRemoteBuffer = VirtualAllocEx( hParentProcess, NULL, (dwPathLength + 1) * sizeof(TCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
						if (pRemoteBuffer)
						{
							if (WriteProcessMemory( hParentProcess, pRemoteBuffer, szDllFullPath, (dwPathLength + 1) * sizeof(TCHAR), NULL ))
							{
								HANDLE hRemoteThread = CreateRemoteThread( hParentProcess, NULL, 0, (LPTHREAD_START_ROUTINE) LoadLibrary, pRemoteBuffer, 0, NULL );
								if (hRemoteThread)
								{
									if (WaitForSingleObject( hRemoteThread, INFINITE ) == WAIT_OBJECT_0)
									{
										DWORD_PTR pfnStationEstablish = 0;
										hr = GetModuleBaseAddress( dwParentProcessID, szDllFullPath, &pfnStationEstablish);
										if (SUCCEEDED(hr))
										{
											pfnStationEstablish += DWORD_PTR( StationEstablish ) - DWORD_PTR( hSelf );
													
											HANDLE hRemoteEvent = NULL;
											if (DuplicateHandle( GetCurrentProcess(), hStationSetup[1], hParentProcess, &hRemoteEvent, EVENT_MODIFY_STATE, FALSE, 0 ))
											{
												HANDLE &hRemoteThread = hStationSetup[0];
												hRemoteThread = CreateRemoteThread( hParentProcess, NULL, 0, (LPTHREAD_START_ROUTINE) pfnStationEstablish, hRemoteEvent, 0, NULL );
												if (hRemoteThread)
												{
													switch (WaitForMultipleObjects( 2, hStationSetup, FALSE, SETUP_TIMEOUT ))
													{
													case WAIT_OBJECT_0 :
														{
															if (!GetExitCodeThread( hRemoteThread, (LPDWORD) &hr ))
																hr = GetLastErrorAsHR;

															break;
														}
													case WAIT_TIMEOUT :
														{
															hr = STATION_E_SETUP_TIMEOUT;
															break;
														}
													case WAIT_FAILED :
														{
															hr = GetLastErrorAsHR;
															break;
														}
													}

													CloseHandle( hRemoteThread );
												}
												else
													hr = GetLastErrorAsHR;
											}
											else
												hr = GetLastErrorAsHR;
										}
									}
									else
										hr = GetLastErrorAsHR;

									CloseHandle( hRemoteThread );
								}
								else
									hr = GetLastErrorAsHR;
							}
							else
								hr = GetLastErrorAsHR;
					
							VirtualFreeEx( hParentProcess, pRemoteBuffer, 0, MEM_RELEASE );
						}
						else
							hr = GetLastErrorAsHR;
				
						CloseHandle( hParentProcess );
					}
					else
						hr = GetLastErrorAsHR;
				}
			}
		}
		else
			hr = GetLastErrorAsHR;

		CloseHandle( hStationSetup[1] );
	}
	else
		hr = GetLastErrorAsHR;

	if (FAILED(hr))
		ExitProcess( (UINT) hr );
}