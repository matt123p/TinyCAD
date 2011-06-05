/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002,2003 Matt Pyne.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// This handles the update of the main window
#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadView.h"
#include "colour.h"
#include "revision.h"
#include "option.h"
#include "ruler.h"
#include "TinyCadRegistry.h"
#include "UserColor.h"

/////////////////////////////////////////////////////////////////////////////
// CTinyCadView drawing

void CTinyCadView::OnDraw(CDC* pDC)
{
	CTinyCadDoc* pDoc = GetCurrentDocument();
	CDC BitmapDC;
	CBitmap *old_bitmap = NULL;

	int selected;

	CRect client;
	if (pDC->IsKindOf(RUNTIME_CLASS(CPaintDC)))
	{
		client = static_cast<CPaintDC*> (pDC)->m_ps.rcPaint;
	}
	else
	{
		GetClientRect(&client);
	}

	// Are we going to use off-screen drawing?
	BOOL osb = !pDC->IsPrinting() && m_use_offscreen_drawing && CreateBitmap(*pDC, client.Width(), client.Height());

	if (osb)
	{
		BitmapDC.CreateCompatibleDC(pDC);
		old_bitmap = BitmapDC.SelectObject(&m_bitmap);
	}

	{
		CContext dc(osb ? &BitmapDC : pDC, GetTransform(), this);

		CDPoint origin = GetTransform().GetOrigin();

		if (osb)
		{
			dc.SetPixelOffset(CPoint(-client.left, -client.top));
		}

		if (pDC->IsPrinting())
		{
			dc.SetBlack(CTinyCadRegistry::GetPrintBandW());
		}

		CDPoint Start, End;
		CRect rect;
		GetClientRect(&rect);
		TransformSnap snap;
		snap.SetGridSnap(FALSE);
		Start = GetTransform().DeScale(snap, CPoint(rect.left, rect.top));
		End = GetTransform().DeScale(snap, CPoint(rect.right, rect.bottom));

		// Is any of this region in the off-page area?
		if (!pDC->IsPrinting())
		{

			// Paint the region white
			if (pDC->IsPrinting())
			{
				dc.SelectBrush(cWHITE);
				dc.SelectPen(PS_SOLID, 1, cWHITE);
			}
			else
			{
				COLORREF col = GetCurrentDocument()->GetOptions()->GetUserColor().Get(CUserColor::BACKGROUND);
				dc.SelectBrush(col, 0);
				dc.SelectPen(PS_SOLID, 1, col);
			}
			dc.Rectangle(CDRect(Start.x - 2, Start.y - 2, End.x + 2, End.y + 2));

			dc.SelectBrush(cOFFPAGE);
			dc.SelectPen(PS_SOLID, 1, cOFFPAGE);

			if (End.x > GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x)
			{
				CDPoint a = CDPoint(GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x, 0);
				dc.Rectangle(CDRect(a.x, a.y, End.x, End.y));
			}
			if (End.y > GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y)
			{
				CDPoint a = CDPoint(Start.x, GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y);
				dc.Rectangle(CDRect(a.x, a.y, End.x, End.y));
			}
			if (Start.x < 0) dc.Rectangle(CDRect(0, Start.y, Start.x, End.y));
			if (Start.y < 0) dc.Rectangle(CDRect(Start.x, 0, End.x, Start.y));

			// Fill this region with a grid
			double grid = GetCurrentDocument()->m_snap.GetGrid();
			double SGrid = dc.GetTransform().doubleScale(grid);
			if (GetCurrentDocument()->GetOptions()->ShowGrid() && SGrid > 10)
			{
				double x = dc.GetTransform().GetOrigin().x;
				double y = dc.GetTransform().GetOrigin().y;

				TransformSnap s = GetCurrentDocument()->m_snap;
				s.SetGridSnap(TRUE);

				x = s.Snap(x);
				y = s.Snap(y);

				for (double xp = x >= 0 ? x : 0; xp < End.x && xp < GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x; xp += grid)
				{
					for (double yp = y >= 0 ? y : 0; yp < End.y && yp < GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y; yp += grid)
					{
						dc.SetPixel(CDPoint(xp, yp), 0);
					}
				}
			}
		}

		Start -= CDPoint(10, 10);
		End += CDPoint(10, 10);

		GetCurrentDocument()->GetSelectBegin();

		drawingIterator it = GetCurrentDocument()->GetDrawingBegin();
		while (it != GetCurrentDocument()->GetDrawingEnd())
		{
			CDrawingObject *obj = *it;

			selected = !pDC->IsPrinting() && GetCurrentDocument()->IsSelected(obj);
			paint_options options = selected ? draw_selected : draw_normal;

			if (!pDC->IsPrinting() || !obj->IsConstruction())
			{
				if (pDC->IsPrinting() || obj->IsInside(Start.x, End.x, Start.y, End.y))
				{
					obj->Paint(dc, options);
				}
			}

			++it;
		}

		// Now draw the selectable object, so it stands out...
		CDrawingObject *obj = GetCurrentDocument()->GetSelectable();
		if (obj != NULL && !GetCurrentDocument()->IsSelected(obj))
		{
			paint_options options = draw_selectable;
			GetCurrentDocument()->GetSelectable()->Paint(dc, options);
		}

		// If only one item is selected then just draw its handles now
		if (GetCurrentDocument()->IsSingleItemSelected())
		{
			GetCurrentDocument()->GetSingleSelectedItem()->PaintHandles(dc);
		}

		// if necessary turn back on the current object to be edited
		if (GetCurrentDocument()->GetEdit() != NULL) GetCurrentDocument()->GetEdit()->Paint(dc, draw_selected);

		// Draw the design details
		GetCurrentDocument()->Display(dc);
	}

	if (osb)
	{
		pDC->BitBlt(client.left, client.top, client.Width(), client.Height(), &BitmapDC, 0, 0, SRCCOPY);
		BitmapDC.SelectObject(old_bitmap);
	}
}

