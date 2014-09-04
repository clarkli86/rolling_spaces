#include "stdafx.h"
#include "UserMessages.h"

UINT WM_REGISTER_WINDOW = 0;
UINT WM_UNREGISTER_WINDOW = 0;
UINT WM_WORKSPACE_ACTIVATE = 0;
UINT WM_WORKSPACE_DEACTIVATE = 0;
UINT WM_WORKSPACE_DRAG_IN = 0;

VOID RegisterUserMessages ()
{
	WM_REGISTER_WINDOW = RegisterWindowMessage (WM_REGISTER_WINDOW_TEXT);
	WM_UNREGISTER_WINDOW = RegisterWindowMessage (WM_UNREGISTER_WINDOW_TEXT);
	WM_WORKSPACE_ACTIVATE = RegisterWindowMessage (WM_WORKSPACE_ACTIVATE_TEXT);
	WM_WORKSPACE_DEACTIVATE = RegisterWindowMessage (WM_WORKSPACE_DEACTIVATE_TEXT);
	WM_WORKSPACE_DRAG_IN = RegisterWindowMessage (WM_WORKSPACE_DRAG_IN_TEXT);
}