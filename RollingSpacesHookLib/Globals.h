#ifndef GLOBALS_H
#define GLOBALS_H

#include "../RollingSpacesHookLib/RollingSpacesHookLib.h"
#include <list>
#include <map>

#define ROLLINGSPACESHOOKLIBNAME TEXT("RollingSpacesHookLib.dll")
#define GETPREVMOUSEACTIONPROC   "GetPrevMouseAction"
#define SETPREVMOUSEACTIONPROC   "SetPrevMouseAction"

extern HMODULE gCurrentModule;

extern HMODULE GetCurrentModuleHandle ();

extern void SetCurrentModuleHandle (HMODULE hModule);

#define MAX_HORIZONTAL_WORKSPACE_COUNT 10
#define MAX_VERTICAL_WORKSPACE_COUNT   10

extern UINT g_rollingSpacesAreaWidth;
//TODO: test, it will be read out from file in the future
extern UINT g_rowCount;
extern UINT g_columnCount;


// Stores the hwnd for each workspace
extern std::map <HWND, std::list<HWND> > gHwndMapList;

/////////////////////////////////////////////////////////
extern MouseActionEnum GetPrevMouseActionStub ();
extern BOOL SetPrevMouseActionStub (MouseActionEnum);
#endif //GLOBALS_H