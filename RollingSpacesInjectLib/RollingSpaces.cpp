#include "stdafx.h"
#include "RollingSpaces.h"
#include "Globals.h"
#include "../common/includes/Utils.h"
#include "../TaskBarLib/FindTaskBar.h"
#include "RollingSpacesInjectLib.h"
#include "UserMessages.h"
#include "DirectionalHotKey.h"

CRollingSpaces::CRollingSpaces(void) : m_hwndRebar (NULL), m_nBandIndex (-1)
{
	//TODO: resize will only use copy constructor, we have to construct objects manually to create test data
	//m_WorkSpaces.resize (m_config.GetRowCount () * m_config.GetColumnCount ());
	for (size_t i = 0; i < g_RollingSpaces.GetConfig ()->GetColumnCount () * g_RollingSpaces.GetConfig ()->GetRowCount (); ++ i) {
		m_WorkSpaces.push_back (CWorkspace ());
	}
	m_WorkSpaces[0].Activate ();//TODO	
}

CRollingSpaces::~CRollingSpaces(void)
{	
}

HWND CRollingSpaces::CreateRollingSpacesWindow ()
{
	HWND hwndRollingSpaces = NULL;
	TCHAR rollingSpacesWndClassName[] = TEXT("RollingSpacesWindow32");
	// Get the handle of current module
	HINSTANCE hInstance = GetCurrentModuleHandle ();	

	HWND hwndRunApps = GetHandleOfRunningApps (GetCurrentModuleHandle (), IDS_RUNAPPS_CLASS, IDS_RUNAPPS_TITLE, IDS_TRAYCLASS);

	if (NULL == hwndRunApps) {
		TraceLastError (TEXT("Find Running Applications Hwnd"));
		return NULL;
	}
	
	// Get the handle of the parent Rebar	
	m_hwndRebar = GetParent (GetParent (hwndRunApps));
	if (NULL == m_hwndRebar) {
		TraceLastError (TEXT("Find Rebar"));
		return NULL;
	}

	WNDCLASS wcRollingSpaces = {0};
	wcRollingSpaces.hInstance = hInstance;
	wcRollingSpaces.lpszClassName = rollingSpacesWndClassName;
	wcRollingSpaces.style = CS_HREDRAW | CS_VREDRAW;
	wcRollingSpaces.lpfnWndProc = RollingSpacesWndProc;
	wcRollingSpaces.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcRollingSpaces.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
		
	/* Register Rolling Spaces Window Class */
	if (0 == RegisterClass (&wcRollingSpaces) && ERROR_CLASS_ALREADY_EXISTS != GetLastError ()) {
		TraceLastError (TEXT("Registering Rolling Spaces Window Class"));		
	}

	hwndRollingSpaces = CreateWindow (rollingSpacesWndClassName, NULL, WS_CHILD | WS_VISIBLE, 
		0, 0, 0, 0,
		m_hwndRebar, NULL, hInstance, NULL);

	if (NULL == hwndRollingSpaces) {
		TraceLastError (TEXT("Creating Rolling Spaces Window"));
		return NULL;
	}

	REBARBANDINFO bandInfo = {0};
	bandInfo.cbSize = sizeof (REBARBANDINFO);

	// Get the index of Running Applications band
	m_nBandIndex = GetBandIndex (m_hwndRebar, GetParent (hwndRunApps));
	if (-1 == m_nBandIndex) {
		TraceLastError (TEXT ("Get the index of Running Applications band"));
		return NULL;
	}
	// Get the rebar information of Running Applications band and use it to set the new band
	LRESULT lRes = SendMessage (m_hwndRebar, RB_GETBANDINFO, m_nBandIndex, (LPARAM)&bandInfo);
	if (0 == lRes) {
		TraceLastError (TEXT ("Get the rebar info of Running Applications band"));
		return NULL;
	}
		
	bandInfo.fMask |= RBBIM_CHILD | RBBIM_SIZE;
	bandInfo.hwndChild = hwndRollingSpaces;
	bandInfo.cx = m_config.GetRollingSpacesAreaWidth ();

	/* Insert a band before Running Applications band */
	lRes = SendMessage (m_hwndRebar, RB_INSERTBAND, m_nBandIndex, (LPARAM)&bandInfo);
	if (0 == lRes) {
		TraceLastError (TEXT ("Inserting band to Rebar"));
		return NULL;
	} else {
		OutputDebugString (TEXT ("Inserting band succeed!"));
	}

	// Create child work spaces	
	UINT rowCount = m_config.GetRowCount ();
	UINT columnCount = m_config.GetColumnCount ();
	RECT rect;
	GetClientRect (hwndRollingSpaces, &rect);
	UINT spaceWidth = (rect.right - rect.left) / columnCount;
	UINT spaceHeight = (rect.bottom - rect.top) / rowCount;

	if (CWorkspace::InitWorkSpaceWindowClass ()) {		
		for (size_t i = 0; i < rowCount; i ++) {
			for (size_t j = 0; j < columnCount; j ++) {
				HWND hwndWorkSpace = CreateWindow (WORKSPACEWINDOWCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
					spaceWidth * j, spaceHeight * i,
					spaceWidth, spaceHeight,
					hwndRollingSpaces, NULL, hInstance, NULL);				

				_ASSERT (NULL != hwndWorkSpace);

				m_WorkSpaces[i * columnCount + j].SetHwnd (hwndWorkSpace);
			}
		}		
	} else {
		TraceLastError (TEXT ("Init work space child window class"));
	}
		
	// Initialize the active workspace
	InitWorkSpaces ();

	ShowWindow (hwndRollingSpaces, SW_SHOW);
	UpdateWindow (hwndRollingSpaces);

	m_hwndRollingSpaces = hwndRollingSpaces;

	// Initialize hot keys
	RegisterHotKey ();

	return hwndRollingSpaces;
}

