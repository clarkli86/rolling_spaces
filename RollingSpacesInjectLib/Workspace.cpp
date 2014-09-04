#include "stdafx.h"
#include "resource.h"
#include "WorkSpace.h"
#include "Globals.h"
#include "UserMessages.h"
#include "RollingSpacesInjectLib.h"
#include "../common/includes/utils.h"
#include "SpaceManagerDlg.h"
#include "../TaskBarLib/FindTaskBar.h"
#include "HookDllStub.h"
#include <algorithm>
#define PEN_WIDTH 2

// Static Context Menu Handle
static HMENU g_hContextMenu = NULL;

CWorkspace::CWorkspace() : m_hwndWorkSpace (NULL), m_IsActive (FALSE)
{
	// TODO
	//testInit ();
}

CWorkspace::CWorkspace(HWND hwndWorkSpace) : m_hwndWorkSpace (hwndWorkSpace), m_IsActive (FALSE)
{
	// TODO
	//testInit ();
}

CWorkspace::~CWorkspace(void)
{
}

VOID CWorkspace::testInit ()
{
	static INT windowHandle = 0;

	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)1, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)1, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)2, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)2, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)3, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)3, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)4, (HWND)++ windowHandle));
	m_listProcessWindow.push_back (CProcessWindow ((HANDLE)4, (HWND)++ windowHandle));

	static INT processNum = 0;
	TCHAR buf[100];
	wsprintf (buf, TEXT ("Process%d"), ++ processNum);
	m_listProcessHandle.push_back (CProcess ((HANDLE)processNum, buf));
	wsprintf (buf, TEXT ("Process%d"), ++ processNum);
	m_listProcessHandle.push_back (CProcess ((HANDLE)processNum, buf));
	wsprintf (buf, TEXT ("Process%d"), ++ processNum);
	m_listProcessHandle.push_back (CProcess ((HANDLE)processNum, buf));
	wsprintf (buf, TEXT ("Process%d"), ++ processNum);
	m_listProcessHandle.push_back (CProcess ((HANDLE)processNum, buf));
}

