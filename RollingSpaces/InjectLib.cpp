#include "stdafx.h"
#include "InjectLib.h"

#include <windowsx.h>
#include <stdio.h>
#include <tchar.h>
#include <malloc.h>        // For alloca
#include <TlHelp32.h>
#include "Resource.h"


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI InjectLibW(DWORD dwProcessId, PCWSTR pszLibFile) {

   BOOL fOk = FALSE; // Assume that the function fails
   HANDLE hProcess = NULL, hThread = NULL;
   PWSTR pszLibFileRemote = NULL;

   __try {
      // Get a handle for the target process.
      hProcess = OpenProcess(
         PROCESS_QUERY_INFORMATION |   // Required by Alpha
         PROCESS_CREATE_THREAD     |   // For CreateRemoteThread
         PROCESS_VM_OPERATION      |   // For VirtualAllocEx/VirtualFreeEx
         PROCESS_VM_WRITE,             // For WriteProcessMemory
         FALSE, dwProcessId);
      if (hProcess == NULL) __leave;

      // Calculate the number of bytes needed for the DLL's pathname
      int cch = 1 + lstrlenW(pszLibFile);
      int cb  = cch * sizeof(WCHAR);

      // Allocate space in the remote process for the pathname
      pszLibFileRemote = (PWSTR) 
         VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
      if (pszLibFileRemote == NULL) __leave;

      // Copy the DLL's pathname to the remote process's address space
      if (!WriteProcessMemory(hProcess, pszLibFileRemote, 
         (PVOID) pszLibFile, cb, NULL)) __leave;

      // Get the real address of LoadLibraryW in Kernel32.dll
      PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
         GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
      if (pfnThreadRtn == NULL) __leave;

      // Create a remote thread that calls LoadLibraryW(DLLPathname)
      hThread = CreateRemoteThread(hProcess, NULL, 0, 
         pfnThreadRtn, pszLibFileRemote, 0, NULL);
      if (hThread == NULL) __leave;

      // Wait for the remote thread to terminate
      WaitForSingleObject(hThread, INFINITE);

      fOk = TRUE; // Everything executed successfully
   }
   __finally { // Now, we can clean everthing up

      // Free the remote memory that contained the DLL's pathname
      if (pszLibFileRemote != NULL) 
         VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

      if (hThread  != NULL) 
         CloseHandle(hThread);

      if (hProcess != NULL) 
         CloseHandle(hProcess);
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI InjectLibA(DWORD dwProcessId, PCSTR pszLibFile) {

   // Allocate a (stack) buffer for the Unicode version of the pathname
   PWSTR pszLibFileW = (PWSTR) 
      _alloca((lstrlenA(pszLibFile) + 1) * sizeof(WCHAR));

   // Convert the ANSI pathname to its Unicode equivalent
   wsprintfW(pszLibFileW, L"%S", pszLibFile);

   // Call the Unicode version of the function to actually do the work.
   return(InjectLibW(dwProcessId, pszLibFileW));
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI EjectLibW(DWORD dwProcessId, PCWSTR pszLibFile) {

   BOOL fOk = FALSE; // Assume that the function fails
   HANDLE hthSnapshot = NULL;
   HANDLE hProcess = NULL, hThread = NULL;

   __try {
      // Grab a new snapshot of the process
      hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
      if (hthSnapshot == NULL) __leave;

      // Get the HMODULE of the desired library
      MODULEENTRY32W me = { sizeof(me) };
      BOOL fFound = FALSE;
      BOOL fMoreMods = Module32FirstW(hthSnapshot, &me);
      for (; fMoreMods; fMoreMods = Module32NextW(hthSnapshot, &me)) {
         fFound = (lstrcmpiW(me.szModule,  pszLibFile) == 0) || 
                  (lstrcmpiW(me.szExePath, pszLibFile) == 0);
         if (fFound) break;
      }
      if (!fFound) __leave;

      // Get a handle for the target process.
      hProcess = OpenProcess(
         PROCESS_QUERY_INFORMATION |   // Required by Alpha
         PROCESS_CREATE_THREAD     | 
         PROCESS_VM_OPERATION,  // For CreateRemoteThread
         FALSE, dwProcessId);
      if (hProcess == NULL) __leave;

      // Get the real address of LoadLibraryW in Kernel32.dll
      PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
         GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
      if (pfnThreadRtn == NULL) __leave;

      // Create a remote thread that calls LoadLibraryW(DLLPathname)
      hThread = CreateRemoteThread(hProcess, NULL, 0, 
         pfnThreadRtn, me.modBaseAddr, 0, NULL);
      if (hThread == NULL) __leave;

      // Wait for the remote thread to terminate
      WaitForSingleObject(hThread, INFINITE);

      fOk = TRUE; // Everything executed successfully
   }
   __finally { // Now we can clean everything up

      if (hthSnapshot != NULL) 
         CloseHandle(hthSnapshot);

      if (hThread     != NULL) 
         CloseHandle(hThread);

      if (hProcess    != NULL) 
         CloseHandle(hProcess);
   }

   return(fOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI EjectLibA(DWORD dwProcessId, PCSTR pszLibFile) {

   // Allocate a (stack) buffer for the Unicode version of the pathname
   PWSTR pszLibFileW = (PWSTR) 
      _alloca((lstrlenA(pszLibFile) + 1) * sizeof(WCHAR));

   // Convert the ANSI pathname to its Unicode equivalent
   wsprintfW(pszLibFileW, L"%S", pszLibFile);

   // Call the Unicode version of the function to actually do the work.
   return(EjectLibW(dwProcessId, pszLibFileW));
}