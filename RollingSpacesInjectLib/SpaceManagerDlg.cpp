// SpaceManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpaceManagerDlg.h"
#include "Globals.h"
#include "shobjidl.h"
#include <objbase.h>
#include "RollingSpacesInjectLib.h"
#include "../common/includes/Utils.h"

// CSpaceManagerDlg dialog
CSpaceManagerDlg::CSpaceManagerDlg() : m_pTaskbar (NULL)
{
	CoInitialize (NULL);
	
	HRESULT hr = CoCreateInstance (CLSID_TaskbarList, NULL, CLSCTX_INPROC, IID_ITaskbarList, (LPVOID*)&m_pTaskbar);
	if (FAILED (hr)) {
		TraceLastError(TEXT ("Initialize COM enviorment inside RollingSpaces!"));
		return;
	}
	
	hr = m_pTaskbar->HrInit ();

	if (FAILED (hr)) {
		TraceLastError (TEXT ("Initialize ITaskbar!"));
		return;
	}
}

CSpaceManagerDlg::~CSpaceManagerDlg ()
{
	m_pTaskbar->Release ();
	CoUninitialize ();	
}
// CSpaceManagerDlg message handlers

BOOL CSpaceManagerDlg::InitDialog(HWND hwndDlg)
{	
	m_hwndDlg = hwndDlg;

	// Initialize tab
	m_hwndTab = GetDlgItem (m_hwndDlg, IDC_TAB);
	TCITEM tie = {0};
	tie.mask = TCIF_TEXT;     
    tie.pszText = TEXT ("Windows");
	TabCtrl_InsertItem (m_hwndTab, 0, &tie);
    tie.pszText = TEXT ("Processes");
	TabCtrl_InsertItem (m_hwndTab, 1, &tie);

		
	m_hwndListWnd = GetDlgItem (m_hwndDlg, IDC_LIST_WND);

	// Create image list
	m_hImageList = ImageList_Create (16, 16, ILC_COLOR32, 0, 0);
	ImageList_AddIcon (m_hImageList, LoadIcon (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDI_FOLDER)));
	ListView_SetImageList (m_hwndListWnd, m_hImageList, LVSIL_SMALL);
	
	RECT rect;
	GetClientRect(m_hwndListWnd, &rect);

	// Initialie the column
	LVCOLUMN column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.fmt = LVCFMT_LEFT;
	column.cx = rect.right - rect.left;
	column.pszText = TEXT ("");

	ListView_InsertColumn (m_hwndListWnd, 0, &column);

	////// Set up initial variables
	m_bDragging = false;
	m_nDragIndex = -1;
	m_nDropIndex = -1;
	m_hDragImage = NULL;

	// Initialize the workspace controls
	// TODO: The size should be read from a config file
	m_WorkSpaceManager.CreateWorkSpaces (GetDlgItem (m_hwndDlg, IDC_STATIC_WORKSPACE));
	m_WorkSpaceManager.GetWorkSpaceCtlWnd (0)->Activate ();	

	// Copy current data to manager dialog
	m_WorkSpaceManager.PrepareData ();
	// Initialize the items
	PrepareItems ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.


