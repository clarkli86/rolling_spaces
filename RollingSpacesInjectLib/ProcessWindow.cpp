#include "ProcessWindow.h"

CProcessWindow::CProcessWindow(void)
{
}

CProcessWindow::CProcessWindow(HANDLE processID, HWND hwndWnd) : m_ProcessID (processID), m_hwndWnd (hwndWnd)
{
}

CProcessWindow::~CProcessWindow(void)
{
}

HANDLE CProcessWindow::GetProcessID ()
{
	return m_ProcessID;
}
HWND   CProcessWindow::GetWindowHandle ()
{
	return m_hwndWnd;
}


BOOL CProcessWindow::operator == (const CProcessWindow& ref) const
{
	return m_ProcessID == ref.m_ProcessID &&
		m_hwndWnd == ref.m_hwndWnd;
}