// Here is the bitmap for off-screen drawing...
//
// We share one bitmap with all views, we can get away with static members
// because we are not multi-thread
CBitmap CTinyCadView::m_bitmap;
int CTinyCadView::m_bitmap_width = 0;
int CTinyCadView::m_bitmap_height = 0;
int CTinyCadView::m_max_bitmap_size = 2048 * 2048;

// Create a bitmap for off-screen drawing...
bool CTinyCadView::CreateBitmap(CDC &dc, int width, int height)
{
	// Is there already a suitable bitmap?
	if (m_bitmap_width >= width && m_bitmap_height >= height)
	{
		return true;
	}

	// Is this beyond the maximum size we are willing to allocate?
	if (width * height > m_max_bitmap_size)
	{
		return false;
	}

	int bpp = dc.GetDeviceCaps(BITSPIXEL);
	if (bpp <= 16)
	{
		bpp = 16;
	}
	else
	{
		bpp = 24;
	}

	// Now try and create the bitmap...
	struct
	{
		BITMAPINFO bi;
		RGBQUAD bipal[3];
	} q;
	q.bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	q.bi.bmiHeader.biWidth = width;
	q.bi.bmiHeader.biHeight = height;
	q.bi.bmiHeader.biPlanes = 1;
	q.bi.bmiHeader.biBitCount = (WORD) bpp;
	q.bi.bmiHeader.biCompression = bpp == 16 ? BI_BITFIELDS : BI_RGB;
	q.bi.bmiHeader.biSizeImage = 0;
	q.bi.bmiHeader.biXPelsPerMeter = 0;
	q.bi.bmiHeader.biYPelsPerMeter = 0;
	q.bi.bmiHeader.biClrUsed = bpp == 16 ? 3 : 0;
	q.bi.bmiHeader.biClrImportant = 0;
	q.bi.bmiColors[0].rgbRed = 0;
	q.bi.bmiColors[0].rgbGreen = 0;
	q.bi.bmiColors[0].rgbBlue = 0;
	q.bi.bmiColors[0].rgbReserved = 0;

	// Set up the 5-6-5 bit masks
	if (bpp == 16)
	{
		((DWORD*) (q.bi.bmiColors))[0] = (WORD) (0x1F << 11); //make sure that RGQQUAD array is after the q.bi struct
		((DWORD*) (q.bi.bmiColors))[1] = (WORD) (0x3F << 5); //otherwise you will get an access violation
		((DWORD*) (q.bi.bmiColors))[2] = (WORD) (0x1F << 0);
	}

	void *bits;
	HBITMAP hb = CreateDIBSection(dc.m_hDC, &q.bi, DIB_RGB_COLORS, &bits, NULL, 0);

	if (!hb)
	{
		// Probably not enough memory...
		return false;
	}

	// Do we need to destroy the old bitmap?
	if (m_bitmap.m_hObject)
	{
		m_bitmap.DeleteObject();
	}

	m_bitmap.Attach(hb);
	m_bitmap_width = width;
	m_bitmap_height = height;

	return true;
}

