#pragma once

#define GLOBAL_HWND_QUEUE_PROTECTION_EVENT_NAME TEXT("GlobalHwndQueueProtectionEvent")

class CHwndQueueProtector
{
public:
	CHwndQueueProtector(void);
	~CHwndQueueProtector(void);

	static BOOL InitializeEvent ();

private:
	//TODO: Close this handle
	static HANDLE hEvent;
};
