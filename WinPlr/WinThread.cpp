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

/*************************************************
* ThSetNewThreadName:
* Set user name for thread by exception
*************************************************/
VOID 
Player::ThreadSystem::ThSetNewThreadName(
	LPCSTR lpName
)
{
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


/*************************************************
* ThBeginXAudioThread:
* Create new thread with XAudio2 methods
*************************************************/
VOID 
Player::ThreadSystem::ThBeginXAudioThread(
	AUDIO_FILE audioFile
)
{
	_beginthread((_beginthread_proc_type)CreateXAudioThread, NULL, &audioFile);
}

/*************************************************
* ThSetNewThreadName:
* Set user name for thread by Windows 10 method
*************************************************/
BOOL
Player::ThreadSystem::ThSetNewWin10ThreadName(
	LPCWSTR lpName
)
{
	// load kerner32 library for check our method
	HMODULE hLib = LoadLibraryA("Kernel32.dll");
	DO_EXIT(hLib, "Can't load kernel32");

	// if our Windows version is 10.0.1603 or greater - 
	// try to use SetThreadDescription function
	if (GetProcAddress(hLib, "SetThreadDescription"))
	{
		// set thread name by Windows 10 function
		if (SUCCEEDED(SetThreadDescription(GetCurrentThread(), lpName)))
		{
			return TRUE;
		}
		else
		{
			__debugbreak();
			DEBUG_MESSAGE("WINPLR: Can't set thread name by WIN10API");
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
