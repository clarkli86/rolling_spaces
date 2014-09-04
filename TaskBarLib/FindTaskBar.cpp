#include "stdafx.h"
#include "FindTaskBar.h"
#include <assert.h>

static HWND FindSubWindowOfTaskBar (HINSTANCE hInstance, UINT TrayClassStringID, const TCHAR *windowClass, const TCHAR *windowTitle);
static HWND FindSubWindow (HWND parentHwnd, const TCHAR *windowClass, const TCHAR *windowTitle);

HWND GetHandleOfQuickLaunch (HINSTANCE hInstance, UINT QuickLaunchClassStringID, UINT QuickLaunchTitleStringID, UINT TrayClassStringID)
{
	TCHAR   quickLaunchClass[256];
	TCHAR   quickLaunchTitle[256];

	if (!LoadString (hInstance, QuickLaunchClassStringID, quickLaunchClass, 256) ||
		!LoadString (hInstance, QuickLaunchTitleStringID, quickLaunchTitle, 256)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}
	return FindSubWindowOfTaskBar (hInstance, TrayClassStringID, quickLaunchClass, quickLaunchTitle);
}

HWND GetHandleOfRunningApps (HINSTANCE hInstance, UINT RunAppsClassStringID, UINT RunAppsTitleStringID, UINT TrayClassStringID)
{
	TCHAR   runAppsClass[256];
	TCHAR   runAppsTitle[256];

	if (!LoadString (hInstance, RunAppsClassStringID, runAppsClass, 256) ||
		!LoadString (hInstance, RunAppsTitleStringID, runAppsTitle, 256)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}

	return FindSubWindowOfTaskBar (hInstance, TrayClassStringID, runAppsClass, runAppsTitle);
}

HWND GetHandleOfRollingSpaces (HINSTANCE hInstance, UINT RollingSpacesClassID, UINT TrayClassStringID)
{
	TCHAR   rollingSpacesClass[256];

	if (!LoadString (hInstance, RollingSpacesClassID, rollingSpacesClass, 256)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}

	return FindSubWindowOfTaskBar (hInstance, TrayClassStringID, rollingSpacesClass, NULL);
}

HWND GetHandleOfTrayNotifyWnd (HINSTANCE hInstance, UINT TrayNotifyWndClassID, UINT TrayClassStringID)
{
	TCHAR   trayNotifyWndClass[256];	

	if (!LoadString (hInstance, TrayNotifyWndClassID, trayNotifyWndClass, 256)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}

	return FindSubWindowOfTaskBar (hInstance, TrayClassStringID, trayNotifyWndClass, NULL);
}

static HWND FindSubWindowOfTaskBar (HINSTANCE hInstance, UINT TrayClassStringID, const TCHAR *windowClass, const TCHAR *windowTitle)
{
	HWND hwndTray = NULL;
	HWND hwndTarget = NULL;

	TCHAR trayClass[256];

	if (!LoadString (hInstance, TrayClassStringID, trayClass, 256)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}

	hwndTray = FindWindow (trayClass, NULL);
	if (NULL != hwndTray) {
		hwndTarget = FindSubWindow (hwndTray, windowClass, windowTitle);
	}

	return hwndTarget;
}

static HWND FindSubWindow (HWND parentHwnd, const TCHAR *windowClass, const TCHAR *windowTitle)
{
	HWND hwndTarget = NULL;

	if (NULL != parentHwnd) {
		hwndTarget = FindWindowEx (parentHwnd, NULL, windowClass, windowTitle);

		if (NULL == hwndTarget) {
			HWND hwndChild = FindWindowEx (parentHwnd, NULL, NULL, NULL);
			//recursive find in the sub windows
			while (NULL != hwndChild && NULL == hwndTarget) {
				hwndTarget = FindSubWindow (hwndChild, windowClass, windowTitle);				

				hwndChild = FindWindowEx (parentHwnd, hwndChild, NULL, NULL);
			}
		}
	}

	return hwndTarget;
}

int GetBandIndex (HWND hwndRebar, HWND hwndBand)
{
	UINT nCount = (UINT)SendMessage (hwndRebar, RB_GETBANDCOUNT, 0, 0);
	for (size_t i = 0; i < nCount; i ++) {
		REBARBANDINFO rebarInfo = {0};
		rebarInfo.fMask = RBBIM_CHILD;
		LRESULT lRes = SendMessage (hwndRebar, RB_GETBANDINFO, i, (LPARAM)&rebarInfo);
		_ASSERT (0 != lRes);

		if (hwndBand == rebarInfo.hwndChild) {
			//Found
			return (int)i;
		}
	}
	
	//Not found
	return -1;
}