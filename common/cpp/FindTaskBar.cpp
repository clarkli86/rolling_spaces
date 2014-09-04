#include "../includes/FindTaskBar.h"
#include "Globals.h"
#include "resource.h"
#include <assert.h>

static HWND FindSubWindowofTaskBar (const TCHAR *windowClass, const TCHAR *windowTitle);
static HWND FindSubWindow (HWND parentHwnd, const TCHAR *windowClass, const TCHAR *windowTitle);

extern HWND GetHandleOfQuickLaunch ()
{
	TCHAR   quickLaunchClass[255];
	TCHAR   quickLaunchTitle[255];

	HINSTANCE hInstance = GetInstanceHandle ();

	if (!LoadString (hInstance, IDS_QUICKLAUNCH_CLASS, quickLaunchClass, 255) ||
		!LoadString (hInstance, IDS_QUICKLAUNCH_TITLE, quickLaunchTitle, 255)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}
	return FindSubWindowofTaskBar (quickLaunchClass, quickLaunchTitle);
}

extern HWND GetHandleOfRunningApps ()
{
	TCHAR   runAppsClass[255];
	TCHAR   runAppsTitle[255];

	HINSTANCE hInstance = GetInstanceHandle ();

	if (!LoadString (hInstance, IDS_RUNAPPS_CLASS, runAppsClass, 255) ||
		!LoadString (hInstance, IDS_RUNAPPS_TITLE, runAppsTitle, 255)) {
		MessageBox (NULL, TEXT ("Unable to read resource file!"), TEXT ("ERROR"), 
			MB_OK | MB_ICONSTOP);
		return NULL;
	}

	return FindSubWindowofTaskBar (runAppsClass, runAppsTitle);
}

static HWND FindSubWindowofTaskBar (const TCHAR *windowClass, const TCHAR *windowTitle)
{
	HWND hwndTray = NULL;	
	HWND hwndTarget = NULL;

	TCHAR trayClass[255];

	HINSTANCE hInstance = GetInstanceHandle ();

	if (!LoadString (hInstance, IDS_TRAYCLASS, trayClass, 255)) {
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