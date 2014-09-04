#include "DetourUser32.h"
#include <Tlhelp32.h>
#include "detours.h"
#include <windows.h>
#include "../common/includes/Utils.h"
#include "../TaskBarLib/FindTaskBar.h"
#include "Exports.h"
#include "resource.h"

#define BUFFER_SIZE 512

BOOL (WINAPI * True_ShowWindow) (HWND hWnd, int nCmdShow) = ShowWindow;
BOOL (WINAPI * True_SetWindowPos) (HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) = SetWindowPos;
HWND (WINAPI * True_SetParent) (HWND hWndChild, HWND hWndNewParent) = SetParent;

static BOOL WINAPI Mine_ShowWindow (HWND hWnd, int nCmdShow);
static BOOL WINAPI Mine_SetWindowPos (HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
static HWND WINAPI Mine_SetParent (HWND hWndChild, HWND hWndNewParent);

extern BOOL IsWindowTaskBarDisplayable (HWND hwnd);
extern VOID RegisterWindow (HWND hwnd);
extern VOID UnregisterWindow (HWND hwnd);

static VOID UpdateAllThreadsInProcess ();

// external variables
extern HINSTANCE g_lhInstance;

VOID DetourUser32API ()
{
	LONG error;
	
	error = DetourTransactionBegin();

	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourTransactionBegin detouring ShowWindow() %d"), error);
		OutputDebugString (buf);
		return;
	}

	// Update every thread in owner process
	//UpdateAllThreadsInProcess ();	
	
	error = DetourAttach(&(PVOID&)True_ShowWindow, Mine_ShowWindow);	
	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourAttach detouring %d"), error);
		OutputDebugString (buf);
		return;
	}

	error = DetourAttach(&(PVOID&)True_SetWindowPos, Mine_SetWindowPos);	
	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourAttach detouring SetWindowPos() %d"), error);
		OutputDebugString (buf);
		return;
	}

	error = DetourAttach(&(PVOID&)True_SetWindowPos, Mine_SetParent);	
	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourAttach detouring SetParent() %d"), error);
		OutputDebugString (buf);
		return;
	}
	
	error = DetourTransactionCommit();
	
	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourTransactionCommit %d"), error);
		OutputDebugString (buf);
	}

	/*HMODULE hProcess = GetModuleHandle (NULL);
	TCHAR szProcessName[BUFFER_SIZE], szMsg[BUFFER_SIZE];
	GetModuleFileName (hProcess, szProcessName, BUFFER_SIZE);
	wsprintf (szMsg, TEXT ("Detoured Process: %s"), szProcessName);
	OutputDebugString (szMsg);*/
}

VOID RestoreUser32API ()
{
	DetourTransactionBegin();

	// Update every thread in owner process
	//UpdateAllThreadsInProcess ();

	DetourDetach(&(PVOID&)True_ShowWindow, Mine_ShowWindow);
	DetourDetach(&(PVOID&)True_SetWindowPos, Mine_SetWindowPos);
	DetourDetach(&(PVOID&)True_SetWindowPos, Mine_SetParent);

	LONG error;
	error = DetourTransactionCommit();
	
	if (NO_ERROR != error) {
		TCHAR buf[BUFFER_SIZE];
		wsprintf (buf, TEXT ("Hook dll: DetourTransactionCommit ShowWindow() %d"), error);
		OutputDebugString (buf);
	}
}

static BOOL WINAPI Mine_ShowWindow (HWND hWnd, int nCmdShow)
{
	if (IsWindowTaskBarDisplayable (hWnd) || IsChild (GetHandleOfTrayNotifyWnd (g_lhInstance, IDS_TRAYNOTIFYWND_CLASS, IDS_TRAYCLASS), hWnd)) {
		switch (nCmdShow) {
			case SW_HIDE:				
					UnregisterWindow (hWnd);
				break;			
			default:				
					RegisterWindow (hWnd);
				break;
		}
	}

	return True_ShowWindow (hWnd, nCmdShow);
}

static BOOL WINAPI Mine_SetWindowPos (HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	if (IsWindowTaskBarDisplayable (hWnd) || IsChild (GetHandleOfTrayNotifyWnd (g_lhInstance, IDS_TRAYNOTIFYWND_CLASS, IDS_TRAYCLASS), hWnd)) {
		if (uFlags & SWP_SHOWWINDOW)
			RegisterWindow (hWnd);		
		else if (uFlags & SWP_HIDEWINDOW)
			UnregisterWindow (hWnd);
	}

	return True_SetWindowPos (hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

static HWND WINAPI Mine_SetParent (HWND hWndChild, HWND hWndNewParent)
{
	HWND previousParent = True_SetParent (hWndChild, hWndNewParent);

	if (NULL == hWndNewParent)
		RegisterWindow (hWndChild);
	else if (NULL != hWndNewParent)
		UnregisterWindow (hWndChild);

	return previousParent;
}

static VOID UpdateAllThreadsInProcess ()
{
	DWORD  dwPID = GetCurrentProcessId ();
	HANDLE hThreadSnap = CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, dwPID);
	if (INVALID_HANDLE_VALUE == hThreadSnap) {
		TraceLastError (TEXT ("Create Thread Snapshot"));
	}

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32); 

	if (!Thread32First (hThreadSnap, &te32)) {
		TraceLastError (TEXT ("ThreadFirst32"));
		CloseHandle (hThreadSnap);
		return;
	}

	LONG error;
	HANDLE hThread;
	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do {
		if (te32.th32OwnerProcessID == dwPID) {
			hThread = OpenThread (THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
			if (NULL == hThread) {
				TraceLastError (TEXT ("OpenThread"));
				break;;
			}

			error = DetourUpdateThread (hThread);

			if (NO_ERROR != error) {
				TCHAR buf[BUFFER_SIZE];
				wsprintf (buf, TEXT ("Hook dll: DetourUpdateThread ShowWindow() %d"), error);
				OutputDebugString (buf);
				break;
			}

			CloseHandle (hThread);
		}
	} while( Thread32Next(hThreadSnap, &te32 ) ); 

	//  Don't forget to clean up the snapshot object.
	CloseHandle (hThreadSnap);	
}