void CSpaceManagerDlg::OnLvnBegindragListWnd(LPNMLISTVIEW pNMListView)
{
	//This routine sets the parameters for a Drag and Drop operation.
	//It sets some variables to track the Drag/Drop as well
	// as creating the drag image to be shown during the drag.

	//// Save the index of the item being dragged in m_nDragIndex
	////  This will be used later for retrieving the info dragged
	m_nDragIndex = pNMListView->iItem;

	//// Create a drag image
	POINT pt;
	INT nOffset = -10; //offset in pixels for drag image
	
	m_hDragImage = ListView_CreateDragImage (m_hwndListWnd, m_nDragIndex, &pt);
	_ASSERT(m_hDragImage); //make sure it was created
	//We will call delete later (in LButtonUp) to clean this up
	
	//// Change the cursor to the drag image
	////	(still must perform DragMove() in OnMouseMove() to show it moving)
	ImageList_BeginDrag (m_hDragImage, 0, nOffset, nOffset - 4);
	//m_pDragImage->BeginDrag(0, CPoint(nOffset, nOffset - 4));
	ClientToScreen (m_hwndListWnd, &pNMListView->ptAction);
	ImageList_DragEnter (GetDesktopWindow (), pNMListView->ptAction.x, pNMListView->ptAction.y); 
	//m_pDragImage->DragEnter(GetDesktopWindow (), pNMListView->ptAction);
	
	//// Set dragging flag and others
	m_bDragging = TRUE;	//we are in a drag and drop operation
	m_nDropIndex = -1;	//we don't have a drop index yet
	m_hwndDragList = m_hwndListWnd; //make note of which list we are dragging from
	m_hwndDropWnd = m_hwndListWnd;
	
	//// Capture all mouse messages
	SetCapture (m_hwndDlg);
}

void CSpaceManagerDlg::OnMouseMove(UINT nFlags, POINT point)
{
		//While the mouse is moving, this routine is called.
	//This routine will redraw the drag image at the present
	// mouse location to display the dragging.
	//Also, while over a CListCtrl, this routine will highlight
	// the item we are hovering over.

	//// If we are in a drag/drop procedure (m_bDragging is true)
	if (m_bDragging)
	{
		//// Move the drag image
		POINT pt = point;	//get our current mouse coordinates
		ClientToScreen(m_hwndDlg, &pt); //convert to screen coordinates
		ImageList_DragMove (pt.x, pt.y);
		//m_pDragImage->DragMove(pt); //move the drag image to those coordinates
		// Unlock window updates (this allows the dragging image to be shown smoothly)
		ImageList_DragShowNolock (FALSE);
		//m_pDragImage->DragShowNolock(false);

		//// Get the CWnd pointer of the window that is under the mouse cursor
		HWND hwndDropWnd = WindowFromPoint (pt);
		_ASSERT(hwndDropWnd); //make sure we have a window

		//If we are hovering over a CListCtrl we need to adjust the highlights
		if(hwndDropWnd == m_hwndListWnd)
		{			
			OutputDebugString (TEXT ("Hit"));
			const INT space = 2;//width of the validate area.
			//Note that we can drop here
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			
			//// Turn off hilight for previous drop target			
			
			// Get the item that is below cursor
			// Convert from screen coordinates to drop target client coordinates
			ScreenToClient(hwndDropWnd, &pt);
			LVHITTESTINFO lvTestInfo = {0};
			lvTestInfo.pt = pt;
			
			m_nDropIndex = ListView_HitTest (m_hwndListWnd, &lvTestInfo);
			//m_nDropIndex = pList->HitTest(pt, &uFlags);			
			if (-1 != m_nDropIndex) {
				RECT rect;
				ListView_GetItemRect (m_hwndListWnd, m_nDropIndex, &rect, LVIR_ICON);
				//pList->GetItemRect (m_nDropIndex, &rect, LVIR_ICON);
				RECT upperRect = rect;
				upperRect.bottom = upperRect.top + space;
				if (PtInRect (&upperRect, pt)) {
					SetCursor (LoadCursor (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDC_INSERT)));//Insert to the above position
				} else {
					RECT lowerRect = rect;
					lowerRect.top = lowerRect.bottom - space;
					if (PtInRect (&lowerRect, pt)) {
						SetCursor (LoadCursor (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDC_INSERT)));
						++ m_nDropIndex; //Insert to the below position
					} else {
						SetCursor (LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW)));
					}
				}
			}			
		}		
		else
		{	
			if (m_WorkSpaceManager.IsPointInWorkSpace (pt)) {
				//TODO: Add checking logic if this item could be placed in the below workspace
				CWorkspaceCtl* pWorkSpaceCtl = m_WorkSpaceManager.FromHandle (hwndDropWnd);
				// Can't drag in the same worksapce
				if (NULL != pWorkSpaceCtl && pWorkSpaceCtl != m_WorkSpaceManager.GetActiveWorkSpace ()) {
					SetCursor (LoadCursor (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDC_DRAGIN)));				
				} else
					SetCursor(LoadCursor(NULL, MAKEINTRESOURCE (IDC_NO)));
			} else {
				//If we are not hovering over a CListCtrl, change the cursor
				// to note that we cannot drop here
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE (IDC_NO)));
			}
		}
		// Lock window updates
		ImageList_DragShowNolock (TRUE);
		//m_pDragImage->DragShowNolock(true);
	}		
}

