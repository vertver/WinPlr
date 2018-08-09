/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio messages
**********************************************************
* WinMsg.cpp
* WinPlr messages handler
*********************************************************/
#include "WinAudio.h"

/*************************************************
* CreateWarningText():
* Create message with warning and close process
*************************************************/
VOID 
CreateWarningText(
	LPCSTR lpMsgText
)
{
	MessageBoxA(
		NULL,
		lpMsgText,
		"Warning",
		MB_OK |
		MB_ICONWARNING
	);
	__debugbreak();
	ExitProcess(FALSE);
	// if warning - FALSE
}

/*************************************************
* CreateErrorText():
* Create message with error and close process
*************************************************/
VOID 
CreateErrorText(
	LPCSTR lpMsgText
)
{
	DWORD dwError = GetLastError();
	LPSTR lpMessage = NULL;
	LPCSTR lpCMessage = NULL;
	std::string szString;

	// if we got system error
	if (!(dwError == NOERROR))
	{
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			lpMessage,
			NULL,
			NULL
		);
		lpCMessage = lpMessage;
	}
	else
	{
		lpCMessage = "No error";
	}

	szString =
		("#ERROR#" +
		std::string("\n") +
		PLAYER_VERSION +
		std::string("\n") +
		std::string("#############################################") +
		lpMsgText +
		std::string("\n") +
		std::string("#############################################\n") +
		std::string("\n") +
		"Do you wand to break program? Press 'yes' if want, or 'no' if don't.");

	// display message with error string
	int msg = MessageBoxA(
		NULL,
		szString.c_str(),
		"Error",
		MB_YESNOCANCEL |
		MB_ICONHAND
	);

	switch (msg)
	{
	case IDYES:	
		// if error - TRUE
		__debugbreak();
		ExitProcess(TRUE);
		break;
	case IDCANCEL:
	case IDNO:
	default:
		break;
	}


}

/*************************************************
* CreateErrorText():
* Create message with error and close process
* (HRESULT info)
*************************************************/
VOID
CreateErrorText(
	LPCSTR lpMsgText,
	HRESULT hr
)
{
	// get all our errors to display
	DWORD dwError = GetLastError();
	_com_error err(hr);
	LPCSTR lpError = err.ErrorMessage();
	LPSTR lpMessage = NULL;
	LPCSTR lpCMessage = NULL;
	std::string szString;

	// if we got system error
	if (!(dwError == NOERROR))
	{
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			lpMessage,
			NULL,
			NULL
		);
		lpCMessage = lpMessage;
	}
	else
	{
		lpCMessage = "No error";
	}

	szString =
		"#ERROR#" +
		std::string("\n") +
		PLAYER_VERSION +
		std::string("\n") +
		std::string("#############################################") +
		lpMsgText +
		std::string("\n") +
		std::string("#############################################\n") +
		"#DESCRIPTION#\n" +
		lpError + std::string("(") + "HRESULT: " +
		std::to_string(hr) + std::string(")") +
		std::string("\n") +
		"Do you wand to break program? Press 'yes' if want, or 'no' if don't.";

	// display message with error string
	int msg = MessageBoxA(
		NULL,
		szString.c_str(),
		"Error",
		MB_YESNOCANCEL |
		MB_ICONHAND
	);

	switch (msg)
	{
	case IDCANCEL:
	case IDYES:
		// if error - TRUE
		__debugbreak();
		ExitProcess(TRUE);
		break;
	case IDNO:
	default:
		break;
	}
}

/*************************************************
* CreateInfoText():
* Create message with information
*************************************************/
VOID 
CreateInfoText(
	LPCSTR lpMsgText
)
{
	MessageBoxA(
		NULL,
		lpMsgText,
		"Info",
		MB_OK |
		MB_ICONASTERISK
	);
	__debugbreak();
	// if info - nothing
}

/*************************************************
* ContinueIfYes():
* Create message with switch case.
* If pressed button yes - continue.
* Else - break and exit
*************************************************/
VOID
ContinueIfYes(
	LPCSTR lpMsgText,
	LPCSTR lpMsgTitle
) 
{
	int iMsg = MessageBoxA(
		NULL,
		lpMsgText,
		lpMsgTitle,
		MB_YESNOCANCEL |
		MB_ICONASTERISK
	);
	switch (iMsg)
	{
	case IDYES:
		break;
	case IDNO:
	case IDCANCEL:
	default:
		ExitProcess(TRUE);
	}
}