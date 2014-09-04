// RollingSpacesHookLib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "RollingSpacesHookLib.h"
#include "../TaskBarLib/FindTaskBar.h"
#include "../common/includes/Utils.h"
#include "Exports.h"
#include "resource.h"
#include "Globals.h"
#include "HwndQueueProtector.h"
#include "DetourUser32.h"
#include <list>
#include <algorithm>
#ifdef _MANAGED
#pragma managed(push, off)
#endif

#define EVENT_WAIT_TIME 3000

/*------------------------------Global Shared Variables------------------------------------*/
/***********************These variables will be shared by all processes in the system!!!!!!!!!!!!!!!!!!!!!!*/
#pragma data_seg(".RollingSpacesGlobalShared")
HHOOK g_lhMouseHook		= NULL;
HHOOK g_lhShellHook	    = NULL;
HHOOK g_lhCallWndHook    = NULL;
HHOOK g_lhCallWndRetHook = NULL;
HHOOK g_lhCBTHook = NULL;

MouseActionEnum g_MouseAction = MOUSE_ACTION_NONE;
HWND			g_hwndDragging = NULL;

HWND g_hwndQueue[MAX_HWND] = {NULL};
int  g_nLastElement = -1;

BOOL volatile g_bRollingSpaceWorking = FALSE;
#pragma data_seg()
// Tell linker to shared this data segment
#pragma comment(linker,"/SECTION:.RollingSpacesGlobalShared,RWS") 
/***********************These variables will be shared by all processes in the system!!!!!!!!!!!!!!!!!!!!!!*/

HINSTANCE g_lhInstance   = NULL;

#define WM_REGISTER_WINDOW_TEXT TEXT("WM_REGISTER_WINDOW")
#define WM_UNREGISTER_WINDOW_TEXT TEXT("WM_UNREGISTER_WINDOW")

UINT WM_REGISTER_WINDOW = 0;
UINT WM_UNREGISTER_WINDOW = 0;

/*------------------------------Global Variables, different copies are owned by each process------------------------------------*/
//LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
//
//LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam);
//
//LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL IsWindowTaskBarDisplayable (HWND hwnd);
VOID RegisterWindow (HWND hwnd);
VOID UnregisterWindow (HWND hwnd);
BOOL IsWindowTaskBarDisplayable (HWND hwnd);

static HWND GetHwndOfRollingSpaces ();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{	
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			{
				g_lhInstance = hModule;
				
				if (!CHwndQueueProtector::InitializeEvent ())
					return FALSE;

				WM_REGISTER_WINDOW = RegisterWindowMessage (WM_REGISTER_WINDOW_TEXT);
				WM_UNREGISTER_WINDOW = RegisterWindowMessage (WM_UNREGISTER_WINDOW_TEXT);
				
				//todo: disable detouring at this moment
				//DetourUser32API ();
			}
			break;
		case DLL_PROCESS_DETACH:
			{
				//RestoreUser32API ();
			}
			break;
		default:
			break;
	}

	return TRUE;
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)  // do not process the message 
		return CallNextHookEx (g_lhCallWndHook, nCode, wParam, lParam);

	PCWPSTRUCT pCW = (PCWPSTRUCT) lParam;

	if (IsWindowTaskBarDisplayable (pCW->hwnd)) {
		switch (pCW->message) {
			case WM_CREATE:
				{
					LPCREATESTRUCT lpCS = (LPCREATESTRUCT) lParam;
					if (NULL == lpCS->hwndParent && (WS_SYSMENU | WS_VISIBLE ) & lpCS->style)
						RegisterWindow (pCW->hwnd);
				}
				break;
			case WM_DESTROY:
				UnregisterWindow (pCW->hwnd);
				break;		
			case WM_SHOWWINDOW:
				{
					if (IsWindowTaskBarDisplayable (pCW->hwnd)) {
						if (pCW->wParam)
							RegisterWindow (pCW->hwnd);
						else
							UnregisterWindow (pCW->hwnd);
					}
				}
				break;
			/*case WM_STYLECHANGED:
				{
					LPSTYLESTRUCT lpStyle = (LPSTYLESTRUCT)pCW->lParam;

					if (!(lpStyle->styleNew & WS_CHILD) && lpStyle->styleOld & WS_CHILD)
						RegisterWindow (pCW->hwnd);
					else if (lpStyle->styleNew & WS_CHILD && !(lpStyle->styleOld & WS_CHILD))
						UnregisterWindow (pCW->hwnd);
				}
				break;*/

			/*case WM_WINDOWPOSCHANGED:
				{					
					LPWINDOWPOS lpWndPos = (LPWINDOWPOS) lParam;
					if (SWP_SHOWWINDOW == (lpWndPos->flags & SWP_SHOWWINDOW)) {
						OutputDebugString (TEXT ("SWP_SHOWWINDOW"));
						RegisterWindow (lpWndPos->hwnd);
					}
					else if (SWP_HIDEWINDOW == (lpWndPos->flags & SWP_HIDEWINDOW)) {
						OutputDebugString (TEXT ("SWP_HIDEWINDOW"));
						UnregisterWindow (lpWndPos->hwnd);
					}
				}
				break;*/
			default:
				break;
		}	
	}

	return CallNextHookEx (g_lhCallWndHook, nCode, wParam, lParam); 
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)  // do not process the message 
		return CallNextHookEx (g_lhCallWndRetHook, nCode, wParam, lParam);	

	/*PCWPRETSTRUCT pCW = (PCWPRETSTRUCT) lParam;

	if (IsWindowTaskBarDisplayable (pCW->hwnd)) {
		if (WM_CLOSE == pCW->message && 0 == pCW->lResult)
			UnregisterWindow (pCW->hwnd);
	}*/

	return CallNextHookEx (g_lhCallWndRetHook, nCode, wParam, lParam); 
}

