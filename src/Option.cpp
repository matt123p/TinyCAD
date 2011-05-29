/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCadView.h"
#include "Option.h"
#include "Registry.h"
#include "Colour.h"

//*************************************************************************
//*                                                                       *
//*  The option categories.                                               *
//*                                                                       *
//*************************************************************************

//=========================================================================
//== ctor/dtor/initializing                                              ==
//=========================================================================

//-------------------------------------------------------------------------
COption::COption()
{
	PinLength = 20;
	PinNumberPos = 0;
	origin = CDPoint();
}
//-------------------------------------------------------------------------
void COption::Init(CTinyCadDoc *pDesign)
{
	// Keep a track of the design
	m_pDesign = pDesign;

	// Init the font settings for normal text, pin text and ruler text
	theFontList = ListOfFonts(ListOfFonts::TEXT_HEIGHT, false);
	theFontList.Add(new ListOfFonts(ListOfFonts::PIN_HEIGHT, false));
	theFontList.Add(new ListOfFonts(ListOfFonts::RULE_HEIGHT, true));

	// Init the line settings for normal and dashed lines
	m_colLineStyles = ListOfStyles(PS_SOLID, 1, cBLACK);
	m_colLineStyles.Add(new ListOfStyles(PS_DASH, 1, cBLACK));

	// Init no fill and black fill styles
	theFillStyleList = ListOfFillStyles(-1, cBLACK);
	theFillStyleList.Add(new ListOfFillStyles(0, cBLACK));

	// Attempt to load the defaults for the grid
	GridShow = CRegistry::GetBool("ShowGrid", false);
	Units = CRegistry::GetInt("Units", 0);
	PinLength = CRegistry::GetInt("PinLength", 30);
	PinNumberPos = CRegistry::GetInt("PinNumberPos", 0);
	AutoDrag = CRegistry::GetBool("AutoDrag", true);
	AutoJunc = CRegistry::GetBool("AutoJunc", true);
	AutoSnap = CRegistry::GetBool("AutoSnap", true);
	AutoSnapRange = CRegistry::GetInt("AutoDragRange", 15);

	// Create the no symbol object
	CDesignFileSymbol *NoSymbol = new CDesignFileSymbol();
	NoSymbol->CreateNoSymbol(pDesign);
	theSymbolList = new ListOfSymbols(NoSymbol);
}

//-------------------------------------------------------------------------
COption::~COption()
{
	delete theSymbolList;
}
//-------------------------------------------------------------------------

//=========================================================================
//== Accessor                                                            ==
//=========================================================================

//-------------------------------------------------------------------------
/** Get the user defined colours. */
CUserColor& COption::GetUserColor()
{
	return m_oColors;
}
//-------------------------------------------------------------------------
/** Get the styles for line drawing. */
ListOfStyles& COption::GetLineStyles()
{
	return m_colLineStyles;
}
//-------------------------------------------------------------------------