void CSpaceManagerDlg::OnLButtonUp(UINT nFlags, POINT point)
{
	//This routine is the end of the drag/drop operation.
	//When the button is released, we are to drop the item.
	//There are a few things we need to do to clean up and
	// finalize the drop:
	//	1) Release the mouse capture
	//	2) Set m_bDragging to false to signify we are not dragging
	//	3) Actually drop the item (we call a separate function to do that)

	//If we are in a drag and drop operation (otherwise we don't do anything)
	if (m_bDragging)
	{
		// Release mouse capture, so that other controls can get control/messages
		ReleaseCapture ();

		// Note that we are NOT in a drag operation
		m_bDragging = FALSE;

		// End dragging image
		ImageList_DragLeave (GetDesktopWindow ());
		ImageList_EndDrag ();

		ImageList_Destroy (m_hDragImage); //must delete it because it was created at the beginning of the drag

		POINT pt = point; //Get current mouse coordinates
		ClientToScreen (m_hwndDlg, &pt); //Convert to screen coordinates
		// Get the CWnd pointer of the window that is under the mouse cursor
		HWND hwndDropWnd = WindowFromPoint (pt);
		_ASSERT (hwndDropWnd); //make sure we have a window pointer
		// If window is CListCtrl, we perform the drop
		if (hwndDropWnd == m_hwndListWnd) {
			m_hwndDropList = hwndDropWnd; //Set pointer to the list we are dropping on
			DropItemOnList(m_hwndDragList, m_hwndDropList); //Call routine to perform the actual drop
		} else {
			if (m_WorkSpaceManager.IsPointInWorkSpace (pt)) {				
				//TODO: Add it to the a workspace
				CWorkspaceCtl* pWorkSpaceCtl = m_WorkSpaceManager.FromHandle (hwndDropWnd);
				// Can't drag in the same worksapce
				if (NULL != pWorkSpaceCtl && pWorkSpaceCtl != m_WorkSpaceManager.GetActiveWorkSpace ()) {
					DropItemOnWorkSpace (pWorkSpaceCtl);					
				}
			}
		}
	}	
}

void CSpaceManagerDlg::DropItemOnWorkSpace (CWorkspaceCtl* pWorkSpaceCtl)
{
	INT iTabSel = TabCtrl_GetCurSel (m_hwndTab);
	if (0 == iTabSel) {
		// Window Dropped
		ListProcessWindow windowHandles = m_WorkSpaceManager.GetActiveWorkSpace ()->GetProcessWindows ();
		ListProcessWindow droppedWindowHandles;

		INT nDragIndex = ListView_GetNextItem (m_hwndDragList, -1, LVNI_SELECTED);
		while(nDragIndex >= 0) {
			ListProcessWindow::iterator iter = windowHandles.begin ();
			// move the iter forward manually
			INT increment = nDragIndex;
			while (increment --) {
				++ iter;
			}

			droppedWindowHandles.push_back (*iter);

			nDragIndex = ListView_GetNextItem (m_hwndListWnd, nDragIndex, LVNI_SELECTED);			
		}

		pWorkSpaceCtl->AddProcessWindows (droppedWindowHandles);
		m_WorkSpaceManager.GetActiveWorkSpace ()->RemoveProcessWindows (droppedWindowHandles);

		PrepareItems ();
	} else if (1 == iTabSel) {
		// Process handle dropped
		/*ListProcessHandle processHandles = m_WorkSpaceManager.GetActiveWorkSpace ()->GetProcessHandles ();
		ListProcessHandle droppedProcessHandles;

		INT nDragIndex = ListView_GetNextItem (m_hwndDragList, 0, LVNI_SELECTED);		
		while(m_nDragIndex > 0) {
			ListProcessHandle::iterator iter = processHandles.begin () + m_nDragIndex;
			processHandles.push_back (*iter);

			m_nDragIndex = ListView_GetNextItem (m_hwndListWnd, nDragIndex, LVNI_SELECTED);			
		}

		pWorkSpaceCtl->AddProcessHandles (droppedProcessHandles);*/
	}
}