/* Common Window Procedure for WorkSpace windows. */
LRESULT CALLBACK CWorkspace::WorkSpaceWindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg) 
	{ 
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint (hwnd, &ps);
 			
			LOGBRUSH logBrush = {BS_NULL, NULL, NULL};
			HBRUSH nullBrush = CreateBrushIndirect (&logBrush);
			HBRUSH oldBrush = (HBRUSH)SelectObject (hdc, nullBrush);
			RECT rect = {0};
			GetClientRect (hwnd, &rect);
			Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);

			if (g_RollingSpaces.IsWorkSpaceActive (hwnd)) {
				HPEN pen = CreatePen (PS_DASH, PEN_WIDTH, RGB(0,0,0));
				HPEN oldPen = (HPEN)SelectObject (hdc, pen);

				INT prevDrawMode = SetROP2 (hdc, R2_NOT);

				Rectangle (hdc,
					rect.left + PEN_WIDTH, rect.top + PEN_WIDTH, 
					rect.right - PEN_WIDTH, rect.bottom - PEN_WIDTH);

				SetROP2 (hdc, prevDrawMode);
				SelectObject (hdc, oldPen);
				DeleteObject (pen);
			}

			SelectObject (hdc, oldBrush);
			DeleteObject (nullBrush);
			EndPaint (hwnd, &ps);
		}
		break;
	case WM_LBUTTONDOWN:
		{				
			// One of the child windows was activated
			g_RollingSpaces.ActivateWorkSpace (hwnd);					
		}
		break;	
	case WM_CONTEXTMENU:
		{
			//todo: destroy menu
			do {
				if (NULL == g_hContextMenu)
					g_hContextMenu = LoadMenu (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDR_MAIN_MENU));
				if (NULL == g_hContextMenu) {
					TraceLastError (TEXT ("Loading Main Menu"));
					break;
				}
				HMENU hSubMenu = GetSubMenu (g_hContextMenu, 0);
				if (NULL == g_hContextMenu) {
					TraceLastError (TEXT ("Loading Sub Menu 0"));
					break;
				}

				POINT pt;
				pt.x = LOWORD(lParam); 
				pt.y = HIWORD(lParam); 

				TrackPopupMenuEx (hSubMenu, TPM_VERNEGANIMATION | TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, 
					pt.x, pt.y, g_RollingSpaces.GetSafeHwnd (), NULL);
			} while (FALSE);
		}
		break;
	case WM_DESTROY:
		{
			if (NULL == g_hContextMenu) {
				DestroyMenu (g_hContextMenu);
				g_hContextMenu = NULL;
			}

			PostQuitMessage (0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

BOOL CWorkspace::InitWorkSpaceWindowClass (VOID)
{	
	WNDCLASS wc = {0};

	wc.hInstance = GetCurrentModuleHandle ();
	wc.lpszClassName = WORKSPACEWINDOWCLASSNAME;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);

	HWND hwndRunApps = GetHandleOfRunningApps (GetCurrentModuleHandle (), IDS_RUNAPPS_CLASS, IDS_RUNAPPS_TITLE, IDS_TRAYCLASS);
	HDC hdc = GetDC (hwndRunApps);
	COLORREF color = GetPixel (hdc, 0, 0);
	LOGBRUSH brush;
	brush.lbStyle = BS_SOLID;
	brush.lbColor = color;
	// TODO: destroy this brush
	HBRUSH hBrush = CreateBrushIndirect (&brush);
	ReleaseDC (hwndRunApps, hdc);

	wc.hbrBackground = hBrush;	
	wc.lpfnWndProc = WorkSpaceWindowProc;
	wc.cbWndExtra = 0;

	if (0 == RegisterClass (&wc) && ERROR_CLASS_ALREADY_EXISTS != GetLastError ()) {
		TraceLastError (TEXT ("Register work space child window class"));
	}

	return TRUE;
}

VOID CWorkspace::RegisterWindow (HWND hwndNew)
{
	if (m_listProcessWindow.end () == std::find (m_listProcessWindow.begin (), m_listProcessWindow.end (), CProcessWindow (0, hwndNew)))
		m_listProcessWindow.push_back (CProcessWindow (0, hwndNew));
}

VOID CWorkspace::UnregisterWindow (HWND hwnd)
{
	m_listProcessWindow.erase (std::remove (m_listProcessWindow.begin (), m_listProcessWindow.end (), CProcessWindow (0, hwnd)),
		m_listProcessWindow.end ());
}

BOOL CWorkspace::IsActive ()
{
	return m_IsActive;
}

VOID CWorkspace::Activate ()
{
	if (m_IsActive)
		return;
	
	m_IsActive = TRUE;

	RECT rect;
	GetClientRect (GetSafeHwnd (), &rect);
	InvalidateRect (GetSafeHwnd (), &rect, TRUE);

	// Setup the hwnd queue
	/*HwndQueueClearStub ();
	ListProcessWindow windows = GetProcessWindows ();
	ListProcessWindow::iterator beginHwnd (windows.begin ()), endHwnd (windows.end ());
	for (; beginHwnd != endHwnd; ++ beginHwnd) {
		HwndQueuePushBackStub (beginHwnd->GetWindowHandle ());
	}*/
	/*
	 * 1. Acquire Global Flag Protection 
	 * 2. Set the Global Flag to true
	 * 3. Release Global Flag
	 * 4. Show Window
	 * 5. Acquire Global Flag Protection 
	 * 6. Set the Global Flag to false
	 * 7. Release Global Flag
	 */
	/*SetGlobalFlagStub (TRUE);*/

	ListProcessWindow::iterator begin (m_listProcessWindow.begin ()), end (m_listProcessWindow.end ());
	for (; begin != end; ++ begin) {
		ShowWindow (begin->GetWindowHandle (), SW_SHOW);
	}

	SetForegroundWindow (GetPreviousForegroundWindow ());
	/*SetGlobalFlagStub (FALSE);*/
}

VOID CWorkspace::Deactivate ()
{
	// To hide the windows moved into inactive workspaces by Space Manager Dialog
	if (!m_IsActive)
		return;

	m_IsActive = FALSE;

	RECT rect;
	GetClientRect (GetSafeHwnd (), &rect);
	InvalidateRect (GetSafeHwnd (), &rect, TRUE);	

	// Setup the hwnd queue
	/*HwndQueueClearStub ();
	ListProcessWindow windows = GetProcessWindows ();
	ListProcessWindow::iterator beginHwnd (windows.begin ()), endHwnd (windows.end ());
	for (; beginHwnd != endHwnd; ++ beginHwnd) {
		HwndQueuePushBackStub (beginHwnd->GetWindowHandle ());
	}	

	SetGlobalFlagStub (TRUE);*/

	CaptureAndHideVisibleWindows ();

	/*SetGlobalFlagStub (FALSE);*/
}

HWND CWorkspace::GetSafeHwnd(void)
{
	return m_hwndWorkSpace;
}

VOID CWorkspace::SetHwnd (HWND hwnd)
{
	m_hwndWorkSpace = hwnd;
}

ListProcessWindow CWorkspace::GetProcessWindows () const
{
	return m_listProcessWindow;
}

ListProcessHandle CWorkspace::GetProcessHandles () const
{
	return m_listProcessHandle;
}

VOID CWorkspace::SetProcessWindows (const ListProcessWindow& processWindows)
{
	m_listProcessWindow = processWindows;
}

VOID CWorkspace::SetProcessHandles (const ListProcessHandle& processHandles)
{
	m_listProcessHandle = processHandles;
}

BOOL CWorkspace::HwndExists (const HWND hwnd) const
{
	if (m_listProcessWindow.end () == std::find (m_listProcessWindow.begin (), m_listProcessWindow.end (), CProcessWindow (0, hwnd)))
		return FALSE;
	else
		return TRUE;
}

// Some windows such as maxthon or message may be implemented specially that they can't be captured by RollingSpaces, so 
// we have to take a workaround here.
ListProcessWindow CWorkspace::CaptureVisibleWindows () const
{
	// Add the left windows into current workspace
	// Enumerate the windows those could be displayed in taskbar and move them into the active workspace
	ListProcessWindow processWindows;

	HWND hwndFirst = FindWindow (NULL, NULL);
	while (hwndFirst) {
		if (IsWindowVisible (hwndFirst)) {				
			long windowStyle = GetWindowLong (hwndFirst, GWL_STYLE);
			if (WS_SYSMENU == (windowStyle & WS_SYSMENU)) {					
				processWindows.push_back (CProcessWindow (0, hwndFirst));
			}
		}

		hwndFirst = GetNextWindow (hwndFirst, GW_HWNDNEXT);
	}	

	return processWindows;
}

// Save the hwnd of foreground window
VOID CWorkspace::SaveForegroundWindow ()
{
	m_hwndForegroundWindow = GetForegroundWindow ();
}

// returns the hwnd of foreground window
HWND CWorkspace::GetPreviousForegroundWindow () const
{
	return m_hwndForegroundWindow;
}

//  before he space manager dlg is called.
VOID CWorkspace::CaptureRemainingWindows ()
{
	m_listProcessWindow = CaptureVisibleWindows ();
	SaveForegroundWindow ();
}

// It is supposed to be called before this workspace is deactived.
VOID CWorkspace::CaptureAndHideVisibleWindows ()
{
	m_listProcessWindow = CaptureVisibleWindows ();
	SaveForegroundWindow ();

	ListProcessWindow::iterator begin (m_listProcessWindow.begin ()), end (m_listProcessWindow.end ());

	for (; begin != end; ++ begin) {
		ShowWindow (begin->GetWindowHandle (), SW_HIDE);
	}
}

VOID CWorkspace::ReleaseWindows ()
{
	ListProcessWindow::iterator begin = m_listProcessWindow.begin (), end = m_listProcessWindow.end ();

	for (; begin != end; ++ begin) {
		ShowWindow (begin->GetWindowHandle (), SW_SHOW);
	}
}