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

#include "stdafx.h"
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "LineUtils.h"
#include "DSize.h"
#include ".\drawhierarchicalsymbol.h"
#include "TinyCadHierarchicalDoc.h"
#include "TinyCad.h"

CDrawHierarchicalSymbol::CDrawHierarchicalSymbol(CTinyCadDoc *pDesign) :
	CDrawMethod(pDesign)
{
	m_Loaded = false;
	rotate = 0;
}

CDrawHierarchicalSymbol::CDrawHierarchicalSymbol(const CDrawHierarchicalSymbol& b) :
	CDrawMethod(b.m_pDesign)
{
	*this = b;
}

CDrawHierarchicalSymbol::~CDrawHierarchicalSymbol(void)
{
	clearSymbol();
}

int CDrawHierarchicalSymbol::getMenuID()
{
	return IDM_TOOLHIERARCHICAL;
}

ObjType CDrawHierarchicalSymbol::GetType()
{
	return xHierarchicalSymbol;
}

const CString CDrawHierarchicalSymbol::GetXMLTag()
{
	return _T("HIERARCHICAL_SYMBOL");
}

const CString CDrawHierarchicalSymbol::GetAltXMLTag()
{ //Note:  Never "fix" the following misspelled text keyword!  It supports reading in older design files where the keyword actually was misspelled
	return _T("HIERACHICAL_SYMBOL"); //This keyword was once misspelled and needs this function to be able to recognize the old keyword in old drawing files.
}

// Get the definition of this symbol
BOOL CDrawHierarchicalSymbol::ExtractSymbol(CDPoint &tr, drawingCollection &method)
{
	if (!m_Loaded)
	{
		if (!Load(m_Filename))
		{
			m_point_b = CDPoint(m_tr.x + m_point_a.x, m_tr.y + m_point_a.y);
			return FALSE;
		}
	}

	method = m_Symbol;

	tr = CDPoint(m_tr.x * scaling_x, m_tr.y * scaling_y);

	// Calculate the bounding box
	if ( (rotate & 3) < 2)
	{
		m_point_b = CDPoint(tr.x + m_point_a.x, tr.y + m_point_a.y);
	}
	else
	{
		m_point_b = CDPoint(tr.y + m_point_a.x, tr.x + m_point_a.y);
	}

	return TRUE;
}

void CDrawHierarchicalSymbol::SetFilenameField()
{
	if (m_fields.size() < CDrawMethod::Name + 1)
	{
		m_fields.resize(CDrawMethod::Name + 1);
	}

	// Set the field for the user to see
	m_fields[CDrawMethod::Name].m_description = "Design";
	m_fields[CDrawMethod::Name].m_value = m_pDesign->formatFilename(m_Filename);
	m_fields[CDrawMethod::Name].m_type = default_show;
	m_fields[CDrawMethod::Name].m_show = TRUE;
}

void CDrawHierarchicalSymbol::clearSymbol()
{
	drawingCollection::iterator i = m_Symbol.begin();
	while (i != m_Symbol.end())
	{
		delete *i;
		++i;
	}

	m_Loaded = false;
	m_Symbol.clear();
}

