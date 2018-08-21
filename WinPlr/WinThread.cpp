/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio thread-system
**********************************************************
* WinThread.cpp
* Thread system for WinPlr
*********************************************************/

#include "WinAudio.h"
#include "WinXAudio.h"

using SET_THREAD_DESCRIPTION_CALL = HRESULT(WINAPI *)(HANDLE handle, PCWSTR name);
static SET_THREAD_DESCRIPTION_CALL lpSetThreadDescription = NULL;

/*************************************************
* ThSetNewThreadName:
* Set user name for thread 
*************************************************/
VOID
Player::ThreadSystem::ThSetNewThreadName(
	_In_ LPCSTR lpName
)
{
	// load kerner32 library for check our method
	HMODULE hLib = GetModuleHandle("kernel32.dll");
	lpSetThreadDescription = (SET_THREAD_DESCRIPTION_CALL)GetProcAddress(hLib, "SetThreadDescription");

	// if our Windows version is 10.0.1603 or greater - 
	// try to use SetThreadDescription function
	if (lpSetThreadDescription)
	{
		// set thread name by Windows 10 function
		R_ASSERT2(lpSetThreadDescription(GetCurrentThread(), GetUnicodeStringFromAnsi(lpName)), "Can't set thread name");
	}
	else
	{
		// else make by custom method
		THREAD_NAME stName;
		ZeroMemory(&stName, sizeof(THREAD_NAME));
		stName.dwType = 0x1000;
		stName.dwThreadID = DWORD(-1);		// current thread
		stName.dwFlags = NULL;
		stName.lpName = lpName;

		// try to raise exception to set name of thread
		__try
		{
			RaiseException(
				0x406D1388,
				NULL,
				sizeof(stName) /
				sizeof(ULONG_PTR),
				(ULONG_PTR*)&stName
			);
		}
		__except (EXCEPTION_CONTINUE_EXECUTION)
		{
		}
	}
}
