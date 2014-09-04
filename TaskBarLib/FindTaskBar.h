#ifndef FIND_TASK_BAR_H
#define FIND_TASK_BAR_H
#include <windef.h>

extern HWND GetHandleOfQuickLaunch (HINSTANCE hInstance, UINT QuickLaunchClassStringID, UINT QuickLaunchTitleStringID, UINT TrayClassStringID);
extern HWND GetHandleOfRunningApps (HINSTANCE hInstance, UINT RunAppsClassStringID, UINT RunAppsTitleStringID, UINT TrayClassStringID);
extern HWND GetHandleOfRollingSpaces (HINSTANCE hInstance, UINT RollingSpacesClassID, UINT TrayClassStringID);
extern HWND GetHandleOfTrayNotifyWnd (HINSTANCE hInstance, UINT TrayNotifyWndClassID, UINT TrayClassStringID);
extern int GetBandIndex (HWND hwndRebar, HWND hwndBand);
#endif //FIND_TASK_BAR_H