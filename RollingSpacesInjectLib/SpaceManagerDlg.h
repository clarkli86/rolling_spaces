// SpaceManagerDlg.h : header file
//

#pragma once
#include "WorkSpaceCtl.h"
#include "WorkSpaceCtlManager.h"
#include <vector>
#include <commctrl.h>

typedef struct {
	LVITEM* plvi;
	std::string sCol2;
} lvItem, *plvItem;

struct ITaskbarList;
// CSpaceManagerDlg dialog
class CSpaceManagerDlg
{
// Construction
public:
	CSpaceManagerDlg ();
	~CSpaceManagerDlg ();
	void DoModal (HWND parent);
// Implementation
protected:
	HICON m_hIcon;

	HIMAGELIST  m_hImageList; //ListView Icon
	HIMAGELIST  m_hDragImage; //Drag Image

	HWND		m_hwndDlg; // Dialog handle
	HWND		m_hwndListWnd; // ListView Control
	HWND		m_hwndTab; // Tab Control
	HWND        m_hwndDragList;		//Which ListCtrl we are dragging FROM
	HWND	    m_hwndDropList;
	BOOL		m_bDragging;	//T during a drag operation
	int			m_nDragIndex;	//Index of selected item in the List we are dragging FROM
	int			m_nDropIndex;	//Index at which to drop item in the List we are dropping ON
	HWND 		m_hwndDropWnd;		//Pointer to window we are dropping on (will be cast to CListCtrl* type)
	
	HWND		m_hwndSpaceManagerDlg;

	ITaskbarList *m_pTaskbar;

	void OnLvnBegindragListWnd(LPNMLISTVIEW pNMListView);
	void DropItemOnList(HWND hwndDragList, HWND hwndDropList);
	void DropItemOnWorkSpace (CWorkspaceCtl*);
	void OnMouseMove(UINT nFlags, POINT point);
	void OnLButtonUp(UINT nFlags, POINT point);
	void OnOK ();
	// Generated message map functions
	BOOL InitDialog(HWND);

public:
	CWorkspaceCtlManager m_WorkSpaceManager;
	
	void PrepareItems ();
	void RemoveTabFromTaskbar (HWND) const;
private:
	static INT_PTR CALLBACK SpaceManagerDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);	
	void SetHandle (HWND);		
};