// hwndDragList and hwndDropList are Handles of the ListView windows
void CSpaceManagerDlg::DropItemOnList(HWND hwndDragList, HWND hwndDropList)
{
	//This routine performs the actual drop of the item dragged.
	//It simply grabs the info from the Drag list (pDragList)
	// and puts that info into the list dropped on (pDropList).
	//Send:	pDragList = pointer to CListCtrl we dragged from,
	//		pDropList = pointer to CListCtrl we are dropping on.
	//Return: nothing.

	////Variables

	// Unhilight the drop target
	//pDropList->SetItemState (m_nDropIndex, 0, LVIS_DROPHILITED);

	//Set up the LV_ITEM for retrieving item from pDragList and adding the new item to the pDropList
	TCHAR szLabel[256];
	LVITEM lviT;

	ZeroMemory(&lviT, sizeof (LVITEM)); //allocate and clear memory space for LV_ITEM
	lviT.iItem		= m_nDragIndex;
	lviT.mask		= LVIF_TEXT;
	lviT.pszText		= szLabel;
	lviT.cchTextMax	= 255;
	
	lvItem lvi;
	lvi.plvi = &lviT;
	lvi.plvi->iItem = m_nDragIndex;
	lvi.plvi->mask = LVIF_TEXT;
	lvi.plvi->pszText = szLabel;
	lvi.plvi->cchTextMax = 255;
	
	if(1 == ListView_GetSelectedCount (hwndDragList))
	{
		// Get item that was dragged
		ListView_GetItem (hwndDragList, lvi.plvi);		
		//lvi.sCol2 = pDragList->GetItemText(lvi.plvi->iItem, 1);

		// Delete the original item (for Move operation)
		// This is optional. If you want to implement a Copy operation, don't delete.
		// This works very well though for re-arranging items within a CListCtrl.
		// It is written at present such that when dragging from one list to the other
		//  the item is copied, but if dragging within one list, the item is moved.
		if(hwndDragList == hwndDropList)
		{
			INT res = ListView_DeleteItem (hwndDragList, m_nDragIndex);			
			if(m_nDragIndex < m_nDropIndex) m_nDropIndex--; //decrement drop index to account for item
															//being deleted above it
		}

		// Insert item into pDropList
		// if m_nDropIndex == -1, iItem = GetItemCount() (inserts at end of list), else iItem = m_nDropIndex
		lvi.plvi->iItem = (m_nDropIndex == -1) ? ListView_GetSelectedCount (hwndDropList) : m_nDropIndex;
		INT res = ListView_InsertItem (hwndDropList, lvi.plvi);		
		//pDropList->SetItemText(lvi.plvi->iItem, 1, (LPCTSTR)lvi.sCol2);

		// Select the new item we just inserted
		ListView_SetItemState (hwndDropList, lvi.plvi->iItem, LVIS_SELECTED, LVIS_SELECTED);		
	}
	else //more than 1 item is being dropped
	{
		//We have to parse through all of the selected items from the DragList
		//1) Retrieve the info for the items and store them in memory
		//2) If we are reordering, delete the items from the list
		//3) Insert the items into the list (either same list or different list)

		std::list<LVITEM> listItems;
			
		//Retrieve the selected items
		m_nDragIndex = ListView_GetNextItem (hwndDragList, -1, LVNI_SELECTED); //iterator for the CListCtrl
		//POSITION pos = pDragList->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
		while(m_nDragIndex >= 0) //so long as we have a valid POSITION, we keep iterating
		{
			LVITEM lvitem = {0};
			lvitem.iItem = m_nDragIndex;
			lvitem.mask = LVIF_TEXT;
			lvitem.pszText = new TCHAR[512]; //since this is a pointer to the string, we need a new pointer to a new string on the heap
			lvitem.cchTextMax = 255;

			//Get the item
			lvitem.iItem = m_nDragIndex; //set the index in the drag list to the selected item
			ListView_GetItem (hwndDragList, &lvitem);
						
			//Save the pointer to the new item in our CList
			listItems.push_back(lvitem);

			m_nDragIndex = ListView_GetNextItem (hwndDragList, m_nDragIndex, LVNI_SELECTED);			
		} //EO while(pos) -- at this poINT we have deleted the moving items and stored them in memory

		if(hwndDragList == hwndDropList) //we are reordering the list (moving)
		{
			//Delete the selected items
			m_nDragIndex = ListView_GetNextItem (hwndDragList, 0, LVNI_SELECTED);
			while(m_nDragIndex > 0)
			{	
				ListView_DeleteItem (hwndDragList, m_nDragIndex);				
				if(m_nDragIndex < m_nDropIndex) m_nDropIndex--; //must decrement the drop index to account
																//for the deleted items
				m_nDragIndex = ListView_GetNextItem (hwndDragList, -- m_nDragIndex, LVNI_SELECTED);
			} //EO while(pos)
		} //EO if(pDragList...

		//Iterate through the items stored in memory and add them back into the CListCtrl at the drop index
		std::list<LVITEM> ::iterator begin (listItems.begin ()), end (listItems.end ());
		for (; begin != end; ++ begin) 
		{			
			m_nDropIndex = (m_nDropIndex == -1) ? ListView_GetItemCount (hwndDropList) : m_nDropIndex;
			begin->iItem = m_nDropIndex;
			ListView_InsertItem (hwndDropList, &(*begin));

			if (NULL != begin->pszText)
				delete[] begin->pszText;
			
			ListView_SetItemState(hwndDropList, begin->iItem, LVIS_SELECTED, LVIS_SELECTED); //highlight/select the item we just added

			m_nDropIndex++; //increment the index we are dropping at to keep the dropped items in the same order they were in in the Drag List			
		} //EO while(listPos)
	}
}

