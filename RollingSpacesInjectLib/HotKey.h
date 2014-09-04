#pragma once
#include <windows.h>

class CHotKey
{
public:
	CHotKey(void);
	virtual ~CHotKey(void);

	virtual VOID ProcessHotKey (const INT id) const = 0;

	INT RegisterHotKey (HWND hWnd, UINT fsModifiers, INT vk);
	BOOL UnregisterHotKey (HWND hWnd, INT id) const;
private:
	static INT s_identifier;
};
