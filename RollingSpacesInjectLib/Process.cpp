#include "Process.h"

CProcess::CProcess(void) : m_IsMonopolized (FALSE)
{
}

CProcess::CProcess(HANDLE processID, const std::wstring &processName) 
: m_ProcessID (processID), m_ProcessName (processName), m_IsMonopolized (FALSE)
{
}

CProcess::~CProcess(void)
{
}

HANDLE CProcess::GetProcessID ()
{
	return m_ProcessID;
}

std::wstring CProcess::GetProcessName ()
{
	return m_ProcessName;
}