// hwnd is assumed to be taskbar-displayable
VOID RegisterWindow (HWND hwnd)
{
	/*
	 * if the rollingspace is displaying its own windows
	 * first check the hwnd queue if hwnd is in the queue, if it is in, just show it, if not, wait until rollingspaces ends up.
	 */
	BOOL flag = GetGlobalFlag ();
	if (flag) {		
		if (!HwndExistInQueue (hwnd)) {
			while (flag) {			
				flag = GetGlobalFlag ();
				Sleep (100);
			}
			PostMessage (GetHwndOfRollingSpaces (), 
				WM_REGISTER_WINDOW, (WPARAM)hwnd, 0);
		}
		// If a window is in the queue, it will be displayed directly.
	} else {
		// Add this window to the active workspace		
		PostMessage (GetHwndOfRollingSpaces (), 
			WM_REGISTER_WINDOW, (WPARAM)hwnd, 0);
	}
}

VOID UnregisterWindow (HWND hwnd)
{
	/*
	 * if the rollingspace is displaying its own windows
	 * first check the hwnd queue if hwnd is in the queue, if it is in, just show it, if not, wait until rollingspaces ends up.
	 */
	BOOL flag = GetGlobalFlag ();
	if (flag) {		
		if (!HwndExistInQueue (hwnd)) {
			while (flag) {			
				flag = GetGlobalFlag ();
				Sleep (100);
			}
			PostMessage (GetHwndOfRollingSpaces (), 
				WM_UNREGISTER_WINDOW, (WPARAM)hwnd, 0);			
		}
		// If a window is in the queue, it will be closed/hided directly.
	} else {
		PostMessage (GetHwndOfRollingSpaces (), 
			WM_UNREGISTER_WINDOW, (WPARAM)hwnd, 0);		
	}	
}

// Check if a window is displayable on taskbar according to its caption text, window style and parent window
// TODO: A window could be added to taskbar mannualy by calling ITaskBar COM interface.
BOOL IsWindowTaskBarDisplayable (HWND hwnd)
{
	if (NULL == GetParent (hwnd)) {				
		long windowStyle = GetWindowLong (hwnd, GWL_STYLE);
		if ((windowStyle & WS_SYSMENU)) 
			return TRUE;				
	}
	return FALSE;
}

static HWND GetHwndOfRollingSpaces ()
{
	static HWND hwndRollingSpaces = NULL;
	if (NULL == hwndRollingSpaces) {
		hwndRollingSpaces = GetHandleOfRollingSpaces (g_lhInstance, IDS_ROLLINGSPACES_CLASS, IDS_TRAYCLASS);		
	}

	if (NULL == hwndRollingSpaces)
		TraceLastError (TEXT ("Get Handle Of RollingSpaces"));

	return hwndRollingSpaces;
}

static HWND GetHwndOfTrayNotifyWnd ()
{
	static HWND hwndRollingSpaces = NULL;
	if (NULL == hwndRollingSpaces) {
		hwndRollingSpaces = GetHandleOfRollingSpaces (g_lhInstance, IDS_ROLLINGSPACES_CLASS, IDS_TRAYCLASS);		
	}

	if (NULL == hwndRollingSpaces)
		TraceLastError (TEXT ("Get Handle Of RollingSpaces"));

	return hwndRollingSpaces;
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{	
	return CallNextHookEx (g_lhCBTHook, nCode, wParam, lParam); 
}

extern "C" BOOL _declspec (dllexport) InstallRollingSpacesCallWndHook ()
{
	if (NULL == (g_lhCallWndHook = SetWindowsHookEx (WH_CALLWNDPROC, CallWndProc, g_lhInstance, 0))) {
		TraceLastError (TEXT ("Install CallWndProc Hook"));
		return FALSE;
	}
	return TRUE;
}

extern "C" BOOL _declspec (dllexport) UninstallRollingSpacesCallWndHook ()
{
	return (0 != UnhookWindowsHookEx (g_lhCallWndHook));
}

extern "C" BOOL _declspec (dllexport) InstallRollingSpacesCallWndRetHook ()
{
	if (NULL == (g_lhCallWndRetHook = SetWindowsHookEx (WH_CALLWNDPROCRET, CallWndRetProc, g_lhInstance, 0))) {
		TraceLastError (TEXT ("Install CallWndRetProc Hook"));
		return FALSE;
	}
	return TRUE;
}

extern "C" BOOL _declspec (dllexport) UninstallRollingSpacesCallWndRetHook ()
{
	return (0 != UnhookWindowsHookEx (g_lhCallWndRetHook));
}

extern "C" BOOL _declspec (dllexport) InstallRollingSpacesCBTHook ()
{
	if (NULL == (g_lhCBTHook = SetWindowsHookEx (WH_CBT, CBTProc, g_lhInstance, 0))) {
		TraceLastError (TEXT ("Install CBTProc Hook"));
		return FALSE;
	}
	return TRUE;
}

extern "C" BOOL _declspec (dllexport) UninstallRollingSpacesCBTHook ()
{
	return (0 != UnhookWindowsHookEx (g_lhCBTHook));
}

extern "C" BOOL _declspec (dllexport) InstallRollingSpacesHooks ()
{
	return InstallRollingSpacesCallWndHook () &&
		InstallRollingSpacesCallWndRetHook ();
}

extern "C" BOOL _declspec (dllexport) UninstallRollingSpacesHooks ()
{
	return UninstallRollingSpacesCallWndHook () &&
		UninstallRollingSpacesCallWndRetHook ();
}

#ifdef _MANAGED
#pragma managed(pop)
#endif