/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net
 * Copyright:	© 1994-2019 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "details.h"
#include "tinycadregistry.h"
#include "colour.h"

const int CDetails::M_NBOXWIDTH = 400;
const int CDetails::M_NLINEHEIGHT = 18;
const int CDetails::M_NRULERHEIGHT = 15;
const int CDetails::M_NPIXELSPERMM = 5;
const CSize CDetails::M_SZMAX(297 * M_NPIXELSPERMM, 210 * M_NPIXELSPERMM);

//-------------------------------------------------------------------------
CDetails::CDetails()
{
	Init();
}
//-------------------------------------------------------------------------
CDetails::~CDetails()
{
}

//-------------------------------------------------------------------------
void CDetails::Init()
{
	m_bIsVisible = true;
	m_szLastChange = ""; // CTime::GetCurrentTime();
	m_bHasRulers = CTinyCadRegistry::GetBool("ShowDesignRulers", false);
	m_iHorizRulerSize = CTinyCadRegistry::GetInt("HorizRulerSize", 7);
	m_iVertRulerSize = CTinyCadRegistry::GetInt("VertRulerSize", 5);

	Reset();
}
//-------------------------------------------------------------------------
void CDetails::Reset()
{
	m_szPage = M_SZMAX;
	m_sTitle = "";
	m_sAuthor = "";
	m_sRevision = "1.0";
	m_sDocNo = "";
	m_sOrg = "";
	m_sSheets = "1 of 1";
	m_szPage = CTinyCadRegistry::GetPageSize();
}
//-------------------------------------------------------------------------
bool CDetails::IsVisible() const
{
	return m_bIsVisible;
}
//-------------------------------------------------------------------------
bool CDetails::HasRulers() const
{
	return m_bHasRulers;
}
//-------------------------------------------------------------------------
bool CDetails::IsPortrait() const
{
	return m_szPage.cx < m_szPage.cy;
}
//-------------------------------------------------------------------------
CString CDetails::GetLastChange() const
{
	return m_szLastChange;
}
//-------------------------------------------------------------------------
CString CDetails::GetTitle() const
{
	return m_sTitle;
}
//-------------------------------------------------------------------------
CString CDetails::GetAuthor() const
{
	return m_sAuthor;
}
//-------------------------------------------------------------------------
CString CDetails::GetRevision() const
{
	return m_sRevision;
}
//-------------------------------------------------------------------------
CString CDetails::GetDocumentNumber() const
{
	return m_sDocNo;
}
//-------------------------------------------------------------------------
CString CDetails::GetOrganisation() const
{
	return m_sOrg;
}
//-------------------------------------------------------------------------
CString CDetails::GetSheets() const
{
	return m_sSheets;
}
//-------------------------------------------------------------------------
// Return the page boundries in a CPoint
CPoint CDetails::GetPageBoundsAsPoint() const
{
	return CPoint(m_szPage);
}
//---------------------------------------------------------------------
CDPoint CDetails::GetOverlap() const
{
	return CDPoint(m_szPage.cx / 10.0, m_szPage.cy / 10.0);
}
//-------------------------------------------------------------------------
// Return the page boundries in a CRect
CRect CDetails::GetPageBoundsAsRect() const
{
	return CRect(CPoint(), m_szPage);
}
//-------------------------------------------------------------------------
void CDetails::SetVisible(bool bIsVisible)
{
	m_bIsVisible = bIsVisible;
}
//-------------------------------------------------------------------------
void CDetails::SetRulers(bool bHasRulers, int v, int h)
{
	m_bHasRulers = bHasRulers;

	if (h >= 1 && h <= 26)
	{
		m_iHorizRulerSize = h;
	}
	if (v >= 1 && v <= 26)
	{
		m_iVertRulerSize = v;
	}

	CTinyCadRegistry::Set("ShowDesignRulers", m_bHasRulers);
	CTinyCadRegistry::Set("HorizRulerSize", m_iHorizRulerSize);
	CTinyCadRegistry::Set("VertRulerSize", m_iVertRulerSize);
}
//-------------------------------------------------------------------------
void CDetails::SetLastChange(const TCHAR * szLastChange)
{
	m_szLastChange = szLastChange;
}
//-------------------------------------------------------------------------
void CDetails::SetTitle(CString sTitle)
{
	m_sTitle = sTitle;
}
//-------------------------------------------------------------------------
void CDetails::SetAuthor(CString sAuthor)
{
	m_sAuthor = sAuthor;
}
//-------------------------------------------------------------------------
void CDetails::SetRevision(CString sRevision)
{
	m_sRevision = sRevision;
}
//-------------------------------------------------------------------------
void CDetails::SetDocumentNumber(CString sDocNo)
{
	m_sDocNo = sDocNo;
}
//-------------------------------------------------------------------------
void CDetails::SetOrganisation(CString sOrg)
{
	m_sOrg = sOrg;
}
//-------------------------------------------------------------------------
void CDetails::SetSheets(CString sSheets)
{
	m_sSheets = sSheets;
}
//-------------------------------------------------------------------------
//-- Set the page boundries from a CPoint
void CDetails::SetPageBounds(CPoint ptBounds)
{
	m_szPage = ptBounds;
}
//-------------------------------------------------------------------------
//-- Update the page boundries etc, using a printer device context
void CDetails::SetPageBounds(PRINTDLG& pd)
{
	CDC* dc = CDC::FromHandle(pd.hDC);

	// Change the scaling to print correct size
	m_szPage.cx = dc->GetDeviceCaps(HORZSIZE) * PIXELSPERMM;
	m_szPage.cy = dc->GetDeviceCaps(VERTSIZE) * PIXELSPERMM;
}
//-------------------------------------------------------------------------
void CDetails::Read(CStream& oArchive)
{
	Init();
	BYTE readb;
	LONG l;

	oArchive >> m_szPage.cx;
	oArchive >> m_szPage.cy;
	oArchive >> m_sTitle;
	oArchive >> m_sAuthor;
	oArchive >> m_sRevision;
	oArchive >> m_sDocNo;
	oArchive >> m_sOrg;
	oArchive >> m_sSheets;
	oArchive >> readb;
	oArchive >> l;

	CTime d(l);
	m_szLastChange = d.Format("%d %B %Y");
	m_bIsVisible = readb != 0;
}
//-------------------------------------------------------------------------
void CDetails::ReadEx(CStream& oArchive)
{
	BYTE nShows;
	CString sDate;

	Init();

	oArchive >> m_szPage.cx;
	oArchive >> m_szPage.cy;
	oArchive >> m_sTitle;
	oArchive >> m_sAuthor;
	oArchive >> m_sRevision;
	oArchive >> m_sDocNo;
	oArchive >> m_sOrg;
	oArchive >> m_sSheets;
	oArchive >> nShows;
	oArchive >> sDate;

	SetLastChange(sDate);
	SetVisible( (nShows & 1) != 0);
	SetRulers( (nShows & 2) != 0, 5, 7);
}
//-------------------------------------------------------------------------
//-- Load a design from a file, loaded design will be selected
void CDetails::ReadXML(CXMLReader& xml, TransformSnap& oSnap)
{
	CString sName;

	xml.intoTag();

	while (xml.nextTag(sName))
	{
		if (sName == _T("Size"))
		{
			xml.getAttribute(_T("width"), m_szPage.cx);
			xml.getAttribute(_T("height"), m_szPage.cy);
		}
		else if (sName == _T("TITLE"))
		{
			xml.getChildData(m_sTitle);
		}
		else if (sName == _T("AUTHOR"))
		{
			xml.getChildData(m_sAuthor);
		}
		else if (sName == _T("REVISION"))
		{
			xml.getChildData(m_sRevision);
		}
		else if (sName == _T("DOCNUMBER"))
		{
			xml.getChildData(m_sDocNo);
		}
		else if (sName == _T("ORGANISATION"))
		{
			xml.getChildData(m_sOrg);
		}
		else if (sName == _T("SHEETS"))
		{
			xml.getChildData(m_sSheets);
		}
		else if (sName == _T("SHOWS"))
		{
			int nShows = 0;
			xml.getChildData(nShows);
			m_bHasRulers = (nShows & 2) != 0;
			SetVisible( (nShows & 1) != 0);
		}
		else if (sName == _T("GUIDES"))
		{
			xml.getAttribute(_T("horiz"), m_iHorizRulerSize);
			xml.getAttribute(_T("vert"), m_iVertRulerSize);
		}
		else if (sName == _T("DATE"))
		{
			CString sLastChange;

			xml.getChildData(sLastChange);
			SetLastChange(sLastChange);
		}
		else if (sName == _T("GRID"))
		{
			oSnap.LoadXML(xml);
		}
	}
}
//-------------------------------------------------------------------------
void CDetails::WriteXML(CXMLWriter& xml) const
{
	int nShows = 0;

	if (m_bIsVisible)
	{
		nShows |= 1;
	}

	if (m_bHasRulers)
	{
		nShows |= 2;
	}

	xml.addTag(_T("Size"));
	xml.addAttribute(_T("width"), m_szPage.cx);
	xml.addAttribute(_T("height"), m_szPage.cy);
	xml.closeTag();

	xml.addTag(_T("GUIDES"));
	xml.addAttribute(_T("horiz"), m_iHorizRulerSize);
	xml.addAttribute(_T("vert"), m_iVertRulerSize);
	xml.closeTag();

	xml.addTag(_T("TITLE"), m_sTitle);
	xml.addTag(_T("AUTHOR"), m_sAuthor);
	xml.addTag(_T("REVISION"), m_sRevision);
	xml.addTag(_T("DOCNUMBER"), m_sDocNo);
	xml.addTag(_T("ORGANISATION"), m_sOrg);
	xml.addTag(_T("SHEETS"), m_sSheets);
	xml.addTag(_T("SHOWS"), nShows);
	xml.addTag(_T("DATE"), m_szLastChange);
}
//-------------------------------------------------------------------------
// Draw the details box
void CDetails::Display(CContext& dc, COption& oOption, CString sPathName) const
{
	DisplayBox(dc, oOption, sPathName);
	DisplayRulers(dc, oOption);
}
//-------------------------------------------------------------------------
// Draw the details box
void CDetails::DisplayBox(CContext& dc, COption& oOption, CString sPathName) const
{
	// Do we display the details?
	if (m_bIsVisible)
	{
		// Select the correct pen
		dc.SelectPen(PS_SOLID, 1, cBLACK);

		// Select the correct font
		dc.SelectFont(*oOption.GetFont(fTEXT), 3);

		// Draw the box to house it all
		CDPoint tl = CDPoint(m_szPage.cx - M_NBOXWIDTH - 2, m_szPage.cy - M_NLINEHEIGHT * 9 - 2);
		CDPoint br = CDPoint(m_szPage.cx - 2, m_szPage.cy - 2);

		// Move if necessary
		if (m_bHasRulers)
		{
			tl -= CDPoint(M_NRULERHEIGHT, M_NRULERHEIGHT);
			br -= CDPoint(M_NRULERHEIGHT, M_NRULERHEIGHT);
		}

		int LineHeight = M_NLINEHEIGHT;
		int TextSpace = LineHeight / 2;
		double BottomRow = (br.x - tl.x) / 3;
		double MiddleRow = br.x - (br.x - tl.x) / 5;

		// Now draw the outline
		dc.MoveTo(tl);
		dc.LineTo(CDPoint(br.x, tl.y));
		dc.LineTo(br);
		dc.LineTo(CDPoint(tl.x, br.y));
		dc.LineTo(tl);

		// Draw the horizontal lines
		dc.MoveTo(CDPoint(tl.x, tl.y + LineHeight * 2));
		dc.LineTo(CDPoint(br.x, tl.y + LineHeight * 2));
		dc.MoveTo(CDPoint(tl.x, tl.y + LineHeight * 5));
		dc.LineTo(CDPoint(br.x, tl.y + LineHeight * 5));
		dc.MoveTo(CDPoint(tl.x, tl.y + LineHeight * 7));
		dc.LineTo(CDPoint(br.x, tl.y + LineHeight * 7));

		// Draw the vertical lines
		dc.MoveTo(CDPoint(MiddleRow, tl.y + LineHeight * 5));
		dc.LineTo(CDPoint(MiddleRow, tl.y + LineHeight * 7));
		dc.MoveTo(CDPoint(tl.x + BottomRow, tl.y + LineHeight * 7));
		dc.LineTo(CDPoint(tl.x + BottomRow, tl.y + LineHeight * 9));
		dc.MoveTo(CDPoint(MiddleRow, tl.y + LineHeight * 7));
		dc.LineTo(CDPoint(MiddleRow, tl.y + LineHeight * 9));

		// Add the text
		dc.SetTextAlign(TA_LEFT | TA_BOTTOM | TA_NOUPDATECP);
		dc.SetTextColor(cBLACK);
		dc.SetROP2(R2_COPYPEN);
		dc.SetBkMode(TRANSPARENT);

		dc.TextOut(tl.x + TextSpace, tl.y + LineHeight, _T("Title"));
		dc.TextOut(tl.x + TextSpace, tl.y + LineHeight * 3, _T("Author"));
		dc.TextOut(tl.x + TextSpace, tl.y + LineHeight * 6, _T("File"));
		dc.TextOut(tl.x + TextSpace, tl.y + LineHeight * 8, _T("Revision"));
		dc.TextOut(MiddleRow + TextSpace, tl.y + LineHeight * 6, _T("Document"));
		dc.TextOut(tl.x + BottomRow + TextSpace, tl.y + LineHeight * 8, _T("Date"));
		dc.TextOut(MiddleRow + TextSpace, tl.y + LineHeight * 8, _T("Sheets"));

		// Add the actual data!
		dc.TextOut(tl.x + TextSpace * 2, tl.y + LineHeight * 2, m_sTitle);
		dc.TextOut(tl.x + TextSpace * 2, tl.y + LineHeight * 4, m_sAuthor);
		dc.TextOut(tl.x + TextSpace * 2, tl.y + LineHeight * 5, m_sOrg);


		// Display the file path name
		dc.TextOut(tl.x + TextSpace * 2, tl.y + LineHeight * 7, static_cast<int> (MiddleRow - tl.x - TextSpace * 4), sPathName);


		dc.TextOut(tl.x + TextSpace * 2, tl.y + LineHeight * 9, m_sRevision);
		dc.TextOut(MiddleRow + TextSpace * 2, tl.y + LineHeight * 7, m_sDocNo);
		dc.TextOut(tl.x + BottomRow + TextSpace * 2, tl.y + LineHeight * 9, GetLastChange());
		dc.TextOut(MiddleRow + TextSpace * 2, tl.y + LineHeight * 9, m_sSheets);
	}
}
//-------------------------------------------------------------------------
// Draw the design rulers
void CDetails::DisplayRulers(CContext& dc, COption& oOption) const
{
	if (m_bHasRulers)
	{
		// Select the correct pen
		dc.SelectPen(PS_SOLID, 1, cLINE);

		// Draw the outline box
		CDPoint tl1 = CDPoint(0, 0);
		CDPoint tl2 = CDPoint(M_NRULERHEIGHT + 2, M_NRULERHEIGHT + 2);
		CDPoint br2 = CDPoint(m_szPage.cx - M_NRULERHEIGHT - 2, m_szPage.cy - M_NRULERHEIGHT - 2);
		CDPoint br1 = CDPoint(m_szPage.cx - 2, m_szPage.cy - 2);

		// Draw the 4 lines that make up the outer boarder
		dc.MoveTo(tl1);
		dc.LineTo(CDPoint(br1.x, tl1.y));
		dc.LineTo(br1);
		dc.LineTo(CDPoint(tl1.x, br1.y));
		dc.LineTo(tl1);

		// Draw the 4 lines that make up the inner boarder
		dc.MoveTo(tl2);
		dc.LineTo(CDPoint(br2.x, tl2.y));
		dc.LineTo(br2);
		dc.LineTo(CDPoint(tl2.x, br2.y));
		dc.LineTo(tl2);

		// Now draw the cross bracings
		dc.MoveTo(tl1);
		dc.LineTo(tl2);
		dc.MoveTo(br1);
		dc.LineTo(br2);
		dc.MoveTo(CDPoint(br1.x, tl1.y));
		dc.LineTo(CDPoint(br2.x, tl2.y));
		dc.MoveTo(CDPoint(tl1.x, br1.y));
		dc.LineTo(CDPoint(tl2.x, br2.y));

		// Now lay out in a 5 x 7 layout...
		dc.SetTextAlign(TA_CENTER | TA_BOTTOM | TA_NOUPDATECP);
		dc.SetTextColor(cBLACK);
		dc.SetROP2(R2_COPYPEN);
		dc.SetBkMode(TRANSPARENT);

		// Select the correct font
		dc.SelectFont(*oOption.GetFont(fPIN), 3);

		int cols = m_iHorizRulerSize;
		int rows = m_iVertRulerSize;

		if (IsPortrait())
		{
			cols = 5;
			rows = 7;
		}

		double split = (br1.x - tl1.x) / cols;
		for (int col = 0; col < cols; col++)
		{
			CString s;
			s.Format(_T("%d"), col + 1);

			dc.MoveTo(CDPoint(col * split, tl1.y));
			dc.LineTo(CDPoint(col * split, tl2.y));
			dc.TextOut(split * col + split / 2, tl2.y - 2, s);

			dc.MoveTo(CDPoint(col * split, br1.y));
			dc.LineTo(CDPoint(col * split, br2.y));
			dc.TextOut(split * col + split / 2, br1.y - 2, s);

		}

		// Select the correct font
		dc.SelectFont(*oOption.GetFont(fPIN), 0);

		split = (br1.y - tl1.y) / rows;
		for (int row = 0; row < rows; row++)
		{
			CString s;
			s.Format(_T("%c"), 'A' + row);

			dc.MoveTo(CDPoint(tl1.x, row * split));
			dc.LineTo(CDPoint(tl2.x, row * split));
			dc.TextOut(tl2.x - 2, split * row + split / 2, s);

			dc.MoveTo(CDPoint(br1.x, row * split));
			dc.LineTo(CDPoint(br2.x, row * split));
			dc.TextOut(br1.x - 1, split * row + split / 2, s);
		}
	}
}
//-------------------------------------------------------------------------

