#ifndef DRAG_INFO_H
#define DRAG_INFO_H

enum MouseActionEnum {
	MOUSE_ACTION_NONE = 0,	
	MOUSE_ACTION_DRAGGING = 1
};

typedef struct _DragInfo {
	MouseActionEnum mouseAction;
	HWND			hwndDragIn;
} DragInfo, *PDragInfo;
#endif //DRAG_INFO_H