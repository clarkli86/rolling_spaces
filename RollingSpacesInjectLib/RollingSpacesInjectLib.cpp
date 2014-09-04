// RollingSpacesInjectLib.cpp : Defines the entry poINT for the DLL application.
//

#include "stdafx.h"
#include <process.h>
#include "RollingSpacesInjectLib.h"
#include "SpaceManagerDlg.h"
#include "../common/includes/Utils.h"
#include "Globals.h"
#include "UserMessages.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

// Thread procedure of rolling spaces
unsigned _stdcall RollingSpacesThreadProc (LPVOID lParam);

unsigned _stdcall RollingSpacesDaemonThreadProc (LPVOID lParam);

// Global RollingSpaces Object
CRollingSpaces g_RollingSpaces;
CSpaceManagerDlg g_SpaceManagerDlg;

// Handle to the main window
HWND ghwndRollingSpaces;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			{					
				// Store the module handle of this dll
				SetCurrentModuleHandle (hModule);				

				RegisterUserMessages ();

				_beginthreadex (NULL, 0, RollingSpacesThreadProc, NULL, NULL, NULL);
			}
			break;
		case DLL_PROCESS_DETACH:
			{				
			}
			break;
	}
    return TRUE;
}

/*
 * If a thread is ternimated, all windows it created will be destroyed.
 * Here we create a message loop to handle the message for our window procedure.
 */
unsigned _stdcall RollingSpacesThreadProc (LPVOID lParam)
{
	ghwndRollingSpaces = g_RollingSpaces.CreateRollingSpacesWindow ();
	if (NULL == ghwndRollingSpaces) {
		TraceLastError (TEXT ("Create Rolling Spaces Window"));
		return -1;
	}

	//_beginthreadex (NULL, 0, RollingSpacesDaemonThreadProc, hwndRollingSpaces, NULL, NULL);

	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0)) {		
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
	return (unsigned)msg.wParam;
}

unsigned _stdcall RollingSpacesDaemonThreadProc (LPVOID lParam)
{
	HWND hwndRollingSpaces = (HWND)lParam;
	while (TRUE) {
		if (!IsWindowVisible (hwndRollingSpaces))
			ShowWindow (hwndRollingSpaces, SW_SHOW);

		// System sent this message to window without sending WM_SIZE after WM_SETTINGCHAGNE
		UINT rowCount = g_RollingSpaces.GetConfig ()->GetRowCount ();
		UINT columnCount = g_RollingSpaces.GetConfig ()->GetColumnCount ();

		RECT rect;
		GetClientRect (hwndRollingSpaces, &rect);
		UINT spaceWidth = (rect.right - rect.left) / columnCount;
		UINT spaceHeight = (rect.bottom - rect.top) / rowCount;
		for (size_t i = 0; i < rowCount; i ++) {
			for (size_t j = 0; j < columnCount; j ++) {
				if (0 == MoveWindow (g_RollingSpaces.GetWorkSpace (i * columnCount + j)->GetSafeHwnd (),
					spaceWidth * j, spaceHeight * i, 
					spaceWidth, spaceHeight,
					TRUE)) {
						TraceLastError (TEXT ("Move child work spaces"));
				}				
			}
		}

		Sleep (500);
	}
	return 0;
}
#ifdef _MANAGED
#pragma managed(pop)
#endif