/*
 * Project:		TinyCAD program for schematic capture
 *				https://www.tinycad.net/
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCadView.h"
#include "option.h"
#include "HeaderStamp.h"
#include "TinyCad.h"

//-------------------------------------------------------------------------
void CTinyCadDoc::SaveXML(CXMLWriter& xml, BOOL Details, BOOL SaveSelect)
{
	SaveXML(xml, m_drawing, Details, SaveSelect);
}

//-------------------------------------------------------------------------
// Save the current design
BOOL CTinyCadDoc::Save(BOOL GetName, BOOL SaveSelect)
{

	CString theFileName = m_pParent->GetPathName();

	// Get the filename if necessary
	if (GetName)
	{
		selectCollection sel = m_selected;

		CFileDialog dlg(FALSE, _T("*.dsn"), NULL, OFN_HIDEREADONLY, _T("Design files (*.dsn)|*.dsn|All files (*.*)|*.*||"), AfxGetMainWnd());

		if (dlg.DoModal() != IDOK) return FALSE;

		theFileName = dlg.GetPathName();

		m_selected = sel;
	}

	// Show the busy icon
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	// Is this file to be saved as text or as a design file?
	CFile theFile;

	// Open the file for saving as a CFile for a CArchive
	BOOL r = theFile.Open(theFileName, CFile::modeCreate | CFile::modeWrite);

	if (r)
	{
		// Now save the file
		CStreamFile stream(&theFile, CArchive::store);

		CXMLWriter xml(&stream);
		SaveXML(xml, TRUE, SaveSelect);

		SetModifiedFlag(FALSE);
	}

	// Turn the cursor back to normal
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	// Was there an error opening the file?
	if (!r)
	// Could not open file to start saving
	Message(IDS_ABORTSAVE, MB_ICONEXCLAMATION);

	return r;
}

const CString CTinyCadDoc::GetXMLTag()
{
	return "TinyCAD";
}

//-------------------------------------------------------------------------
void CTinyCadDoc::SaveXML(CXMLWriter &xml, drawingCollection &drawing, BOOL Details, BOOL SaveSelect, BOOL SaveResources, BOOL SaveOriginObject)
{
	// Write the objects to the file
	try
	{
		xml.addTag(GetXMLTag());

		// If necessary write the header stamp
		if (Details)
		{
			xml.addTag(_T("NAME"));
			xml.addChildData(m_sheet_name);
			xml.closeTag();

			xml.addTag(_T("DETAILS"));
			m_oDetails.WriteXML(xml);
			m_snap.SaveXML(xml);
			xml.closeTag();
		}

		if (SaveResources)
		{
			// Find out which resources are in use

			for (drawingIterator i = drawing.begin(); i != drawing.end(); i++)
			{
				(*i)->TagResources();
			}

			// Save the resource details details
			theOptions.SaveFontsXML(xml);
			theOptions.SaveStylesXML(xml);
			theOptions.SaveFillStylesXML(xml);
			theOptions.SaveMetaFilesXML(xml);
		}

		// Only save the symbols if we are not saving
		// to a library or the header...
		if (Details)
		{
			theOptions.SaveSymbolsXML(xml);
			theOptions.WriteXML(xml);
		}

		// Save the document origin
		if (!SaveOriginObject)
		{
			SaveDocumentOriginXML(xml);
		}

		// Save symbol objects
		for (drawingIterator i = drawing.begin(); i != drawing.end(); i++)
		{
			CDrawingObject* obj = *i;

			if (obj->GetType() != xError && (Details || !obj->IsConstruction() || (SaveOriginObject && obj->GetType() == xOrigin)) && (!Details || obj->GetType() != xOrigin) && (!SaveSelect || IsSelected(obj)))
			{

				// Now save the actual object data
				obj->SaveXML(xml);
			}
		}

		xml.closeTag();

	} catch (CException *e)
	{
		// Could not save the file properly
		e->ReportError();
		e->Delete();
	}
}

BOOL CTinyCadDoc::ReadFile(CStream &theArchive)
{
	drawingCollection drawing;

	if (ReadFile(theArchive, TRUE, drawing))
	{
		m_drawing.insert(m_drawing.end(), drawing.begin(), drawing.end());
		UnSelect();
		SetModifiedFlag(FALSE);

		return TRUE;
	}

	return FALSE;
}

BOOL CTinyCadDoc::ReadFileXML(CXMLReader &xml, BOOL AlreadyStarted)
{
	drawingCollection drawing;

	if (ReadFileXML(xml, TRUE, drawing, AlreadyStarted))
	{
		m_drawing.insert(m_drawing.end(), drawing.begin(), drawing.end());
		UnSelect();
		SetModifiedFlag(FALSE);

		return TRUE;
	}

	return FALSE;
}

//-- Load a design from a file, loaded design will be selected
BOOL CTinyCadDoc::ReadFile(CStream &theArchive, BOOL Details, drawingCollection &drawing)
{
	try
	{
		// Save the old layer setting
		CDrawingObject* obj = NULL;
		BYTE tp = xNULL;
		CHeaderStamp oHeader;

		LONG pos = theArchive.GetPos();

		oHeader.Read(theArchive);

		if (!oHeader.IsChecked(false))
		{
			// Perhaps this is XML?
			theArchive.Seek(pos);
			CXMLReader xml(&theArchive);
			return ReadFileXML(xml, Details, drawing, FALSE);
		}

		while (tp != xEndFile)
		{
			theArchive >> tp;

			switch (tp)
			{
				case xDesignInformation:
					GetDetails().Read(theArchive);
					break;
				case xDesignInformation2:
					GetDetails().ReadEx(theArchive);
					m_snap.Load(theArchive, Details);
					break;
				case xOptions:
					if (Details)
					{
						theOptions.ReadNative(theArchive);
					}
					else
					{
						COption().ReadNative(theArchive);
					}
					obj = NULL;
					break;
				case xFont:
					theOptions.LoadFonts(theArchive);
					obj = NULL;
					break;
				case xLineStyle:
					theOptions.LoadStyles(theArchive);
					obj = NULL;
					break;
				case xFillStyle:
					theOptions.LoadFillStyles(theArchive);
					obj = NULL;
					break;
				case xMetaFiles:
					theOptions.LoadMetaFiles(theArchive);
					obj = NULL;
					break;
				case xSymbols:
					theOptions.LoadSymbols(theArchive);
					obj = NULL;
					break;
				case xLayerTable:
				{
					// Read in the number of Layers in this list
					CString theNewName;
					WORD NumberOfResources;
					theArchive >> NumberOfResources;

					hRESOURCE OldResourceNumber = 0;

					while (NumberOfResources > 0)
					{
						theArchive >> OldResourceNumber;
						theArchive >> theNewName;
						NumberOfResources--;
					}
					obj = NULL;
				}
					break;
				case xRuler:
					obj = new CDrawRuler(this, FALSE);
					break;
				case xBus:
					obj = new CDrawLine(this, xBus);
					break;
				case xWire:
					obj = new CDrawLine(this, xWire);
					break;
				case xBusName:
					obj = new CDrawText(this, xBusName);
					break;
				case xBusNameEx:
					obj = new CDrawText(this, xBusNameEx);
					break;
				case xLabel:
					obj = new CDrawLabel(this);
					break;
				case xLabelEx:
					obj = new CDrawLabel(this);
					break;
				case xLabelEx2:
					obj = new CDrawLabel(this);
					break;
				case xJunction:
					obj = new CDrawJunction(this);
					break;
				case xNoConnect:
					obj = new CDrawNoConnect(this);
					break;
				case xBusSlash:
					obj = new CDrawBusSlash(this);
					break;
				case xPower:
					obj = new CDrawPower(this);
					break;
				case xPin:
				case xPinEx:
					obj = new CDrawPin(this);
					break;
				case xLine:
					obj = new CDrawPolygon(this, xLine);
					break;
				case xLineEx:
					obj = new CDrawPolygon(this, xLineEx);
					break;
				case xLineEx2:
					obj = new CDrawPolygon(this, xLineEx2);
					break;
				case xPolygon:
					obj = new CDrawPolygon(this, xPolygon);
					break;
				case xDash:
					obj = new CDrawPolygon(this, xDash);
					break;
				case xText: // The old text type
					obj = new CDrawText(this, xText);
					break;
				case xTextEx:
					obj = new CDrawText(this, xTextEx);
					break;
				case xTextEx2:
					obj = new CDrawText(this, xTextEx2);
					break;
				case xCircle:
				case xCircleEx:
				case xCircleEx2:
				case xCircleEx3:
					obj = new CDrawSquare(this, xCircleEx3);
					break;
				case xSquare:
				case xSquareEx:
				case xSquareEx2:
				case xSquareEx3:
					obj = new CDrawSquare(this, xSquareEx3);
					break;
				case xNoteText:
					obj = new CDrawNoteText(this,xNoteText);
					break;
				case xArc:
				case xArcEx:
				case xArcEx2:
					obj = new CDrawPolygon(this, static_cast<ObjType> (tp));
					break;
				case xMethod: // The old method type
				case xMethodEx:
				case xMethodEx2:
				case xMethodEx3:
					obj = new CDrawMethod(this);
					break;
				case xTag:
					obj = new CDrawTag(this);
					break;
				case xMetaFile:
					obj = new CDrawMetaFile(this);
					break;
				default:
					obj = NULL;
					break;
			}

			if (obj != NULL)
			{
				// Provide special handling for old objects
				switch (tp)
				{
					case xSquare:
					case xSquareEx:
					case xCircle:
					case xCircleEx:
						((CDrawSquare *) obj)->OldLoad(theArchive, tp);
						break;
					case xCircleEx2:
					case xSquareEx2:
						((CDrawSquare *) obj)->OldLoad2(theArchive);
						break;
					case xMethod:
						((CDrawMethod *) obj)->OldLoad(theArchive);
						break;
					case xMethodEx:
						((CDrawMethod *) obj)->OldLoad2(theArchive);
						break;
					case xMethodEx2:
						((CDrawMethod *) obj)->OldLoad3(theArchive);
						break;
					case xPin:
						((CDrawPin *) obj)->OldLoad(theArchive);
						break;
					case xLabel:
					case xLabelEx:
					case xLabelEx2:
						((CDrawLabel *) obj)->Load(theArchive, (ObjType) tp);
						break;
					default:
						obj->Load(theArchive);
						break;
				}

				// Now add object to linked list
				if (!obj->IsEmpty())
				{
					drawing.insert(drawing.end(), obj);
				}
			}
		}

	}

	catch (CException *e)
	{
		CString s;
		CString msg;
		e->GetErrorMessage(msg.GetBuffer(256), 256, NULL);
		msg.ReleaseBuffer();
		s.Format(_T("Cannot load file.\r\n%s"), (LPCTSTR)msg);
		AfxMessageBox(s);
		e->Delete();
	}

	return drawing.size() > 0;
}

////// Load a design from a file, loaded design will be selected //////
BOOL CTinyCadDoc::ReadFileXML(CXMLReader &xml, BOOL Details, drawingCollection &drawing, BOOL AlreadyStarted)
{
	try
	{
		// Read in the first item...
		CString name;
		bool sheets = false;

		if (!AlreadyStarted)
		{
			xml.nextTag(name);
			if (name == "TinyCADSheets")
			{
				xml.intoTag();
				sheets = true;
				// We must search for the first TinyCAD section
				while (xml.nextTag(name) && name != GetXMLTag())
				{
				}
			}

			// Check we are at the right point
			if (name != _T("TinyCAD") && name != _T("HierarchicalSymbol"))
			{
				Message(IDS_ABORTVERSION, MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		BOOL ResetMerge = TRUE;
		CDPoint origin;

		xml.intoTag();

		while (xml.nextTag(name))
		{
			// Save the old layer setting
			CDrawingObject *obj = NULL;

			if (name == "DETAILS")
			{
				if (Details)
				{
					GetDetails().ReadXML(xml, m_snap);
				}
			}
			else if (name == "NAME")
			{
				if (Details)
				{
					xml.getChildData(m_sheet_name);
				}
			}
			else if (name == "GRID")
			{
				if (Details)
				{
					m_snap.LoadXML(xml);
				}
			}
			else if (name == "FONT")
			{
				if (ResetMerge)
				{
					theOptions.ResetMerge();
					ResetMerge = FALSE;
				}
				theOptions.LoadFontXML(xml);
			}
			else if (name == "STYLE")
			{
				if (ResetMerge)
				{
					theOptions.ResetMerge();
					ResetMerge = FALSE;
				}
				theOptions.LoadStyleXML(xml);
			}
			else if (name == "FILL")
			{
				if (ResetMerge)
				{
					theOptions.ResetMerge();
					ResetMerge = FALSE;
				}
				theOptions.LoadFillStyleXML(xml);
			}
			else if (name == "META" || name == "IMAGE")
			{
				if (ResetMerge)
				{
					theOptions.ResetMerge();
					ResetMerge = FALSE;
				}
				theOptions.LoadMetaFileXML(xml);
			}
			else if (name == "SYMBOLDEF")
			{
				if (ResetMerge)
				{
					theOptions.ResetMerge();
					ResetMerge = FALSE;
				}
				theOptions.LoadSymbolXML(xml);
			}
			else if (name == "OPTIONS")
			{
				theOptions.ReadXML(xml);
			}
			else if (name == CDrawBusSlash::GetXMLTag())
			{
				obj = new CDrawBusSlash(this);
			}
			else if (name == CDrawTag::GetXMLTag())
			{
				obj = new CDrawTag(this);
			}
			else if (name == CDrawJunction::GetXMLTag())
			{
				obj = new CDrawJunction(this);
			}
			else if (name == CDrawLine::GetXMLTag(xBus))
			{
				obj = new CDrawLine(this, xBus);
			}
			else if (name == CDrawLine::GetXMLTag(xWire))
			{
				obj = new CDrawLine(this, xWire);
			}
			else if (name == CDrawLine::GetXMLTag(xLineEx2))
			{
				obj = new CDrawLine(this, xLineEx2);
			}
			else if (name == CDrawMetaFile::GetXMLTag())
			{
				obj = new CDrawMetaFile(this);
			}
			else if (name == CDrawMethod::GetXMLTag())
			{
				obj = new CDrawMethod(this);
			}
			else if ( (name == CDrawHierarchicalSymbol::GetXMLTag()) || (name == CDrawHierarchicalSymbol::GetAltXMLTag())) //GetAltXMLTag() supports an old mispelled keyword
			{
				obj = new CDrawHierarchicalSymbol(this);
			}
			else if (name == CDrawNoConnect::GetXMLTag())
			{
				obj = new CDrawNoConnect(this);
			}
			else if (name == CDrawOrigin::GetXMLTag())
			{
				// Allow only a single origin object
				if (!origin.hasValue())
				{
					obj = new CDrawOrigin(this);
				}
			}
			else if (name == CDrawPin::GetXMLTag())
			{
				obj = new CDrawPin(this);
			}
			else if (name == CDrawRuler::GetXMLTag())
			{
				obj = new CDrawRuler(this, FALSE);
			}
			else if (name == CDrawSquare::GetXMLTag( TRUE ))
				{
					obj = new CDrawSquare(this, xSquareEx3);
				}
			else if (name == CDrawNoteText::GetXMLTag())
			{
				obj = new CDrawNoteText(this, xNoteText);
			}
			else if (name == CDrawSquare::GetXMLTag( FALSE ))
			{
				obj = new CDrawSquare(this, xCircleEx3);
			}
			else if (name == CDrawText::GetXMLTag(xTextEx2))
			{
				obj = new CDrawText(this, xTextEx2);
			}
			else if (name == CDrawText::GetXMLTag(xBusNameEx))
			{
				obj = new CDrawText(this, xBusNameEx);
			}
			else if (name == CDrawLabel::GetXMLTag())
			{
				obj = new CDrawLabel(this);
			}
			else if (name == CDrawPolygon::GetXMLTag())
			{
				obj = new CDrawPolygon(this);
			}
			else if (name == CDrawPower::GetXMLTag())
			{
				obj = new CDrawPower(this);
			}

			if (obj != NULL)
			{
				// Load this object...
				obj->LoadXML(xml);

				// Now add object to linked list
				if (!obj->IsEmpty())
				{
					drawing.insert(drawing.end(), obj);
					if (obj->GetType() == xOrigin)
					{
						// Capture the drawing origin
						origin = obj->m_point_a;
					}
				}
			}
		}

		xml.outofTag();

		if (sheets)
		{
			xml.outofTag();
		}

		// Set the document origin
		// only while editing symbols
		if (IsEditLibrary())
		{
			theOptions.SetOrigin(origin);
		}

	} catch (CException *e)
	{
		e->ReportError();
		e->Delete();
		return FALSE;
	}

	return drawing.size() > 0;
}

// Save the document origin as XML
void CTinyCadDoc::SaveDocumentOriginXML(CXMLWriter &xml)
{
	if (theOptions.HasOrigin())
	{
		// Save the drawing origin as an origin object
		CDPoint origin = theOptions.GetOrigin();
		xml.addTag(CDrawOrigin::GetXMLTag());
		xml.addAttribute(_T("pos"), CDPoint(origin));
		xml.closeTag();
	}
}

