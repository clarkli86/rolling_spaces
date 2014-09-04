#pragma once
#include <windows.h>
#include <list>
#include "ProcessWindow.h"
#include "Process.h"

#define WORKSPACEWINDOWCLASSNAME          TEXT("WorkspaceWindow32")

typedef std::list <CProcessWindow> ListProcessWindow;
typedef std::list <CProcess> ListProcessHandle;

class CWorkspace
{
public:
	CWorkspace ();
	explicit CWorkspace(HWND hwndWorkSpace);
	~CWorkspace(void);

	HWND GetSafeHwnd (void);

	VOID SetHwnd (HWND hwnd);

	VOID RegisterWindow (HWND);

	VOID UnregisterWindow (HWND);

	BOOL IsActive ();

	VOID Activate ();

	VOID Deactivate ();

	ListProcessWindow GetProcessWindows () const;

	ListProcessHandle GetProcessHandles () const;

	VOID SetProcessWindows (const ListProcessWindow&);

	VOID SetProcessHandles (const ListProcessHandle&);
	
	BOOL HwndExists (const HWND hwnd) const;

	VOID CaptureRemainingWindows ();

	VOID CaptureAndHideVisibleWindows ();

	static BOOL InitWorkSpaceWindowClass ();

	VOID ReleaseWindows ();
private:
	static LRESULT CALLBACK CWorkspace::WorkSpaceWindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ListProcessWindow CaptureVisibleWindows () const;

	VOID SaveForegroundWindow ();
	HWND GetPreviousForegroundWindow () const;

	VOID testInit ();

	// private data
	HWND m_hwndWorkSpace;

	BOOL m_IsActive;

	ListProcessWindow m_listProcessWindow;// Stores the window handle and its process handle
	ListProcessHandle m_listProcessHandle;// If a handle is in this list, that means it is contrlled, all its windows will be added to this workspace

	HWND m_hwndForegroundWindow;// Stores the hwnd of foreground window
};
