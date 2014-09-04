#pragma once
#include <windows.h>

class CProcessWindow
{
public:
	CProcessWindow(void);
	CProcessWindow(HANDLE, HWND);
	~CProcessWindow(void);

	HANDLE GetProcessID ();
	HWND   GetWindowHandle ();

	BOOL operator == (const CProcessWindow& ref) const;
private:
	HANDLE m_ProcessID;
	HWND m_hwndWnd;
};
