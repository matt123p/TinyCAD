/*
 TinyCAD program for schematic capture
 Copyright 1994/1995/2002-2005 Matt Pyne.

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

// TextEditView.cpp : implementation file
//

#include "stdafx.h"
#include "TinyCad.h"
#include "TextEditView.h"
#include ".\texteditview.h"
#include "TextEditDoc.h"

// CTextEditView
IMPLEMENT_DYNCREATE(CTextEditView, CView)

CTextEditView::CTextEditView()
{
}

CTextEditView::~CTextEditView()
{
}

BEGIN_MESSAGE_MAP(CTextEditView, CView)
	ON_WM_SIZE()
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CTextEditView diagnostics

#ifdef _DEBUG
void CTextEditView::AssertValid() const
{
	CView::AssertValid();
}

void CTextEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// CTextEditView message handlers

void CTextEditView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	CTextEditDoc* pDoc = static_cast<CTextEditDoc*> (GetDocument());

	// Create a CEdit box to show the text...
	CRect r;
	GetClientRect(r);
	int ro = pDoc->GetReadOnly() ? ES_READONLY : 0;
	m_edit_ctrl.Create(ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ro, r, this, 1000);

	// Create the font for the edit control
	m_edit_font.CreateFont(-14, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Courier New"));
	m_edit_ctrl.SetFont(&m_edit_font);

	// Now put the text into the edit control
	m_edit_ctrl.SetWindowText(static_cast<CTextEditDoc*> (GetDocument())->GetText());
}

void CTextEditView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_edit_ctrl.m_hWnd)
	{
		m_edit_ctrl.MoveWindow(0, 0, cx, cy);
	}
}

void CTextEditView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CTextEditDoc* pDoc = static_cast<CTextEditDoc*> (GetDocument());

	writeBack();
	m_print_data = pDoc->GetText();

	// Count the number of pages...
	int pages = 0;
	while (!m_print_data.IsEmpty())
	{
		++pages;
		printPage(pDC, false);
	}

	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(pages);

	CView::OnBeginPrinting(pDC, pInfo);
}

BOOL CTextEditView::OnPreparePrinting(CPrintInfo* pInfo)
{

	return CView::DoPreparePrinting(pInfo);
}

void CTextEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CTextEditDoc* pDoc = static_cast<CTextEditDoc*> (GetDocument());
	m_print_data = pDoc->GetText();

	if (pInfo)
	{
		// Count the number of pages...
		int pages = pInfo->m_nCurPage;
		while (!m_print_data.IsEmpty() && pages > 1)
		{
			--pages;
			printPage(pDC, false);
		}

	}

	CView::OnPrepareDC(pDC, pInfo);
}

void CTextEditView::writeBack()
{
	CTextEditDoc* pDoc = static_cast<CTextEditDoc*> (GetDocument());
	if (!pDoc->GetReadOnly())
	{
		m_edit_ctrl.GetWindowText(pDoc->GetText());
	}
}

void CTextEditView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{

	CView::OnEndPrinting(pDC, pInfo);
}

// CTextEditView drawing
void CTextEditView::OnDraw(CDC* pDC)
{
	// We only need to do something if we are printing...
	if (pDC->IsPrinting())
	{
		printPage(pDC, true);
	}
}

void CTextEditView::printPage(CDC *pDC, bool print)
{
	CTextEditDoc* pDoc = static_cast<CTextEditDoc*> (GetDocument());

	// Get the height of the screen
	int height = pDC->GetDeviceCaps(VERTRES);

	// Create the correct font
	int size = (pDC->GetDeviceCaps(LOGPIXELSY) * 10) / 72; // 10 pts height
	CFont font;
	font.CreateFont(size, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Courier New"));

	CFont *old_font = pDC->SelectObject(&font);

	// Reset the printing
	CSize sz;
	sz = pDC->GetTextExtent("_____");
	m_printing_y_pos = sz.cy;
	m_print_margin = sz.cx;

	// Now print out the text
	bool done = false;
	int line = 1;
	while (!done && m_printing_y_pos < height - size * 2)
	{
		CString l;
		int brk = m_print_data.FindOneOf(_T("\r\n"));
		if (brk == -1)
		{
			l = m_print_data;
			m_print_data = "";
			done = true;
		}
		else
		{
			l = m_print_data.Left(brk);

			if (brk < m_print_data.GetLength())
			{
				TCHAR s1 = m_print_data[brk];
				TCHAR s2 = m_print_data[brk + 1];
				if (s1 != s2 && (s1 == '\r' || s2 == '\n'))
				{
					++brk;
				}
			}
			m_print_data = m_print_data.Mid(brk + 1);

		}

		if (line > m_printing_line)
		{
			printLine(pDC, l, print);
		}
		++line;
	}

	pDC->SelectObject(old_font);
}

void CTextEditView::printLine(CDC *pDC, const TCHAR *s, bool print)
{
	CSize sz;
	sz = pDC->GetTextExtent(s);

	if (print)
	{
		pDC->TextOut(m_print_margin, m_printing_y_pos, s);
	}

	m_printing_y_pos += (sz.cy * 3) / 2;
	++m_printing_line;
}

void CTextEditView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (!bActivate)
	{
		writeBack();
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