BOOL CDrawHierarchicalSymbol::Load(const TCHAR *filename)
{
	// We are attempting a load
	m_Loaded = true;

	// Open the file
	CFile file;

	BOOL r = file.Open(filename, CFile::modeRead);
	if (!r)
	{
		CString s;
		s.Format(_T("File not found: %s"), filename);
		AfxMessageBox(s);
		m_tr = CDPoint(-50, -50);
		return FALSE;
	}

	// Use the XML loader
	CString name;
	CStreamFile stream(&file, CArchive::load);
	CXMLReader xml(&stream);

	xml.nextTag(name);

	if (name != "TinyCADSheets")
	{
		AfxMessageBox(IDS_NOHIERARCHICAL_SYMBOL, MB_ICONEXCLAMATION);
		m_tr = CDPoint(-50, -50);
		return FALSE;
	}

	// Now find the hierarchical symbol in this design
	xml.intoTag();

	bool found = false;
	while (xml.nextTag(name))
	{
		// Save the old layer setting
		//CDrawingObject *obj = NULL;

		if ( (name == _T("HierarchicalSymbol")) || (name == _T("HierachicalSymbol"))) //for historical reasons, the misspelled "HierachicalSymbol" must continue to be recognized
		{
			// Hierarchical symbol loader...
			clearSymbol();
			m_Loaded = true;
			m_pDesign->ReadFileXML(xml, FALSE, m_Symbol, TRUE);
			found = true;
		}
	}

	xml.outofTag();

	if (!found)
	{
		AfxMessageBox(IDS_NOHIERARCHICAL_SYMBOL, MB_ICONEXCLAMATION);
		return FALSE;
	}

	// Find the co-ords of the bounding box of this symbol
	CDPoint a = CDPoint(0, 0);
	CDPoint b = m_Symbol.front()->m_point_a;

	BOOL has_pins = FALSE;
	CDPoint pin = CDPoint(0, 0);

	drawingIterator it = m_Symbol.begin();
	while (it != m_Symbol.end())
	{
		CDrawingObject *pointer = *it;

		if (pointer->GetType() == xPinEx)
		{
			CDrawPin *thePin = static_cast<CDrawPin*> (pointer);

			if (!has_pins)
			{
				if (!thePin->IsInvisible())
				{
					// Use this for the pin-offset calculation
					pin = pointer->m_point_a;
					has_pins = TRUE;
				}
			}
		}

		a.x = max(a.x,max(pointer->m_point_a.x,pointer->m_point_b.x));
		a.y = max(a.y,max(pointer->m_point_a.y,pointer->m_point_b.y));

		b.x = min(b.x,min(pointer->m_point_a.x,pointer->m_point_b.x));
		b.y = min(b.y,min(pointer->m_point_a.y,pointer->m_point_b.y));

		++it;
	}

	// If this symbol has pins, then snap the pin to the grid,
	// rather than the bounding rect
	CDPoint snapa, snapb;
	CDPoint pin_offset = CDPoint(0, 0);

	// Snap the co-ords of the bounding box to the grid
	snapa = m_pDesign->m_snap.Snap(a);
	snapb = m_pDesign->m_snap.Snap(b);

	// Now make sure the pins stay on-grid
	if (has_pins)
	{
		// Snap the co-ords of the pin to the grid
		pin_offset = pin - m_pDesign->m_snap.Snap(pin);

		snapa = snapa + pin_offset;
		snapb = snapb + pin_offset;
	}

	// Make sure snapping is always positive
	if (snapa.x < a.x) snapa.x += m_pDesign->m_snap.GetGrid();
	if (snapa.y < a.y) snapa.y += m_pDesign->m_snap.GetGrid();
	if (snapb.x > b.x) snapb.x -= m_pDesign->m_snap.GetGrid();
	if (snapb.y > b.y) snapb.y -= m_pDesign->m_snap.GetGrid();

	// Now translate so that bottom left hand corner is at 0,0
	it = m_Symbol.begin();
	while (it != m_Symbol.end())
	{
		CDrawingObject *pointer = *it;
		pointer->m_point_a = CDPoint(pointer->m_point_a.x - snapa.x, pointer->m_point_a.y - snapa.y);
		pointer->m_point_b = CDPoint(pointer->m_point_b.x - snapa.x, pointer->m_point_b.y - snapa.y);

		++it;
	}

	// Store top right hand corner
	m_tr = CDPoint(snapb.x - snapa.x, snapb.y - snapa.y);

	// Set the filename
	m_Filename = filename;
	SetFilenameField();

	return TRUE;
}

BOOL CDrawHierarchicalSymbol::IsNoSymbol()
{
	if (!m_Loaded)
	{
		Load(m_Filename);
	}

	return m_Symbol.size() == 0;
}

// Open a file chooser to select the design
BOOL CDrawHierarchicalSymbol::SelectFile()
{
	CFileDialog dlg(TRUE, _T("*.dsn"), NULL, OFN_HIDEREADONLY, _T("Design files (*.dsn)|*.dsn|All files (*.*)|*.*||"), AfxGetMainWnd());

	if (dlg.DoModal() != IDOK) return FALSE;

	return Load(dlg.GetPathName());
}

// Store this method in the drawing
CDrawingObject* CDrawHierarchicalSymbol::Store()
{
	CDrawHierarchicalSymbol *NewObject;

	NewObject = new CDrawHierarchicalSymbol(m_pDesign);

	*NewObject = *this;

	m_pDesign->Add(NewObject);

	return NewObject;
}