INT_PTR CALLBACK CSpaceManagerDlg::SpaceManagerDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		g_SpaceManagerDlg.InitDialog (hwndDlg);
		return TRUE;
	case WM_NOTIFY:		
		g_SpaceManagerDlg.RemoveTabFromTaskbar (hwndDlg);
		switch (((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGE:
			if (((LPNMHDR)lParam)->idFrom == IDC_TAB)
			{
				g_SpaceManagerDlg.PrepareItems ();
				return TRUE;
			}
			break;
		case LVN_BEGINDRAG:
			if (((LPNMHDR)lParam)->idFrom == IDC_LIST_WND)
			{
				g_SpaceManagerDlg.OnLvnBegindragListWnd ((LPNMLISTVIEW)lParam);
				return TRUE;
			}
			break;
		default:
			break;
		}
		return TRUE;
	case WM_MOUSEMOVE:
		{
			POINT pt;
			pt.x = LOWORD (lParam); 
			pt.y = HIWORD (lParam); 
			g_SpaceManagerDlg.OnMouseMove (wParam, pt);
		}
		return TRUE;
	case WM_LBUTTONUP:
		{
			POINT pt;
			pt.x = LOWORD (lParam); 
			pt.y = HIWORD (lParam); 
			g_SpaceManagerDlg.OnLButtonUp (wParam, pt);
		}
		return TRUE;
	case WM_COMMAND:		
		switch (LOWORD(wParam)) 
		{ 
		case IDC_OK: 
			{
				g_SpaceManagerDlg.OnOK ();
			}
			// The code flows of IDOK and IDCANCEL are deliberately cascaded
		case IDC_CANCEL: 
			EndDialog(hwndDlg, wParam); 
			return TRUE; 
		} 
	default:
		break;
	} 
	return FALSE; 

}

