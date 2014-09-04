#include "HotKey.h"
#include "../common/includes/Utils.h"

INT CHotKey::s_identifier = 0;

CHotKey::CHotKey(void)
{
}

CHotKey::~CHotKey(void)
{
}

INT CHotKey::RegisterHotKey (HWND hWnd, UINT fsModifiers, INT vk)
{
	if (::RegisterHotKey (hWnd, ++ s_identifier, fsModifiers, vk))
		return s_identifier;		
	else 
		return -1;
}

BOOL CHotKey::UnregisterHotKey (HWND hWnd, INT id) const
{	
	BOOL bRet = ::UnregisterHotKey (hWnd, id);
		
	if (!bRet)
		TraceLastError (TEXT ("Unregister hot key"));

	return bRet;
}