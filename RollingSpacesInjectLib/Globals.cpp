#include "stdafx.h"
#include "Globals.h"
#include "../common/includes/Utils.h"

/* --------------------Global Variables ------------------------*/
// Global variable module handle
HMODULE gCurrentModule;

/* --------------------Global Variables ------------------------*/

extern HINSTANCE GetCurrentModuleHandle () 
{
	return gCurrentModule;
}

extern void SetCurrentModuleHandle (HMODULE hModule)
{
	_ASSERTE (NULL != hModule);
	gCurrentModule = hModule;
}