VOID CRollingSpaces::InitWorkSpaces ()
{
	// Enumerate the windows those could be displayed in taskbar and move them into the active workspace
	ListProcessWindow processWindows;

	HWND hwndFirst = FindWindowEx (NULL, NULL, NULL, NULL);
	while (hwndFirst) {
		if (IsWindowVisible (hwndFirst)) {				
			long windowStyle = GetWindowLong (hwndFirst, GWL_STYLE);
			if (windowStyle & WS_SYSMENU) {					
				processWindows.push_back (CProcessWindow (0, hwndFirst));
			}
		}

		hwndFirst = FindWindowEx (NULL, hwndFirst, NULL, NULL);
	}

	m_WorkSpaces[0].SetProcessWindows (processWindows);
	// TODO: maybe read the settings of last time
	m_WorkSpaces[0].Activate ();
}

// Window procedure of rolling spaces window
LRESULT CALLBACK CRollingSpaces::RollingSpacesWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_SIZE:
			{
				UINT newWidth = LOWORD (lParam);
				UINT newHeight = HIWORD (lParam);

				UINT rowCount = g_RollingSpaces.GetConfig ()->GetRowCount ();
				UINT columnCount = g_RollingSpaces.GetConfig ()->GetColumnCount ();
			
				UINT spaceWidth = newWidth / columnCount;
				UINT spaceHeight = newHeight / rowCount;

				// Move the child windows
				for (size_t i = 0; i < rowCount; i ++) {
					for (size_t j = 0; j < columnCount; j ++) {
						if (0 == MoveWindow (g_RollingSpaces.GetWorkSpace (i * columnCount + j)->GetSafeHwnd (),
							spaceWidth * j, spaceHeight * i, 
							spaceWidth, spaceHeight,
							TRUE)) {
							TraceLastError (TEXT ("Move child work spaces"));
						}						
					}
				}
			}
			break;				
		//case WM_WINDOWPOSCHANGING:
		//	{
		//		// System sent this message to window without sending WM_SIZE after WM_SETTINGCHAGNE
		//		UINT rowCount = g_RollingSpaces.GetConfig ()->GetRowCount ();
		//		UINT columnCount = g_RollingSpaces.GetConfig ()->GetColumnCount ();

		//		WINDOWPOS * pWindowPos = (WINDOWPOS *)lParam;
		//		UINT spaceWidth = (pWindowPos->cx) / columnCount;
		//		UINT spaceHeight = (pWindowPos->cy) / rowCount;
		//		for (size_t i = 0; i < rowCount; i ++) {
		//			for (size_t j = 0; j < columnCount; j ++) {
		//				RECT rect;
		//				GetClientRect (g_RollingSpaces.GetWorkSpace (i * columnCount + j)->GetSafeHwnd (), &rect);
		//				InvalidateRect (g_RollingSpaces.GetWorkSpace (i * columnCount + j)->GetSafeHwnd (), &rect, TRUE);
		//			}
		//		}
		//	}
		//	break;
		case WM_HOTKEY:
			{
				g_RollingSpaces.ProcessHotKey ((int)wParam);
			}
			break;
		case WM_COMMAND:
			{
				// From menu
				if (HIWORD (wParam) == 0)
					return g_RollingSpaces.ProcessMenuCmd (hwnd, msg, wParam, lParam);
			}
			break;
		case WM_DESTROY:
			{				
				g_RollingSpaces.EjectFromExplorer ();

				PostQuitMessage (0);
			}
			break;
		default:
			{
				if (WM_REGISTER_WINDOW == msg) {
					TCHAR buf[50], buf2[100];
					GetWindowText ((HWND)wParam, buf, 50);
					swprintf_s (buf2, 100, TEXT ("Register Window: %s"), buf);
					OutputDebugString (buf2);

					g_RollingSpaces.RegisterWindow ((HWND)wParam);
				} else if (WM_UNREGISTER_WINDOW == msg) {
					TCHAR buf[50], buf2[100];
					GetWindowText ((HWND)wParam, buf, 50);
					swprintf_s (buf2, 100, TEXT ("Unregister Window: %s"), buf);
					OutputDebugString (buf2);

					g_RollingSpaces.UnregisterWindow ((HWND)wParam);
				} else
					return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			break;
	}

	return 0;
}

