#pragma once
/*
 * Purpose: This class manages the workspace control windows in the manager dialog.
 */
#include <windows.h>
#include <vector>
#include "WorkSpaceCtl.h"

class CWorkspaceCtlManager
{
	typedef std::vector <CWorkspaceCtl> VecWorkSpace;

public:
	CWorkspaceCtlManager(void);
	~CWorkspaceCtlManager(void);

	CWorkspaceCtl* FromHandle (const HWND);

	CWorkspaceCtl* GetWorkSpaceCtlWnd (const UINT);
	
	void ShowWorkSpace (const HWND, UINT cmd);

	void ActivateWorkSpace (const HWND);

	BOOL IsWorkSpaceActive (const HWND);

	BOOL CreateWorkSpaces (HWND parent);

	BOOL IsPointInWorkSpace (const POINT&);//requires screen coordinates 

	VOID PrepareData (); // It should be called when the manager dialog is initialized.

	VOID SubmitGrouping (); // It should be called when OK button is clicked

	CWorkspaceCtl* GetActiveWorkSpace ();

	INT GetActiveWorkSpaceIndex ();
private:
	VecWorkSpace  m_WorkSpaces;	
};
