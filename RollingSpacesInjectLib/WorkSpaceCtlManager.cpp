#include "StdAfx.h"
#include "WorkSpaceCtlManager.h"
#include "WorkSpaceCtl.h"
#include "../common/includes/Utils.h"
#include "RollingSpacesInjectLib.h"
#include "Config.h"
#include "Globals.h"

CWorkspaceCtlManager::CWorkspaceCtlManager(void)
{	
	m_WorkSpaces.resize (4);	
	m_WorkSpaces[0].Activate ();
}

CWorkspaceCtlManager::~CWorkspaceCtlManager(void)
{
}

CWorkspaceCtl* CWorkspaceCtlManager::FromHandle (const HWND hwnd)
{
	_ASSERT (!m_WorkSpaces.empty ());
	
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (begin->GetSafeHwnd () == hwnd)
			return static_cast <CWorkspaceCtl*> (&(*begin));	
	}

	return NULL;
}

CWorkspaceCtl* CWorkspaceCtlManager::GetWorkSpaceCtlWnd (const UINT index)
{
	return &m_WorkSpaces[index];
}

void CWorkspaceCtlManager::ShowWorkSpace (const HWND hwnd, UINT cmd)
{

}

BOOL CWorkspaceCtlManager::IsWorkSpaceActive (const HWND hwnd)
{
	CWorkspaceCtl* pWorkSpace = FromHandle (hwnd);
	_ASSERT (NULL != pWorkSpace);

	return pWorkSpace->IsActive ();
}

void CWorkspaceCtlManager::ActivateWorkSpace (const HWND hwndWorkSpace)
{
	CWorkspaceCtl* pWorkSpace = FromHandle (hwndWorkSpace);

	if (NULL == hwndWorkSpace)
		return;

	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	for (; begin != end; ++ begin) {
		if (begin->GetSafeHwnd () != hwndWorkSpace)
			begin->Deactivate ();
	}

	pWorkSpace->Activate ();
}

BOOL CWorkspaceCtlManager::CreateWorkSpaces (HWND parent)
{
	_ASSERT (NULL != parent);

	// Create child work spaces	
	UINT rowCount = g_RollingSpaces.GetConfig ()->GetRowCount ();
	UINT columnCount = g_RollingSpaces.GetConfig ()->GetColumnCount ();
	RECT rect;
	GetWindowRect (parent, &rect);
	UINT spaceWidth = (rect.right - rect.left) / columnCount;
	UINT spaceHeight = (rect.bottom - rect.top) / rowCount;

	if (CWorkspaceCtl::InitWorkSpaceCtlWndClass ()) {		
		for (size_t i = 0; i < rowCount; i ++) {
			for (size_t j = 0; j < columnCount; j ++) {
				HWND hwndWorkSpace = CreateWindow (WORKSPACECTLWNDCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
					spaceWidth * j, spaceHeight * i,
					spaceWidth, spaceHeight,
					parent, NULL, GetCurrentModuleHandle (), NULL);				

				_ASSERT (NULL != hwndWorkSpace);

				m_WorkSpaces[i * columnCount + j].SetHwnd (hwndWorkSpace);
			}
		}
		return TRUE;
	} else {
		TraceLastError (TEXT ("Init work space child window class"));
		return FALSE;
	}
}

BOOL CWorkspaceCtlManager::IsPointInWorkSpace (const POINT& pt)
{
	_ASSERT (!m_WorkSpaces.empty ());
	
	VecWorkSpace::iterator begin (m_WorkSpaces.begin ()), end (m_WorkSpaces.end ());

	RECT rect;
	for (; begin != end; ++ begin) {
		GetWindowRect (begin->GetSafeHwnd (), &rect);

		if (PtInRect (&rect, pt))
			return TRUE;
	}

	return FALSE;
}

VOID CWorkspaceCtlManager::PrepareData ()
{
	size_t workSpaceCount = g_RollingSpaces.GetConfig ()->GetRowCount () * g_RollingSpaces.GetConfig ()->GetColumnCount ();

	for (size_t i = 0; i < workSpaceCount; ++ i) {
		m_WorkSpaces[i].PrepareData (g_RollingSpaces.GetWorkSpace (i));
	}
}

VOID CWorkspaceCtlManager::SubmitGrouping ()
{
	size_t workSpaceCount = g_RollingSpaces.GetConfig ()->GetRowCount () * g_RollingSpaces.GetConfig ()->GetColumnCount ();

	for (size_t i = 0; i < workSpaceCount; ++ i) {
		m_WorkSpaces[i].SubmitGrouping (g_RollingSpaces.GetWorkSpace (i));
	}
}

CWorkspaceCtl* CWorkspaceCtlManager::GetActiveWorkSpace ()
{
	INT indx = GetActiveWorkSpaceIndex ();
	if (-1 != indx)
		return &m_WorkSpaces[indx];
	else
		return NULL;
}

INT CWorkspaceCtlManager::GetActiveWorkSpaceIndex ()
{
	for (size_t i = 0; i < m_WorkSpaces.size (); ++ i) {
		if (m_WorkSpaces[i].IsActive ())
			return i;
	}

	return -1;
}