VOID CRollingSpaces::ProcessHotKey (const INT id) const
{
	if (m_pHotKey)
		m_pHotKey->ProcessHotKey (id);
}

CWorkspace* CRollingSpaces::FromHandle (const HWND hwnd)
{
	_ASSERT (!m_WorkSpaces.empty ());
	
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (begin->GetSafeHwnd () == hwnd)
			return static_cast <CWorkspace*> (&(*begin));	
	}

	return NULL;
}

BOOL CRollingSpaces::IsWorkSpaceActive (const HWND hwnd)
{
	CWorkspace* pWorkSpace = FromHandle (hwnd);
	_ASSERT (NULL != pWorkSpace);

	return pWorkSpace->IsActive ();
}

BOOL CRollingSpaces::AddWindowToWorkSpace (const HWND hwndWorkSpace, const HWND hwndDragIn)
{
	return TRUE;
}

CWorkspace* CRollingSpaces::GetWorkSpace (const UINT uIndex)
{
	return &m_WorkSpaces[uIndex];
}

CConfig* CRollingSpaces::GetConfig ()
{
	return &m_config;
}

// Activate the workspace and deactivate the others
void CRollingSpaces::ActivateWorkSpace (const HWND hwndWorkSpace)
{
	CWorkspace* pWorkSpace = FromHandle (hwndWorkSpace);

	if (NULL == hwndWorkSpace)
		return;

	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (begin->GetSafeHwnd () != hwndWorkSpace) {			
			begin->Deactivate ();

			RemoveDuplicatedWindow (begin);
		}
	}

	pWorkSpace->Activate ();
}

CWorkspace* CRollingSpaces::GetActiveWorkSpace ()
{
	INT indx = GetActiveWorkSpaceIndex ();
	if (-1 != indx)
		return &m_WorkSpaces[indx];
	else
		return NULL;
}

INT CRollingSpaces::GetActiveWorkSpaceIndex ()
{
	for (size_t i = 0; i < m_WorkSpaces.size (); ++ i) {
		if (m_WorkSpaces[i].IsActive ())
			return i;
	}

	return -1;
}

