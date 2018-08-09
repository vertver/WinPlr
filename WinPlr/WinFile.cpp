/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio file-system
**********************************************************
* WinFile.cpp
* File-system for WinPlr
*********************************************************/
#include "WinAudio.h"
#include "WinFileReader.h"

/*************************************************
* LoadFileToBuffer():
* Load data to structs and handles
*************************************************/
HANDLE_DATA
Player::Buffer::LoadFileToBuffer(
	HWND hwnd,
	HANDLE hData,
	FILE_DATA dFile,
	PCM_DATA dPCM,
	FFT_DATA dFFT
)
{
	// set zero for our structs
	ZeroMemory(&dFile, sizeof(FILE_DATA));
	ZeroMemory(&dPCM, sizeof(PCM_DATA));
	ZeroMemory(&dFFT, sizeof(FFT_DATA));

	// set filedialog struct
	OPENFILENAMEA oFN;
	char szName[MAX_PATH];
	szName[0] = '\0';		// needy for correct filedialog work

	// get params to our struct
	ZeroMemory(&oFN, sizeof(OPENFILENAMEA));
	oFN.lStructSize = sizeof(OPENFILENAMEA);
	oFN.hwndOwner = hwnd;
	oFN.nMaxFile = MAX_PATH;
	oFN.lpstrFile = szName;
	oFN.lpstrFilter = "Audio files (.wav)\0*.wav\0";
	oFN.lpstrTitle = "Open audio file";
	oFN.lpstrFileTitle = NULL;
	oFN.lpstrInitialDir = NULL;
	oFN.nFilterIndex = 1;
	oFN.nMaxFileTitle = 0;
	oFN.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// if we can't open filedialog - exit
	if (!GetOpenFileNameA(&oFN))
	{
		CreateInfoText("The file is empty.");
		ExitProcess(FALSE);
	}

	// if filedialog can launch
	hData = CreateFileA(
		oFN.lpstrFile,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	// if we can't open file - exit
	DO_EXIT(hData, "Filesystem error! Can't find file!");

	// we must allocate our file to use it
	DWORD dwSize = GetFileSize(hData, NULL);
	LPVOID lpData = malloc(dwSize);
	DWORD dwSizeWritten = NULL;

	// if we can't allocate - exit
	DO_EXIT(
		ReadFile(hData, lpData, dwSize, &dwSizeWritten, NULL),
		"Filesystem error! Can't allocate pointer with data!"
	);

	// get params to our structs
	dFile.dwSize = dwSizeWritten;
	dFile.eType = WAV_FILE;
	dFile.hFile = hData;
	dFile.lpFile = lpData;

	CloseHandle(hData);

	// 
	FileReader fReader;
	PCM_DATA fileType = fReader.GetFullFileInfo(oFN.lpstrFile);

	dPCM.bWindows = TRUE;						// WINAPI methods play audio
	dPCM.dwSamplerate = fileType.dwSamplerate;
	dPCM.wBitrate = fileType.wBitrate;
	dPCM.dwTime = 10000;						// 10000 msecs - 10 secs
	dPCM.lpData = lpData;
	dPCM.lpName = oFN.lpstrFileTitle;
	dPCM.lpPath = oFN.lpstrFile;
	dPCM.wChannels = fileType.wChannels;

	dFFT.lpData = lpData;
	dFFT.dwSizeBuffer = 1024;
	dFFT.dwSizeWindow = 2048;
	dFFT.eType = BLACKMANHARRIS_WINDOW;

	HANDLE_DATA hdReturn = { };
	ZeroMemory(&hdReturn, sizeof(HANDLE_DATA));
	hdReturn.dData = dFile;
	hdReturn.dFFT = dFFT;
	hdReturn.dPCM = dPCM;
	hdReturn.hwnd = hwnd;
	return hdReturn;
}

/*************************************************
* UnloadFileFromBuffer():
* Free all handles and structs
*************************************************/
VOID
Player::Buffer::UnloadFileFromBuffer(
	HANDLE_DATA hdData
)
{
	CloseHandle(hdData.dData.hFile);
}

/*************************************************
* CheckBufferFile():
* Checks buffer on validate
*************************************************/
BOOL
Player::Buffer::CheckBufferFile(
	HANDLE_DATA hdData
)
{
	if (!hdData.hwnd && !hdData.dData.lpFile)
		return FALSE;
	else
		return TRUE;
}
