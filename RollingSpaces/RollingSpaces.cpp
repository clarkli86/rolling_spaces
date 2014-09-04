#include "stdafx.h"
#include "../Common/includes/Utils.h"
#include "InjectLib.h"
#include "../RollingSpacesHookLib/RollingSpacesHookLib.h"
#include <Tlhelp32.h>
#include <Shellapi.h>
#include <string>
#include "../TaskBarLib/FindTaskBar.h"
#include "resource.h"

#define BUFFER_SIZE 512L

static BOOL InjectEdjectExplorer (const TCHAR *szInjectLibName, BOOL inject);
static VOID InstallDetours ();
static void CleanupRollingSpaces ();
// Function prototype of Install/Uninstall Mouse Hook
typedef BOOL (*InstallRollingSpacesHooksFunc)();
typedef BOOL (*UninstallRollingSpacesHooksFunc)();

#define ROLLINGSPACES_INSTALL_HOOKS   "InstallRollingSpacesHooks"
#define ROLLINGSPACES_UNINSTALL_HOOKS "UninstallRollingSpacesHooks"

/* 
 * Global variables
 */
HINSTANCE g_hInstance;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	g_hInstance = hInstance;
	// Default we will inject Explorer
	BOOL injectExplorer = TRUE;
	int numOfArgs = 0;
	
	// Parse the command line argument
	if (0 != _tcslen (lpCmdLine)) {
		LPWSTR *args = CommandLineToArgvW (lpCmdLine, &numOfArgs);

		if (numOfArgs >= 1) {
			if (0 == _tcsicmp (args[0], TEXT("-i"))) {
				// Inject
			} else if (0 ==_tcsicmp (args[0], TEXT("-e"))) {
				// Eject
				injectExplorer = FALSE;
			} else {
				ErrorExit (TEXT ("Wrong command line argument!"));
			}
		}
	}
	
	// Install Detours Dll
	//InstallDetours ();

	// Load the Hook Dll
	/*HMODULE hHookLib = LoadLibrary (TEXT ("RollingSpacesHookLib.dll"));
	if (NULL == hHookLib) {
		ErrorExit (TEXT ("RollingSpacesHookLib.dll not found!"));
		return -1;
	}*/

	if (injectExplorer) {
		// Install Hook
		/*InstallRollingSpacesHooksFunc installHook = (InstallRollingSpacesHooksFunc)GetProcAddress (hHookLib, ROLLINGSPACES_INSTALL_HOOKS);
		if (NULL == installHook) {
			ErrorExit (TEXT ("Get Proc Address of Install hooks in Hook Dll failed!"));
		} else {
			if (!installHook ()) {
				ErrorExit (TEXT ("Install hooks failed!"));
			}
		}	*/

		// Inject RollingSpacesInjectLib into Explorer		
		if (!InjectEdjectExplorer (TEXT ("RollingSpacesInjectLib.dll"), TRUE)) {
			ErrorExit (TEXT ("Injecting RollingSpacesInjectLib.dll"));
		}		
	} else {
		// Uninstall the Mouse Hook
		/*UninstallRollingSpacesHooksFunc uninstallHook = (UninstallRollingSpacesHooksFunc)GetProcAddress (hHookLib, ROLLINGSPACES_UNINSTALL_HOOKS);
		if (NULL == uninstallHook) {
			ErrorExit (TEXT ("Get Proc Address of Uninstall hooks in Hook Dll failed!"));
		} else {
			if (!uninstallHook ()) {
				ErrorExit (TEXT ("Uninstall hooks failed!"));
			}
		}*/

		// todo: Eject RollingSpacesInjectLib into Explorer			
		CleanupRollingSpaces ();

		if (!InjectEdjectExplorer (TEXT ("RollingSpacesInjectLib.dll"), FALSE)) {
			ErrorExit (TEXT ("Injecting RollingSpacesInjectLib.dll"));
		}
	}

	/*FreeLibrary (hHookLib);	*/
	return TRUE;
}

static BOOL InjectEdjectExplorer (const TCHAR *szInjectLibName, BOOL inject)
{
	const TCHAR szProcessName[] = TEXT("explorer.exe");

	HANDLE hProcessSnap;	
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE ) {
		OutputDebugString(TEXT ("CreateToolhelp32Snapshot (of processes)"));
		return( FALSE );
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	if( !Process32First( hProcessSnap, &pe32 ) ) {
		OutputDebugString (TEXT ("Failed when calling Process32First"));
		CloseHandle( hProcessSnap );     // Must clean up the snapshot object!		
		return( FALSE );
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do {
		if (0 == _tcsnicmp (szProcessName, pe32.szExeFile, _tcslen (szProcessName))) {
			TCHAR szLibFile[MAX_PATH];
			GetModuleFileName(NULL, szLibFile, sizeof(szLibFile));	

			TCHAR * szStartCopyPos = _tcsrchr(szLibFile, TEXT('\\')) + 1;
			_tcsncpy_s(szStartCopyPos, szLibFile + MAX_PATH - szStartCopyPos, szInjectLibName, szLibFile + MAX_PATH - szStartCopyPos);
			// Found the target process
			BOOL bRes;

			if (inject)
				bRes = InjectLib (pe32.th32ProcessID , szLibFile);
			else 
				bRes = EjectLib (pe32.th32ProcessID, szInjectLibName);

			if (!bRes) {
				ErrorExit (TEXT ("Inject or Eject Lib"));
			}
			break;
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return TRUE;
}

static VOID InstallDetours ()
{	
	TCHAR szSystemDir[BUFFER_SIZE];
	std::wstring dllName (TEXT ("detoured.dll"));

	if (!GetSystemDirectory (szSystemDir, BUFFER_SIZE))
		ErrorExit (TEXT ("Failed to get system directory"));
	
	std::wstring newPath (szSystemDir);

	if (!newPath.empty () && newPath[newPath.length () - 1] != '\\' && newPath[newPath.length () - 1] != '/')
		newPath.append (TEXT ("\\"));

	newPath += dllName;

	if (!CopyFile (dllName.c_str (), newPath.c_str (), FALSE)) {
		ErrorExit (TEXT ("Copy detoured.dll"));
	};
}

static void CleanupRollingSpaces ()
{
	HWND hwndRollingSpaces = GetHandleOfRollingSpaces (g_hInstance, IDS_ROLLINGSPACES_CLASS, IDS_TRAYCLASS);
	if (NULL != hwndRollingSpaces)
		SendMessage (hwndRollingSpaces, WM_CLOSE, 0, 0);
}