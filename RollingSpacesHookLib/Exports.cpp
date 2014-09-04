#include "stdafx.h"
#include "Exports.h"
#include "../common/includes/Utils.h"
#include "HwndQueueProtector.h"

extern int g_nLastElement;
extern HWND g_hwndQueue[];
extern volatile BOOL g_bRollingSpaceWorking;

#define GLOBAL_FLAG_PROTECTION_EVENT_NAME TEXT("GlobalFlagProtectionEvent")

#define BUFFER_SIZE 512

static VOID dumpQueue (LPCTSTR szFunc)
{
	TCHAR buf[BUFFER_SIZE];
	for (int i = 0; i <= g_nLastElement; i ++) {
		wsprintf (buf, TEXT ("%s: %dth hwnd = %x,"), szFunc, i, g_hwndQueue[i]);
		OutputDebugString (buf);
	}	
}

extern "C" VOID _declspec (dllexport) HwndQueueClear ()
{	
	CHwndQueueProtector protector;

	g_nLastElement = -1;
}

extern "C" BOOL _declspec (dllexport) HwndQueuePushBack (HWND hwnd)
{	
	CHwndQueueProtector protector;

	if (g_nLastElement < MAX_HWND - 1) {
		g_hwndQueue[++ g_nLastElement] = hwnd;
		//dumpQueue (TEXT ("HwndQueuePushBack"));
		return TRUE;
	} else
		return FALSE;
}

extern "C" VOID _declspec (dllexport) HwndRemoveFromQueue (HWND hwnd)
{	
	CHwndQueueProtector protector;

	for (int i = 0; i <= g_nLastElement; i ++) {
		if (hwnd == g_hwndQueue[i]) {
			for (int j = i; j < g_nLastElement; j ++) {
				g_hwndQueue[j] = g_hwndQueue[j + 1];
			}
			-- g_nLastElement;
			-- i;
		}
	}
	//dumpQueue (TEXT ("HwndRemoveFromQueue"));
}

extern "C" BOOL _declspec (dllexport) HwndExistInQueue (HWND hwnd)
{	
	CHwndQueueProtector protector;

	for (int i = 0; i <= g_nLastElement; i ++) {
		if (hwnd == g_hwndQueue[i]) {
			//dumpQueue (TEXT ("HwndExistInQueue"));
			return TRUE;
		}
	}
	//dumpQueue (TEXT ("HwndExistInQueue"));
	return FALSE;
}

extern "C" HWND _declspec (dllexport) HwndQueuePop ()
{	
	CHwndQueueProtector protector;

	if (g_nLastElement < 0) {
		//dumpQueue (TEXT ("HwndQueuePop"));
		//OutputDebugString (TEXT ("g_nLastElement = -1"));
		return NULL;
	} else {
		HWND hwndTemp = g_hwndQueue[0];
		for (int i = 0; i < g_nLastElement; i ++) {		
			g_hwndQueue[i] = g_hwndQueue[i + 1];						
		}
		-- g_nLastElement;

		//dumpQueue (TEXT ("HwndQueuePop"));

		return hwndTemp;
	}
}

extern "C" HWND _declspec (dllexport) HwndQueueHead ()
{	
	CHwndQueueProtector protector;

	if (g_nLastElement < 0) {		
		return NULL;
	} else {
		return g_hwndQueue[0];
	}
}

extern "C" BOOL _declspec (dllexport) GetGlobalFlag ()
{	
	BOOL flag;

	HANDLE hGlobalFlagEvent = CreateEvent (NULL, FALSE, TRUE, GLOBAL_FLAG_PROTECTION_EVENT_NAME);

	if (NULL == hGlobalFlagEvent) {
		TraceLastError (TEXT ("Creating/Openning global event protection event"));
		return FALSE;
	}

	WaitForSingleObject (hGlobalFlagEvent, INFINITE);

	flag = g_bRollingSpaceWorking;

	SetEvent (hGlobalFlagEvent);

	return flag;
}

extern "C" VOID _declspec (dllexport) SetGlobalFlag (BOOL flag)
{	
	HANDLE hGlobalFlagEvent = CreateEvent (NULL, FALSE, TRUE, GLOBAL_FLAG_PROTECTION_EVENT_NAME);

	if (NULL == hGlobalFlagEvent) {
		TraceLastError (TEXT ("Creating/Openning global event protection event"));
		return;
	}

	WaitForSingleObject (hGlobalFlagEvent, INFINITE);

	g_bRollingSpaceWorking = flag;

	SetEvent (hGlobalFlagEvent);
}