VOID CRollingSpaces::RegisterWindow (HWND hwnd)
{
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (begin->HwndExists (hwnd))
			return;
	}

	GetActiveWorkSpace ()->RegisterWindow (hwnd);
}

VOID CRollingSpaces::UnregisterWindow (HWND hwnd)
{
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin)
		begin->UnregisterWindow (hwnd);	
}

HWND CRollingSpaces::GetSafeHwnd () const
{
	return m_hwndRollingSpaces;
}

LRESULT CRollingSpaces::ProcessMenuCmd (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD (wParam)) {
		case ID_MAINMENU_EXIT:
				CallConsoleToEject ();
			break;
		case ID_MAINMENU_MANAGE_DLG:
			{
				// Capture the remaining windows
				GetActiveWorkSpace ()->CaptureRemainingWindows ();
				VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

				for (; begin != end; ++ begin) {
					if (&(*begin) != GetActiveWorkSpace ()) {			
						begin->Deactivate ();

						RemoveDuplicatedWindow (begin);
					}
				}

				g_SpaceManagerDlg.DoModal (GetHandleOfRollingSpaces (GetCurrentModuleHandle (), IDS_ROLLINGSPACES_CLASS, IDS_TRAYCLASS));			
			}
			break;
		default:
			return DefWindowProc (hwnd, msg, wParam, lParam);
	}
	// processed
	return 0;
}

BOOL CRollingSpaces::CallConsoleToEject ()
{
	TCHAR filePath[MAX_PATH];
	//todo: allocate more memory if the buffer is not big enough
	if (GetModuleFileName (GetCurrentModuleHandle (), filePath, MAX_PATH) >= MAX_PATH) {
		TraceLastError (TEXT ("Get Inject DLL Module Path"));
		return FALSE;
	}

	std::wstring exeName (TEXT ("RollingSpaces.exe"));
	
	std::wstring exePath (filePath);

	std::wstring::size_type lastSeperatorIndex = exePath.find_last_of ('\\');
	++ lastSeperatorIndex;//move to the next pos
	exePath.erase (lastSeperatorIndex, exePath.length () - lastSeperatorIndex);

	exePath += exeName;

	STARTUPINFO startupInfo = {0};
	startupInfo.cb = sizeof (STARTUPINFO);

	PROCESS_INFORMATION processInfo;
	TCHAR parameters[32768] = TEXT ("RollingSpaces.exe -e"); 
	if (CreateProcess (exePath.c_str (), parameters, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
		NULL, NULL, &startupInfo, &processInfo))
		return TRUE;
	else
		TraceLastError (TEXT ("Call RollingSpaces Console to eject DLL"));

	return FALSE;
}

VOID CRollingSpaces::RegisterHotKey ()
{
	if (NULL != m_pHotKey)
		delete m_pHotKey;

	m_pHotKey = new CDirectionalHotKey ();
}

VOID CRollingSpaces::UnregisterHotKey ()
{
	if (NULL != m_pHotKey)
		delete m_pHotKey;

	m_pHotKey = NULL;	
}

VOID CRollingSpaces::RemoveDuplicatedWindow (const VecWorkSpace::iterator iter)
{
	ListProcessWindow windows = iter->GetProcessWindows ();
	ListProcessWindow::iterator windowsBegin, windowsEnd (windows.end ());
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (iter != begin) {
			for (windowsBegin = windows.begin (); windowsBegin != windowsEnd; ++ windowsBegin)
				begin->UnregisterWindow (windowsBegin->GetWindowHandle ());
		}
	}
}

VOID CRollingSpaces::EjectFromExplorer ()
{
	/*
	 * 1. Unregister hot keys
	 * 2. Restore all windows		
	 * 3. Remove Rebar band
	 * Above are supposed to be done by the destructor of RollingSpace
	 */								
	g_RollingSpaces.UnregisterHotKey ();
	
	for (size_t i = 0; i < m_WorkSpaces.size (); ++ i) {
		m_WorkSpaces[i].ReleaseWindows ();
	}
	if (NULL != m_hwndRebar && -1 != m_nBandIndex)
		SendMessage (m_hwndRebar, RB_DELETEBAND, m_nBandIndex, 0);
}