void CSpaceManagerDlg::DoModal(HWND parent)
{
	DialogBox (GetCurrentModuleHandle (), MAKEINTRESOURCE (IDD_SPACE_MANAGER), parent, SpaceManagerDialogProc);
}

void CSpaceManagerDlg::PrepareItems ()
{
	// Clear original items.
	ListView_DeleteAllItems (m_hwndListWnd);

	INT iTabSel = TabCtrl_GetCurSel (m_hwndTab);
	
	if (0 == iTabSel) {
		// Display Windows
		LVITEM item = {0};
		item.mask = LVIF_TEXT;
		item.iItem = 0;

		ListProcessWindow listProcessWindows = m_WorkSpaceManager.GetActiveWorkSpace ()->GetProcessWindows ();

		ListProcessWindow::iterator begin (listProcessWindows.begin ()), end (listProcessWindows.end ());

		for (; begin != end; ++ begin) {
			TCHAR szWindowTitle[BUFFER_SIZE];
			//TODO: for testing
			if (1) {
				GetWindowText (begin->GetWindowHandle (), szWindowTitle, sizeof (szWindowTitle));				
			} else {
				wsprintf (szWindowTitle, TEXT ("Handle%d"), begin->GetWindowHandle ());
			}
			item.pszText = szWindowTitle;
			
			if (-1 == ListView_InsertItem (m_hwndListWnd, &item))
				TraceLastError (TEXT ("Insert Window Item"));

			++ item.iItem;
		}

		RECT rect;
		GetClientRect (m_hwndListWnd, &rect);
		InvalidateRect (m_hwndListWnd, &rect, TRUE);
	} else	if (1 == iTabSel) {		
		// Display Processes
		LVITEM item = {0};
		item.mask = LVIF_TEXT;
		item.iItem = 0;

		ListProcessHandle listProcessHandles = m_WorkSpaceManager.GetActiveWorkSpace ()->GetProcessHandles ();

		ListProcessHandle::iterator begin (listProcessHandles.begin ()), end (listProcessHandles.end ());

		for (; begin != end; ++ begin) {
			std::wstring tempStr = begin->GetProcessName ();
			item.pszText = (LPWSTR) (tempStr.c_str ());
			
			if (-1 == ListView_InsertItem (m_hwndListWnd, &item))
				TraceLastError (TEXT ("Insert Process Item"));

			++ item.iItem;
		}

		RECT rect;
		GetClientRect (m_hwndListWnd, &rect);
		InvalidateRect (m_hwndListWnd, &rect, TRUE);
	}
}

VOID CSpaceManagerDlg::OnOK()
{
	m_WorkSpaceManager.SubmitGrouping ();
	g_RollingSpaces.ActivateWorkSpace (g_RollingSpaces.GetActiveWorkSpace ()->GetSafeHwnd ());
}

void CSpaceManagerDlg::RemoveTabFromTaskbar (HWND hwndDlg) const
{	
	if (NULL == hwndDlg)
		return;	

	HRESULT hr = m_pTaskbar->DeleteTab (hwndDlg);
	
	if (FAILED (hr)) {
		TraceLastError( TEXT ("Failed to Add Tab!"));
		return;
	}	
}