#include "DirectionalHotKey.h"
#include "RollingSpacesInjectLib.h"
#include "../common/includes/Utils.h"

CDirectionalHotKey::CDirectionalHotKey(void)
{
	RegisterDirectionalKeys ();
}

CDirectionalHotKey::~CDirectionalHotKey(void)
{
}

VOID CDirectionalHotKey::RegisterDirectionalKeys ()
{
	HWND hwndRollingSpace = g_RollingSpaces.GetSafeHwnd ();
	
	ivUp = CHotKey::RegisterHotKey (hwndRollingSpace, MOD_SHIFT| MOD_ALT, VK_UP);
	if (-1 == ivUp)
		TraceLastError (TEXT ("Register VK_UP"));

	ivDown = CHotKey::RegisterHotKey (hwndRollingSpace, MOD_SHIFT | MOD_ALT, VK_DOWN);
	if (-1 == ivDown)
		TraceLastError (TEXT ("Register VK_DOWN"));

	ivLeft = CHotKey::RegisterHotKey (hwndRollingSpace, MOD_SHIFT | MOD_ALT, VK_LEFT);
	if (-1 == ivLeft)
		TraceLastError (TEXT ("Register VK_LEFT"));

	ivRight = CHotKey::RegisterHotKey (hwndRollingSpace, MOD_SHIFT | MOD_ALT, VK_RIGHT);
	if (-1 == ivRight)
		TraceLastError (TEXT ("Register VK_RIGHT"));
}

VOID CDirectionalHotKey::UnregisterDirectionalKeys ()
{
	HWND hwndRollingSpace = g_RollingSpaces.GetSafeHwnd ();
		
	if (-1 != ivUp)
		CHotKey::UnregisterHotKey (hwndRollingSpace, ivUp);

	if (-1 != ivDown)
		CHotKey::UnregisterHotKey (hwndRollingSpace, ivDown);

	if (-1 != ivLeft)
		CHotKey::UnregisterHotKey (hwndRollingSpace, ivLeft);

	if (-1 != ivRight)
		CHotKey::UnregisterHotKey (hwndRollingSpace, ivRight);
}

VOID CDirectionalHotKey::ProcessHotKey (const INT id) const
{
	HWND hwndRollingSpace = g_RollingSpaces.GetSafeHwnd ();

	UINT rowCount, columnCount;
	rowCount = g_RollingSpaces.GetConfig ()->GetRowCount ();
	columnCount = g_RollingSpaces.GetConfig ()->GetColumnCount ();
	
	INT activeWorkSpaceIndex = g_RollingSpaces.GetActiveWorkSpaceIndex ();
	if (-1 != activeWorkSpaceIndex) {
		if (ivUp == id) {
			activeWorkSpaceIndex -= columnCount;
		} else if (ivDown == id) {
			activeWorkSpaceIndex += columnCount;
		} else if (ivLeft == id && activeWorkSpaceIndex % columnCount != 0) {
			// if not the left edge
			-- activeWorkSpaceIndex;
		} else if (ivRight == id && (activeWorkSpaceIndex + 1) % columnCount != 0) {
			// if not the right edge
			++ activeWorkSpaceIndex;
		}

		if (activeWorkSpaceIndex >= 0 && activeWorkSpaceIndex < rowCount * columnCount)
			g_RollingSpaces.ActivateWorkSpace (g_RollingSpaces.GetWorkSpace (activeWorkSpaceIndex)->GetSafeHwnd ());
	}
}