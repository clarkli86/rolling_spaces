#pragma once
#include <windows.h>
#include <list>
#include "ProcessWindow.h"
#include "WorkSpace.h"
#include "Process.h"

#define WORKSPACECTLWNDCLASSNAME          TEXT("WorkspaceCtlWnd32")

class CWorkspaceCtl
{
public:
	CWorkspaceCtl(void);
	explicit CWorkspaceCtl (HWND);
	~CWorkspaceCtl(void);

	HWND GetSafeHwnd (void);

	VOID SetHwnd (HWND);

	VOID AddProcessWindows (const ListProcessWindow&);

	VOID AddProcessHandles (const ListProcessHandle&);

	VOID RemoveProcessWindows (const ListProcessWindow&);

	BOOL IsActive ();

	VOID Activate ();

	VOID Deactivate ();

	VOID PrepareData (const CWorkspace*);
	VOID SubmitGrouping (CWorkspace* pWorkSpace);

	ListProcessWindow GetProcessWindows ();

	ListProcessHandle GetProcessHandles ();

	static BOOL InitWorkSpaceCtlWndClass ();
private:
	static LRESULT CALLBACK WorkSpaceCtlWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND m_hwndWorkSpaceCtl;
	BOOL m_IsActive;

	ListProcessWindow m_listProcessWindow;// Stores the window handle and its process handle
	ListProcessHandle m_listProcessHandle;// If a handle is in this list, that means it is contrlled, all its windows will be added to this workspace
};
