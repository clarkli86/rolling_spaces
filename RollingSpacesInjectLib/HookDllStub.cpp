#include "stdafx.h"
#include "../common/includes/Utils.h"
#include "HookDllStub.h"

VOID HwndQueueClearStub ()
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return;
	}

	typedef VOID (*HwndQueueClear) ();
	HwndQueueClear proc = (HwndQueueClear) GetProcAddress (hHookLib, HWNDQUEUECLEARPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address HwndQueueClear of Hook dll"));
		return;
	} else 
		proc ();
}

BOOL HwndQueuePushBackStub (HWND hwnd)
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return FALSE;
	}

	typedef BOOL (*HwndQueuePushBack) (HWND);
	HwndQueuePushBack proc = (HwndQueuePushBack) GetProcAddress (hHookLib, HWNDQUEUEPUSHBACKPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address HwndQueuePushBack of Hook dll"));
		return FALSE;
	} else 
		return proc (hwnd);
}

HWND HwndQueueHeadStub ()
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return FALSE;
	}

	typedef HWND (*HwndQueueHead) ();
	HwndQueueHead proc = (HwndQueueHead) GetProcAddress (hHookLib, HWNDQUEUEHEADPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address HwndQueueHead of Hook dll"));
		return NULL;
	} else 
		return proc ();
}

HWND HwndQueuePopStub ()
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return FALSE;
	}

	typedef HWND (*HwndQueuePop) ();
	HwndQueuePop proc = (HwndQueuePop) GetProcAddress (hHookLib, HWNDQUEUEPOPPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address HwndQueuePop of Hook dll"));
		return NULL;
	} else 
		return proc ();
}

BOOL GetGlobalFlagStub ()
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
		return FALSE;
	}

	typedef BOOL (*GetGlobalFlag) ();
	GetGlobalFlag proc = (GetGlobalFlag) GetProcAddress (hHookLib, GETGLOBALFLAGPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address GetGlobalFlag of Hook dll"));
		return FALSE;
	} else 
		return proc ();
}

VOID SetGlobalFlagStub (BOOL flag)
{
	HMODULE hHookLib = GetModuleHandle (ROLLINGSPACESHOOKLIBNAME);
	if (NULL == hHookLib) {
		TraceLastError (TEXT ("Get Module Handle of Hook dll"));
	}

	typedef VOID (*SetGlobalFlag) (BOOL);
	SetGlobalFlag proc = (SetGlobalFlag) GetProcAddress (hHookLib, SETGLOBALFLAGPROC);
	if (NULL == proc) {
		TraceLastError (TEXT ("Get Proc Address SetGlobalFlag of Hook dll"));
	} else 
		proc (flag);
}