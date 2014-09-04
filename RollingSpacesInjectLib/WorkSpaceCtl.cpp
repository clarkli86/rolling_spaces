#include "StdAfx.h"
#include <algorithm>
#include "Resource.h"
#include "WorkSpaceCtl.h"
#include "../common/includes/Utils.h"
#include "Globals.h"
#include "SpaceManagerDlg.h"
#include "RollingSpacesInjectLib.h"

CWorkspaceCtl::CWorkspaceCtl() : m_hwndWorkSpaceCtl (NULL), m_IsActive (FALSE)
{
}

CWorkspaceCtl::CWorkspaceCtl(HWND hwnd) : m_hwndWorkSpaceCtl (hwnd), m_IsActive (FALSE)
{
}

CWorkspaceCtl::~CWorkspaceCtl(void)
{
}

BOOL CWorkspaceCtl::InitWorkSpaceCtlWndClass ()
{

	WNDCLASS wc = {0};

	wc.hInstance = GetCurrentModuleHandle ();
	//TODO: Move to globals
	wc.lpszClassName = WORKSPACECTLWNDCLASSNAME;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);	
	wc.lpfnWndProc = WorkSpaceCtlWndProc;

	if (0 == RegisterClass (&wc) && ERROR_CLASS_ALREADY_EXISTS != GetLastError ()) {
		TraceLastError (TEXT ("Register work space child window class"));
		return FALSE;
	} else
		return TRUE;

}

/* Common Window Procedure for WorkSpace windows. */
LRESULT CALLBACK CWorkspaceCtl::WorkSpaceCtlWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg) 
	{ 
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint (hwnd, &ps);
 			
			const UINT PEN_WIDTH = 2;
			LOGBRUSH logBrush = {BS_NULL, NULL, NULL};
			HBRUSH nullBrush = CreateBrushIndirect (&logBrush);
			HBRUSH oldBrush = (HBRUSH)SelectObject (hdc, nullBrush);
			RECT rect = {0};
			GetClientRect (hwnd, &rect);
			Rectangle (hdc, rect.left, rect.top, rect.right, rect.bottom);

			if (g_SpaceManagerDlg.m_WorkSpaceManager.IsWorkSpaceActive (hwnd)) {
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
			g_SpaceManagerDlg.m_WorkSpaceManager.ActivateWorkSpace (hwnd);		
		}
		break;	
	case WM_DESTROY:
		{
			//PostQuitMessage (0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

BOOL CWorkspaceCtl::IsActive ()
{
	return m_IsActive;
}

VOID CWorkspaceCtl::Activate ()
{
	if (!m_IsActive) {
		m_IsActive = TRUE;

		RECT rect;
		GetClientRect (GetSafeHwnd (), &rect);
		InvalidateRect (GetSafeHwnd (), &rect, TRUE);
	}

	// Update the processes and windows in listview
	g_SpaceManagerDlg.PrepareItems ();
}

VOID CWorkspaceCtl::Deactivate ()
{
	if (m_IsActive) {
		m_IsActive = FALSE;

		RECT rect;
		GetClientRect (GetSafeHwnd (), &rect);
		InvalidateRect (GetSafeHwnd (), &rect, TRUE);
	}
}

HWND CWorkspaceCtl::GetSafeHwnd(void)
{
	return m_hwndWorkSpaceCtl;
}

VOID CWorkspaceCtl::SetHwnd (HWND hwnd)
{
	m_hwndWorkSpaceCtl = hwnd;
}

VOID CWorkspaceCtl::PrepareData (const CWorkspace* pWorkSpace)
{
	_ASSERT (NULL != pWorkSpace);

	m_listProcessWindow = pWorkSpace->GetProcessWindows ();
	m_listProcessHandle = pWorkSpace->GetProcessHandles ();
}

VOID CWorkspaceCtl::SubmitGrouping (CWorkspace* pWorkSpace)
{
	_ASSERT (NULL != pWorkSpace);

	pWorkSpace->SetProcessWindows (m_listProcessWindow);
	pWorkSpace->SetProcessHandles (m_listProcessHandle);

	BOOL showWindow = pWorkSpace->IsActive ();
	ListProcessWindow::iterator begin (m_listProcessWindow.begin ()), end (m_listProcessWindow.end ());
	for (; begin != end; ++ begin)
		ShowWindow (begin->GetWindowHandle(), showWindow ? SW_SHOW : SW_HIDE);
}

ListProcessWindow CWorkspaceCtl::GetProcessWindows ()
{
	return m_listProcessWindow;
}

ListProcessHandle CWorkspaceCtl::GetProcessHandles ()
{
	return m_listProcessHandle;
}

VOID CWorkspaceCtl::AddProcessHandles (const ListProcessHandle& processHandles)
{
}

VOID CWorkspaceCtl::AddProcessWindows (const ListProcessWindow& processWindows)
{	
	ListProcessWindow::const_iterator begin (processWindows.begin ()), end (processWindows.end ());

	for (; begin != end; ++ begin)
		m_listProcessWindow.push_back (*begin);
}

VOID CWorkspaceCtl::RemoveProcessWindows (const ListProcessWindow& processWindows)
{
	ListProcessWindow::const_iterator begin (processWindows.begin ()), end (processWindows.end ());

	for (; begin != end; ++ begin)
		m_listProcessWindow.erase (std::remove (m_listProcessWindow.begin (), m_listProcessWindow.end (), *begin), m_listProcessWindow.end ());
}