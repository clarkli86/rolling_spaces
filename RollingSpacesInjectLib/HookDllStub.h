#ifndef HWND_QUEUE_STUB_H
#define HWND_QUEUE_STUB_H

#define ROLLINGSPACESHOOKLIBNAME TEXT("RollingSpacesHookLib.dll")
#define HWNDQUEUECLEARPROC		 "HwndQueueClear"
#define HWNDQUEUEPUSHBACKPROC	 "HwndQueuePushBack"
#define HWNDQUEUEHEADPROC		 "HwndQueueHead"
#define HWNDQUEUEPOPPROC		 "HwndQueuePop"

#define GETGLOBALFLAGPROC		 "GetGlobalFlag"
#define SETGLOBALFLAGPROC		 "SetGlobalFlag"

extern VOID HwndQueueClearStub ();
extern BOOL HwndQueuePushBackStub (HWND hwnd);
extern BOOL GetGlobalFlagStub ();
extern VOID SetGlobalFlagStub (BOOL flag);
#endif //HWND_QUEUE_STUB_H