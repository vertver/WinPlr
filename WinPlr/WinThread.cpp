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
* set user name for thread by exception
*************************************************/
VOID 
Player::ThreadSystem::ThSetNewThreadName(
	DWORD dwThreadID,
	LPCSTR lpName
)
{
	THREAD_NAME stName;
	ZeroMemory(&stName, sizeof(THREAD_NAME));
	stName.dwType = 0x1000;
	stName.dwThreadID = dwThreadID;
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