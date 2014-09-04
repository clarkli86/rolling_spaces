#ifndef HWND_QUEUE_H
#define HWND_QUEUE_H

#define MAX_HWND		512

extern "C" BOOL _declspec (dllexport) HwndExistInQueue (HWND hwnd);
extern "C" BOOL _declspec (dllexport) GetGlobalFlag ();
#endif //HWND_QUEUE_H