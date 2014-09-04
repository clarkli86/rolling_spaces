#include "stdafx.h"
#include "Globals.h"
#include "../common/includes/Utils.h"

/* --------------------Global Variables ------------------------*/
// Global variable module handle
HMODULE gCurrentModule;

UINT g_rollingSpacesAreaWidth = 50;
//TODO: test, it will be read out from file in the future
UINT g_rowCount = 2;
UINT g_columnCount = 2;

std::map <HWND, std::list<HWND> > gHwndMapList;
/* --------------------Global Variables ------------------------*/

extern HINSTANCE GetCurrentModuleHandle () 
{
	return gCurrentModule;
}

extern void SetCurrentModuleHandle (HMODULE hModule)
{
	_ASSERTE (hModule != NULL);
	gCurrentModule = hModule;
}

MouseActionEnum GetPrevMouseActionStub ()
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return MOUSE_ACTION_NONE;
	}

	typedef MouseActionEnum (*GetPrevMouseAction) ();
	GetPrevMouseAction proc = (GetPrevMouseAction) GetProcAddress (hHookLib, GETPREVMOUSEACTIONPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address GetPrevMouseAction of Hook dll"));
		return MOUSE_ACTION_NONE;
	}

	return proc ();
}

BOOL SetPrevMouseActionStub (MouseActionEnum mouseAction)
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return FALSE;
	}

	typedef VOID (*SetPrevMouseAction) (MouseActionEnum action);
	SetPrevMouseAction proc = (SetPrevMouseAction) GetProcAddress (hHookLib, SETPREVMOUSEACTIONPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address SetPrevMouseAction of Hook dll"));
		return FALSE;
	} else 
		proc (mouseAction);

	return TRUE;
}