Transform &CTinyCadView::GetTransform()
{
	return m_Printing ? m_Printing_Transform : GetCurrentDocument()->GetTransform();
}

// The Zoom function
// OnSetZoom:
// Sets the zoom from a menu selection
void CTinyCadView::ChangeZoomFactor(double NewZoom)
{
	GetTransform().SetZoomFactor(NewZoom);
}

void CTinyCadView::SetZoomFactor(double NewZoom)
{
	ChangeZoomFactor(NewZoom);
	SetScroll(GetTransform().GetOrigin().x, GetTransform().GetOrigin().y);
	Invalidate();
}

// Track the size of the window
void CTinyCadView::OnSize(UINT a, int cx, int cy)
{
	CView::OnSize(a, cx, cy);

	if (IsWindowVisible())
	{
		CTinyCadRegistry::SetMDIMaximize(GetParentFrame()->IsZoomed() != 0);
	}

	if (m_pDocument == NULL) return;

	CRect nSize;
	GetClientRect(nSize);
	if (vRuler != NULL) vRuler->OnNewSize(nSize);
	if (hRuler != NULL) hRuler->OnNewSize(nSize);

	if (GetCurrentDocument())
	{
		SetScroll(GetTransform().GetOrigin().x, GetTransform().GetOrigin().y);
	}

}

// The scroll bar functions:
// These control the horizontal and vertical scrolling of the window
// (uses OnSize to track the window's size to stop the user being able to place text off the max size)
void CTinyCadView::SetScroll(double NewX, double NewY, bool first)
{
	CRect rect;
	int px, py;

	// How big is the current window?
	GetClientRect(rect);

	// Convert region into internal co-ords
	px = static_cast<int> (GetTransform().doubleDeScale(rect.right));
	py = static_cast<int> (GetTransform().doubleDeScale(rect.bottom));

	// Allow a 10% overlap
	CDPoint xlap = GetCurrentDocument()->GetDetails().GetOverlap();
	int Xlap = static_cast<int> (xlap.x);
	int Ylap = static_cast<int> (xlap.y);

	// Now does this fit?
	if (px + NewX > GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x + Xlap) NewX = GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x + Xlap - px;
	if (py + NewY > GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y + Ylap) NewY = GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y + Ylap - py;
	if (NewX < -Xlap) NewX = -Xlap;
	if (NewY < -Ylap) NewY = -Ylap;

	if (first || GetTransform().GetOrigin() != CDPoint(NewX, NewY) || m_old_zoom_factor != GetTransform().GetZoomFactor())
	{
		GetTransform().SetOriginX(NewX);
		GetTransform().SetOriginY(NewY);

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.nMin = 0;
		si.nMax = GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().x + Xlap * 2;
		si.nPage = static_cast<int> (GetTransform().doubleDeScale(rect.Width()));
		si.nPos = static_cast<int> (GetTransform().GetOrigin().x) + Xlap;
		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
		SetScrollInfo(SB_HORZ, &si, TRUE);

		si.cbSize = sizeof(SCROLLINFO);
		si.nMin = 0;
		si.nMax = GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint().y + Ylap * 2;
		si.nPage = static_cast<int> (GetTransform().doubleDeScale(rect.Height()));
		si.nPos = static_cast<int> (GetTransform().GetOrigin().y) + Ylap;
		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
		SetScrollInfo(SB_VERT, &si, TRUE);

		if (hRuler != NULL) hRuler->RedrawWindow();
		if (vRuler != NULL) vRuler->RedrawWindow();
		Invalidate();
	}

	m_old_zoom_factor = GetTransform().GetZoomFactor();
}

