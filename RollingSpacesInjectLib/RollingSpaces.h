#pragma once
#include <windows.h>
#include <vector>
#include <list>
#include "WorkSpace.h"
#include "Config.h"

class CHotKey;

class CRollingSpaces
{
	typedef std::vector <CWorkspace> VecWorkSpace;

public:
	CRollingSpaces(void);
	~CRollingSpaces(void);

	HWND CreateRollingSpacesWindow ();

	CWorkspace* FromHandle (const HWND);

	CWorkspace* GetWorkSpace (const UINT);

	INT GetActiveWorkSpaceIndex ();

	CWorkspace* GetActiveWorkSpace ();

	CConfig* GetConfig ();
	
	VOID ActivateWorkSpace (const HWND);

	BOOL IsWorkSpaceActive (const HWND);

	BOOL AddWindowToWorkSpace (const HWND hwndWorkSpace, const HWND hwndDragIn);

	VOID RegisterWindow (HWND);

	VOID UnregisterWindow (HWND);

	VOID InitWorkSpaces ();

	HWND GetSafeHwnd () const;

	VOID RegisterHotKey ();
	VOID UnregisterHotKey ();

	VOID EjectFromExplorer ();
private:
	static LRESULT CALLBACK RollingSpacesWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	

	VOID ProcessHotKey (const INT id) const;
	LRESULT ProcessMenuCmd (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL CallConsoleToEject ();
	
	VOID RemoveDuplicatedWindow (const VecWorkSpace::iterator);
	
	CConfig m_config;

	VecWorkSpace  m_WorkSpaces;	
	HWND m_hwndRollingSpaces;

	HWND m_hwndRebar;
	INT  m_nBandIndex;

	CHotKey *m_pHotKey;
};
