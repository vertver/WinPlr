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


/*************************************************
* ThCreateNewMutex():
* Create mutex with user name and 
* return itself
*************************************************/
HANDLE 
Player::ThreadSystem::ThCreateNewMutex(
	LPCSTR lpName
)
{
	HANDLE hMutex = CreateMutexA(NULL, FALSE, lpName);
	DO_EXIT(hMutex, "Error! Can't create mutex!");
	return hMutex;
}

/*************************************************
* ThCreateNewThread:
* Create thread with user method and 
* return thread id
*************************************************/
DWORD 
Player::ThreadSystem::ThCreateNewThread(
	LPVOID lpFunc,
	HANDLE hMutex
)
{
	DWORD lpUserThreadID = NULL;

	// create window thread
	HANDLE hThread = CreateThread(
		NULL,
		NULL,
		(LPTHREAD_START_ROUTINE)lpFunc,
		hMutex,
		NULL,
		&lpUserThreadID
	);
	DO_EXIT(hThread, "Error! Can't create thread");
	return lpUserThreadID;
}

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