void CTinyCadView::SetScrollCentre(CDPoint c)
{
	CRect rect;
	CDPoint p;

	GetClientRect(rect);
	p = GetTransform().DeScale(GetCurrentDocument()->m_snap, CPoint(rect.right / 2, rect.bottom / 2));
	SetScroll(GetTransform().GetOrigin().x + c.x - p.x, GetTransform().GetOrigin().y + c.y - p.y);
}

void CTinyCadView::SetScrollPoint(CDPoint c, CPoint p)
{
	CRect rect;
	CDPoint p2;

	GetClientRect(rect);
	//p2=GetTransform().DeScale(GetCurrentDocument()->m_snap,p - GetTransform().GetPixelOffset());
	p2 = GetTransform().DeScale(p - GetTransform().GetPixelOffset());
	SetScroll(GetTransform().GetOrigin().x + c.x - p2.x, GetTransform().GetOrigin().y + c.y - p2.y);
}

// The message handlers for the VScroll and HScroll messages
void CTinyCadView::OnHScroll(UINT wParam, UINT pos, CScrollBar*)
{
	CRect rect;
	int moveFast, moveSlow;
	int x = static_cast<int> (GetTransform().GetOrigin().x);
	int y = static_cast<int> (GetTransform().GetOrigin().y);

	GetClientRect(rect);
	int Width = static_cast<int> (GetTransform().doubleDeScale(rect.right));
	moveFast = (Width << 4) / 20;
	moveSlow = (Width * 20) / 100;

	switch (wParam)
	{
		case SB_LINEUP:
			SetScroll(x - moveSlow, y);
			break;
		case SB_PAGEUP:
			SetScroll(x - moveFast, y);
			break;
		case SB_LINEDOWN:
			SetScroll(x + moveSlow, y);
			break;
		case SB_PAGEDOWN:
			SetScroll(x + moveFast, y);
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
		{
			CDPoint xlap = GetCurrentDocument()->GetDetails().GetOverlap();
			SetScroll(static_cast<int> (pos) - static_cast<int> (xlap.x), y);
		}
			break;
	}
}

// The message handlers for the VScroll and HScroll messages
void CTinyCadView::OnVScroll(UINT wParam, UINT pos, CScrollBar*)
{
	CRect rect;
	int moveFast, moveSlow;
	int x = static_cast<int> (GetTransform().GetOrigin().x);
	int y = static_cast<int> (GetTransform().GetOrigin().y);

	GetClientRect(rect);
	int Width = static_cast<int> (GetTransform().doubleDeScale(rect.bottom));
	moveFast = (Width << 4) / 20;
	moveSlow = (Width * 20) / 100;

	switch (wParam)
	{
		case SB_LINEUP:
			SetScroll(x, y - moveSlow);
			break;
		case SB_PAGEUP:
			SetScroll(x, y - moveFast);
			break;
		case SB_LINEDOWN:
			SetScroll(x, y + moveSlow);
			break;
		case SB_PAGEDOWN:
			SetScroll(x, y + moveFast);
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
		{
			CDPoint xlap = GetCurrentDocument()->GetDetails().GetOverlap();
			SetScroll(x, static_cast<int> (pos) - static_cast<int> (xlap.y));
		}
			break;
	}
}

