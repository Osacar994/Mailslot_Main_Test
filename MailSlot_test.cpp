// MailSlot_test.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"

HANDLE hSlot;
LPTSTR SlotName = TEXT("\\\\.\\mailslot\\sample_mailslot");

BOOL WINAPI MakeSlot(LPTSTR lpszSlotName)
{
	hSlot = CreateMailslot(lpszSlotName,0,MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES) NULL);

	if(hSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailSlot failed with %d\n", GetLastError());
		return FALSE;
	}
	else printf("Mailslot created successfully. \n");
	return TRUE;
}

BOOL WriteSlot(HANDLE hSlot, LPTSTR lpszMessage)
{
	BOOL fResult;
	DWORD cdWritten;

	fResult = WriteFile(hSlot,lpszMessage,(DWORD)(lstrlen(lpszMessage)+1)*sizeof(TCHAR),&cdWritten,(LPOVERLAPPED) NULL);

	if(!fResult)
	{
		printf("WriteFile failed with %d. \n", GetLastError());
		return FALSE;
	}
	printf("Slot written to successfully. \n");
	return TRUE;
}

BOOL ReadSlot(void)
{
	DWORD cbMessage, cMessage, cbRead;
	BOOL fResult;
	LPTSTR lpszBuffer;
	TCHAR achID[80];
	DWORD cAllMessages;
	HANDLE hEvent;
	OVERLAPPED ov;

	cbMessage = cMessage = cbRead = 0;

	hEvent = ::CreateEvent( NULL, FALSE, FALSE, TEXT("ExampleSlot"));
	if(NULL ==hEvent) return FALSE;
	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.hEvent = hEvent;

	fResult = GetMailslotInfo( hSlot, (LPDWORD) NULL, &cbMessage, &cMessage,(LPDWORD) NULL);

	if(!fResult)
	{
		printf("GetMailslotInfo failed with %d. \n", GetLastError());
		return FALSE;
	}

	if(cbMessage == MAILSLOT_NO_MESSAGE)
	{
		printf("Waiting for a message...\n");
		return TRUE;
	}

	cAllMessages = cMessage;

	while (cMessage != 0)
	{
		::StringCchPrintf((LPTSTR) achID,80,TEXT("\nMessage #%d of %d \n"), cAllMessages - cMessage +1, cAllMessages);

		lpszBuffer = (LPTSTR) GlobalAlloc(GPTR,lstrlen((LPTSTR) achID)*sizeof(TCHAR) + cbMessage);

		if(NULL == lpszBuffer) return FALSE;
		lpszBuffer[0] = '\0';

		fResult = ::ReadFile(hSlot,lpszBuffer,cbMessage,&cbRead,&ov);

		if(!fResult)
		{
			printf("ReadFile failed with %d. \n", GetLastError());
			::GlobalFree((HGLOBAL) lpszBuffer);
			return FALSE;
		}

		::StringCbCat(lpszBuffer, lstrlen((LPTSTR) achID)*sizeof(TCHAR) + cbMessage,(LPTSTR) achID);

		// Mostrar el Mensaje

		_tprintf(TEXT("Contents of the mailslot: %s\n"), lpszBuffer);

		::GlobalFree((HGLOBAL) lpszBuffer);

		fResult = GetMailslotInfo(hSlot,(LPDWORD) NULL, &cbMessage, &cMessage,(LPDWORD) NULL);

		if(!fResult)
		{
			printf("GetMailslotInfo failed (%d)\n", GetLastError());
			return FALSE;
		}
	}
	CloseHandle(hEvent);
	return TRUE;
}
int _tmain(int argc, _TCHAR* argv[])
{
	MakeSlot(SlotName);
	// Escribir en el MailSlot
	HANDLE hFile;
	hFile = CreateFile(SlotName,GENERIC_WRITE,FILE_SHARE_READ,(LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d. \n", GetLastError());
		system("pause");
		return FALSE;
	}

	WriteSlot(hFile, TEXT("Message one for mailslot."));
	WriteSlot(hFile, TEXT("Message two for mailslot."));

	Sleep(5000);

	WriteSlot(hFile, TEXT("Message three for mailslot."));

	CloseHandle(hFile);
	
	//Leer en el MailSlot
	while (true)
	{
		ReadSlot();
		Sleep(3000);
	}

	system("pause");
	return TRUE;
}

