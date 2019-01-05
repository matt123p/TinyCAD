/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net/
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "context.h"
#include "stream.h"
#include "xmlwriter.h"

//=========================================================================
class CDetails
{
private:
	static const int M_NBOXWIDTH;
	static const int M_NLINEHEIGHT;
	static const int M_NRULERHEIGHT;
	//-- How many pixels in each milimetre
	static const int M_NPIXELSPERMM;
	//-- The default size of the page
public:
	static const CSize M_SZMAX;
	// The size of the page
	CSize m_szPage;
	//-- Do we display the details box?
	bool m_bIsVisible;
	//-- Do we show the rulers at the side of the page?
	bool m_bHasRulers;
	//-- How many divisions in the horizontal ruler?
	int m_iHorizRulerSize;
	//-- How many divisions in the vertical ruler?
	int m_iVertRulerSize;
	//-- The date when this design was last edited/saved
	CString m_szLastChange;
	//-- The title of this design
	CString m_sTitle;
	//-- The author of this design
	CString m_sAuthor;
	//-- The revision code of this design
	CString m_sRevision;
	//-- The document number of this design
	CString m_sDocNo;
	//-- The organisation which designed this design
	CString m_sOrg;
	//-- The number of sheets in this design
	CString m_sSheets;
	CDetails();
	~CDetails();

private:
	void Init();

public:
	void Reset();
	void Read(CStream& oArchive);
	void ReadEx(CStream& oArchive);
	void ReadXML(CXMLReader& xml, TransformSnap& oSnap);
	void WriteXML(CXMLWriter& xml) const;

	//-- Draw the details box
private:
	void DisplayBox(CContext & dc, COption& oOption, CString sPathName) const;
	void DisplayRulers(CContext & dc, COption& oOption) const;

public:
	void Display(CContext & dc, COption& oOption, CString sPathName) const;
	bool IsVisible() const;
	bool HasRulers() const;
	bool IsPortrait() const;
	CString GetLastChange() const;
	CString GetTitle() const;
	CString GetAuthor() const;
	CString GetRevision() const;
	CString GetDocumentNumber() const;
	CString GetOrganisation() const;
	CString GetSheets() const;
	CPoint GetPageBoundsAsPoint() const;
	CDPoint GetOverlap() const;
	CRect GetPageBoundsAsRect() const;
	void SetVisible(bool bIsVisible);
	void SetRulers(bool bHasRulers, int v, int h);
	void SetLastChange(const TCHAR* szLastChange);
	void SetTitle(CString sTitle);
	void SetAuthor(CString sAuthor);
	void SetRevision(CString sRevision);
	void SetDocumentNumber(CString sDocNo);
	void SetOrganisation(CString sOrganisation);
	void SetSheets(CString sSheets);
	//-- Set the page boundries from a CPoint
	void SetPageBounds(CPoint ptBounds);
	//-- Update the page boundries etc, using a printer device context
	void SetPageBounds(PRINTDLG& pd);
};
//=========================================================================
