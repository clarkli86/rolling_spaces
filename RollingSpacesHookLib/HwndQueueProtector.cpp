#include "stdafx.h"
#include "HwndQueueProtector.h"
#include "../common/includes/Utils.h"

HANDLE CHwndQueueProtector::hEvent = NULL;

CHwndQueueProtector::CHwndQueueProtector(void)
{
	WaitForSingleObject (CHwndQueueProtector::hEvent, INFINITE);
}

CHwndQueueProtector::~CHwndQueueProtector(void)
{
	SetEvent (CHwndQueueProtector::hEvent);
}

BOOL CHwndQueueProtector::InitializeEvent ()
{
	if (NULL != CHwndQueueProtector::hEvent) {		
		hEvent = CreateEvent (NULL, FALSE, TRUE, GLOBAL_HWND_QUEUE_PROTECTION_EVENT_NAME);

		if (NULL == hEvent) {
			TraceLastError (TEXT ("Creating/Openning global Hwnd Queue protection event"));
			return FALSE;
		}
	}
	return TRUE;
}