void COption::UnTag()
{
	theFontList.UnTag();
	m_colLineStyles.UnTag();
	theFillStyleList.UnTag();
	theMetaFileList.UnTag();
	theSymbolList->UnTag();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

////// Some default settings for dialogs ////// 

void COption::SetPinLength(int pl)
{
	PinLength = pl;
	CRegistry::Set("PinLength", PinLength);
}

void COption::SetPinNumberPos(int pl)
{
	PinNumberPos = pl;
	CRegistry::Set("PinNumberPos", PinNumberPos);
}

////// Save the current settings along with the design //////

void COption::WriteXML(CXMLWriter &xml)
{
	xml.addTag(_T("OPTIONS"));

	// !!	xml.addTag( "FONT", CurrentFont )
	// !!	xml.addTag( "STYLE", CurrentStyle );
	xml.addTag(_T("GRID"), GridShow);
	xml.addTag(_T("UNITS"), Units);
	m_oColors.WriteXML(xml);

	xml.closeTag();
}

void COption::ReadXML(CXMLReader &xml)
{
	CString name;
	xml.intoTag();

	// Set the default colours (ignoring the registry)
	m_oColors.Init();

	while (xml.nextTag(name))
	{
		if (name == "FONT")
		{
			// !!		xml.getChildData( CurrentFont );
		}
		else if (name == "STYLE")
		{
			// !!		xml.getChildData( CurrentStyle );
		}
		else if (name == "GRID")
		{
			xml.getChildData(GridShow);
		}
		else if (name == "UNITS")
		{
			xml.getChildData(Units);
		}
		else
		{
			m_oColors.ReadXML(xml, name);
		}
	}

	xml.outofTag();
}

void COption::ReadNative(CStream& oArchive)
{
	LONG ScrollBar;
	hRESOURCE iCurrentFont, iCurrentStyle;
	oArchive >> iCurrentFont;
	oArchive >> iCurrentStyle;
	oArchive >> GridShow;
	oArchive >> Units;
	oArchive >> ScrollBar;

	// No longer supported
	// CurrentFont = GetNewFontNumber( CurrentFont );
	// CurrentStyle = GetNewStyleNumber( CurrentStyle );
}

////// The edit options //////

//-------------------------------------------------------------------------
//-- Change the size of a font and create a new entry
hFONT COption::ChangeFontSize(hFONT n, double delta_height, double width)
{
	LOGFONT Font = *GetFont(n);
	Font.lfHeight += static_cast<int> (delta_height);

	if (width != 0)
	{
		Font.lfWidth = static_cast<int> (width);
	}

	return AddFont(&Font);
}
//-------------------------------------------------------------------------
int COption::GetFontWidth(hFONT n)
{
	LOGFONT* Font = GetFont(n);

	// If we have an aspect ratio that is not 1.0 then we must
	// first find the "average" width of the font.  We can only
	// do this by first selecting the font with an aspect ratio
	// of 1.0 and measuring it.

	if (Font->lfWidth == 0)
	{
		CClientDC dc(AfxGetMainWnd());
		CFont f;
		f.CreateFontIndirect(Font);

		TEXTMETRIC metrics;

		CFont* old_font = dc.SelectObject(&f);
		dc.GetTextMetrics(&metrics);
		dc.SelectObject(old_font);
		Font->lfWidth = metrics.tmAveCharWidth;
	}

	return Font->lfWidth;
}
//-------------------------------------------------------------------------
void COption::ChooseFont(ObjType t, CDC &PrinterDC, HWND Window)
{
	CHOOSEFONT cf;
	LOGFONT aLogFont = * (GetFont(GetCurrentFont(t)));

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = Window;
	cf.hDC = PrinterDC.m_hDC;
	cf.lpLogFont = &aLogFont;
	cf.iPointSize = 10;
	cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_BOTH;
	cf.rgbColors = cBLACK;
	cf.lCustData = 0;
	cf.lpfnHook = NULL;
	cf.lpTemplateName = NULL;
	cf.hInstance = NULL;
	cf.lpszStyle = NULL;
	cf.nFontType = SCREEN_FONTTYPE;
	cf.nSizeMin = 5;
	cf.nSizeMax = 128;

	if (::ChooseFont(&cf))
	{
		CurrentFont[t] = AddFont(&aLogFont);
	}
}

////// The symbol operators //////

// Convert a CPoint to a Unit CString
CString COption::PointToDisplay(CDPoint a, BOOL horiz)
{
	CString r;

	double Scale = GetUnits() == 0 ? (float) PIXELSPERMM : ((float) PIXELSPERMM * 25.4f);
	double sx = a.x / Scale;
	double sy = a.y / Scale;
	if (horiz)
	{
		r.Format(_T("%9.03f"), sy, GetUnits() == 0 ? _T("mm") : _T("\""));
	}
	else
	{
		r.Format(_T("%9.03f"), sx, GetUnits() == 0 ? _T("mm") : _T("\""));
	}

	return r;
}

// Convert a CPoint to a Unit CString
CString COption::PointToUnit(CDPoint a)
{
	CString r;

	double Scale = GetUnits() == 0 ? (float) PIXELSPERMM : ((float) PIXELSPERMM * 25.4f);
	double sx = a.x / Scale;
	double sy = a.y / Scale;
	r.Format(_T("%9.03f,%9.03f%s"), sx, sy, GetUnits() == 0 ? _T(" mm") : _T(" \""));

	return r;
}
//-------------------------------------------------------------------------
void COption::ResetMerge()
{
	theFontList.ResetMerge();
	m_colLineStyles.ResetMerge();
	theFillStyleList.ResetMerge();
	theMetaFileList.ResetMerge();
	theSymbolList->ResetMerge();
}
//-------------------------------------------------------------------------
hFONT COption::GetCurrentFont(ObjType t)
{
	return CurrentFont[t];
}
//-------------------------------------------------------------------------
hFONT COption::AddFont(LOGFONT* lf, hFONT MergeNumber)
{
	return theFontList.Add(new ListOfFonts(*lf), MergeNumber);
}
//-------------------------------------------------------------------------

bool COption::HasOrigin()
{
	return origin.hasValue();
}

void COption::SetOrigin(CDPoint org)
{
	origin = org;
	if (!org.hasValue())
	{
		origin = org;
	}
}

CDPoint COption::GetOrigin()
{
	if (origin.hasValue())
	{
		return origin;
	}

	return CDPoint(0, 0);
}
