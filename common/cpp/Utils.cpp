#include <windows.h>
#include <stdio.h>
#include "../includes/Utils.h"

extern void ErrorExit(LPTSTR lpszFunction) 
{ 
    TCHAR szBuf[256]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

	swprintf_s (szBuf, sizeof (szBuf),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
 
    MessageBox (NULL, szBuf, TEXT("Error"), MB_OK | MB_ICONERROR);

	ExitProcess (dw);
    LocalFree(lpMsgBuf);     
}

extern void TraceLastError (LPTSTR lpszFunction) 
{ 
    TCHAR szBuf[256]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    swprintf_s(szBuf, sizeof (szBuf),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
 
    OutputDebugString (szBuf);
	
    LocalFree(lpMsgBuf);     
}