void CDrawHierarchicalSymbol::LoadXML(CXMLReader &xml)
{
	xml.getAttribute(_T("pos"), m_point_a);
	xml.getAttribute(_T("rotate"), rotate);
	xml.getAttribute(_T("can_scale"), can_scale);
	xml.getAttribute(_T("scale_x"), scaling_x);
	xml.getAttribute(_T("scale_y"), scaling_y);
	xml.getAttribute(_T("file"), m_Filename);

	m_Filename = m_pDesign->unformatXMLFilename(m_Filename);

	xml.intoTag();
	int i = 0;
	CString name;
	while (xml.nextTag(name))
	{
		if (name == _T("FIELD"))
		{
			m_fields.resize(i + 1);
			CField &f = m_fields[i];

			int t = 0;
			xml.getAttribute(_T("type"), t);
			xml.getAttribute(_T("description"), f.m_description);
			xml.getAttribute(_T("value"), f.m_value);
			xml.getAttribute(_T("show"), f.m_show);
			xml.getAttribute(_T("pos"), f.m_position);
			f.m_type = static_cast<SymbolFieldType> (t);

			++i;
		}
	}
	xml.outofTag();

	m_segment = 0;
}

// Load and save to an XML file
void CDrawHierarchicalSymbol::SaveXML(CXMLWriter &xml)
{
	xml.addTag(GetXMLTag());

	xml.addAttribute(_T("pos"), CDPoint(m_point_a));
	xml.addAttribute(_T("rotate"), rotate);
	xml.addAttribute(_T("can_scale"), can_scale);
	xml.addAttribute(_T("scale_x"), scaling_x);
	xml.addAttribute(_T("scale_y"), scaling_y);
	xml.addAttribute(_T("file"), m_pDesign->formatXMLFilename(m_Filename));

	// Now read in the fields
	//int field_size = m_fields.size();;
	for (unsigned int i = 0; i < m_fields.size(); i++)
	{
		CField &f = m_fields[i];

		int t = static_cast<int> (f.m_type);

		xml.addTag(_T("FIELD"));
		xml.addAttribute(_T("type" ), t);
		xml.addAttribute(_T("description"), f.m_description);

		if (i == CDrawMethod::Name)
		{
			// We save the filename elsewhere...
			xml.addAttribute(_T("value"), _T(""));
		}
		else
		{
			xml.addAttribute(_T("value"), f.m_value);
		}

		xml.addAttribute(_T("show"), f.m_show);
		xml.addAttribute(_T("pos"), CDPoint(f.m_position));
		xml.closeTag();
	}

	xml.closeTag();
}

// Show this on the screen
void CDrawHierarchicalSymbol::Paint(CContext &dc, paint_options options)
{
	CDrawMethod::Paint(dc, options);

	if (IsNoSymbol())
	{
		dc.SelectBrush();
		dc.SelectPen(PS_SOLID, 0, cBLACK);
		dc.Rectangle(CDRect(m_point_a.x, m_point_a.y, m_point_b.x, m_point_b.y));
	}
}

void CDrawHierarchicalSymbol::BeginEdit(BOOL re_edit)
{
	g_EditToolBar.m_HierarchicalEdit.Open(m_pDesign, this);
}

void CDrawHierarchicalSymbol::EndEdit()
{
	RButtonDown(CDPoint(0, 0), CDPoint(0, 0));
	g_EditToolBar.m_HierarchicalEdit.Close();
}

int CDrawHierarchicalSymbol::GetContextMenu()
{
	return IDR_HIERARCHICAL_EDIT;
}

// Get the filename associated with this symbol
CString CDrawHierarchicalSymbol::GetFilename()
{
	return m_Filename;
}

void CDrawHierarchicalSymbol::ContextMenu(CDPoint p, UINT id)
{
	switch (id)
	{
		case ID_CONTEXT_OPENDESIGN:
			CTinyCadApp::EditDesign(GetFilename());
			break;
		case ID_CONTEXT_RELOADSYMBOLFROMDESIGN:
			Display();
			Load(GetFilename());
			Display();
			break;
	}
}

// The copy operator
CDrawHierarchicalSymbol& CDrawHierarchicalSymbol::operator=(const CDrawHierarchicalSymbol& b)
{
	m_point_a = b.m_point_a;
	m_point_b = b.m_point_b;
	m_segment = b.m_segment;
	m_pDesign = b.m_pDesign;
	part = b.part;
	rotate = b.rotate;
	can_scale = b.can_scale;
	m_fields = b.m_fields;
	scaling_x = b.scaling_x;
	scaling_y = b.scaling_y;

	clearSymbol();
	m_Loaded = false;
	m_Filename = b.m_Filename;
	m_tr = CDPoint(0, 0);

	return *this;
}

