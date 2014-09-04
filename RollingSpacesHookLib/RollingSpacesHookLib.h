#ifndef ROLLING_SPACES_HOOK_LIB_H
#define ROLLING_SPACES_HOOK_LIB_H
#include "DragInfo.h"

extern "C" MouseActionEnum _declspec (dllexport) GetPrevMouseAction ();

extern "C" VOID _declspec (dllexport) SetPrevMouseAction (MouseActionEnum action);

extern "C" BOOL _declspec (dllexport) InstallMouseHook ();

extern "C" BOOL _declspec (dllexport) UninstallMouseHook ();

#endif //ROLLING_SPACES_HOOK_LIB_H