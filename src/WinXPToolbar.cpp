/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */
#include "stdafx.h"
#include "TinyCad.h"
#include "WinXPToolbar.h"
#include ".\winxptoolbar.h"
// CWinXPToolbar
IMPLEMENT_DYNAMIC(CWinXPToolbar, CToolBar)
CWinXPToolbar::CWinXPToolbar()
{
}

CWinXPToolbar::~CWinXPToolbar()
{
}

BEGIN_MESSAGE_MAP(CWinXPToolbar, CToolBar)
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

// CWinXPToolbar message handlers


void CWinXPToolbar::OnNcPaint()
{
	EraseNonClient();
}

void CWinXPToolbar::EraseNonClient(void)
{
	//Get DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	//Draw borders in the nclient area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	//Erase parts that don't get drawn
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM) dc.m_hDC);

	//Draw the gripper
	DrawGripper(&dc, rectWindow);
}

void CWinXPToolbar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	//Paint the client area
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void CWinXPToolbar::DrawGripper(CDC * pDC, const CRect& rect)
{
	pDC->FillSolidRect(&rect, ::GetSysColor(COLOR_BTNFACE));
	CToolBar::DrawGripper(pDC, rect);
}

void CWinXPToolbar::AttachToolbarImages(UINT imageWidth, UINT inNormalImageID, UINT inDisabledImageID, UINT inHotImageID)
{
	// get the toolbar control associated with the CToolbar object
	CToolBarCtrl& barCtrl = GetToolBarCtrl();

	// make high-color image lists for each of the bitmaps
	// and attach the image lists to the toolbar control
	if (inNormalImageID)
	{
		MakeToolbarImageList(imageWidth, inNormalImageID, m_ToolbarImages);
		barCtrl.SetImageList(&m_ToolbarImages);
	}
	if (inDisabledImageID)
	{
		MakeToolbarImageList(imageWidth, inDisabledImageID, m_ToolbarImagesDisabled);
		barCtrl.SetDisabledImageList(&m_ToolbarImagesDisabled);
	}
	if (inHotImageID)
	{
		MakeToolbarImageList(imageWidth, inHotImageID, m_ToolbarImagesHot);
		barCtrl.SetHotImageList(&m_ToolbarImagesHot);
	}
}

// create an image list for the specified BMP resource
void CWinXPToolbar::MakeToolbarImageList(UINT imageWidth, UINT inBitmapID, CImageList& outImageList)
{
	CBitmap bitmap;

	// if we use CBitmap::LoadBitmap() to load the bitmap, the colors
	// will be reduced to the bit depth of the main screen and we won't
	// be able to access the pixels directly. To avoid those problems,
	// we'll load the bitmap as a DIBSection instead and attach the
	// DIBSection to the CBitmap.
	VERIFY (bitmap.Attach (::LoadImage (::AfxFindResourceHandle(
									MAKEINTRESOURCE (inBitmapID), RT_BITMAP),
							MAKEINTRESOURCE (inBitmapID), IMAGE_BITMAP, 0, 0,
							(LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));

	// obtain size of bitmap
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	int nTotalWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;

	// create a 24 bit image list with the same dimensions and number
	// of buttons as the toolbar
	VERIFY (outImageList.Create (
					imageWidth, nHeight, ILC_COLOR24|ILC_MASK, nTotalWidth / imageWidth, 0));

	// attach the bitmap to the image list
	VERIFY (outImageList.Add (&bitmap, RGB (0xC0, 0xC0, 0xC0)) != -1);
}

