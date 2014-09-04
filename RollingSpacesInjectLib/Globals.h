#ifndef GLOBALS_H
#define GLOBALS_H

extern HMODULE gCurrentModule;

extern HMODULE GetCurrentModuleHandle ();

extern void SetCurrentModuleHandle (HMODULE hModule);

#define MAX_HORIZONTAL_WORKSPACE_COUNT 10
#define MAX_VERTICAL_WORKSPACE_COUNT   10

#define EVENT_WAIT_TIME				   3000

#define BUFFER_SIZE					   (512L)
/////////////////////////////////////////////////////////
#endif //GLOBALS_H