#ifndef INJECT_LIB_H
#define INJECT_LIB_H

#ifdef UNICODE
#define InjectLib InjectLibW
#define EjectLib  EjectLibW
#else
#define InjectLib InjectLibA
#define EjectLib  EjectLibA
#endif   // !UNICODE

extern BOOL WINAPI InjectLib(DWORD dwProcessId, PCWSTR pszLibFile);
extern BOOL WINAPI EjectLib(DWORD dwProcessId, PCWSTR pszLibFile);

#endif //INJECT_LIB_H