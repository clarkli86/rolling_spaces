#pragma once
#include <windows.h>
#include <string>

class CProcess
{
public:
	CProcess(void);
	CProcess (HANDLE processID, const std::wstring &processName);
	~CProcess(void);

	HANDLE GetProcessID ();
	std::wstring GetProcessName ();
private:
	HANDLE m_ProcessID;
	BOOL   m_IsMonopolized; // specifies if the windows of this process are all controlled by the workspace
	std::wstring m_ProcessName;		
};
