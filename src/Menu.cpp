/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2004 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#include "stdafx.h"
#include "TinyCadView.h"
#include "TinyCad.h"
#include "MainFrm.h"
#include "StreamClipboard.h"
#include "DlgAbout.h"
#include "DetailsPropertySheet.h"

// The menu commands!
// NOTE: many menu commands are in-line in main.h


//////// the FILE menu ////////


void CTinyCadView::OnFileDesign()
{
	CDetailsPropertySheet theDialog( GetDocument(), this );
	
  if (theDialog.DoModal() == IDOK)
  {
	GetCurrentDocument()->SetModifiedFlag( TRUE );
	GetCurrentDocument()->Invalidate();
  }
}


void CTinyCadView::OnFileImport() 
{ 
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument())); 
	GetCurrentDocument()->UnSelect(); 
	GetCurrentDocument()->Import( TRUE );
}


void CTinyCadView::OnFilePageSet()
{
  // Get rid of any drawing tool
  GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

  CDlgPageSizeBox theDialog(this,GetCurrentDocument()->GetDetails().GetPageBoundsAsPoint());
  if (theDialog.DoModal()==IDOK) {
	GetCurrentDocument()->GetDetails().SetPageBounds(theDialog.GetSize());
	Invalidate();
  }
}




////// The Find Menu //////

extern CDlgERCListBox theERCListBox;


// Find a string in this design
void CTinyCadView::OnFindFind()
{
	// Get rid of any drawing tool at this moment
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Do the dialog
	CDlgFindBox theDialog(this);

	if (theDialog.DoModal()!=IDOK)
		return;

	// Get the string to be found
	CString FindString = theDialog.GetString();

	// Now convert it to lower case only (as this will be a case insensive search)
	FindString.MakeLower();

	
	// Delete all the errors which are currently in the design
	theERCListBox.Close();
	// GetCurrentDocument()->DeleteErrors();

	CMultiSheetDoc *pDoc = static_cast<CMultiSheetDoc*>(GetDocument());
	theERCListBox.Open(pDoc);
	int CurrentError = 0;

	// Now look for this string
	CString theFoundString = "";

	for (int i = 0; i < pDoc->GetNumberOfSheets(); i++)
	{
		drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
		while (it != pDoc->GetSheet(i)->GetDrawingEnd()) 
		{
			CDrawingObject *pointer = *it;

			theFoundString = pointer->Find(FindString);
			if (theFoundString!="") {
				TCHAR buffer[STRLEN];
				_tcscpy_s(buffer,pointer->GetName());
				_tcscat_s(buffer,_T(": "));
				_tcscat_s(buffer,theFoundString);
				pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i),pointer->m_point_a,CurrentError++));
				theERCListBox.AddString(buffer);
			}
			
			++ it;
		}
	}

	// Was the string found?
	if (CurrentError == 0) {
		Message(IDS_NOFIND);
		theERCListBox.Close();
	}
}


//////// The VIEW menu ////////

void CTinyCadView::OnViewZoomIn()
{
  CMenu *pMenu = GetMenu();
  double NewZoom = GetTransform().doubleScale(1.0)*1.3;

  if (NewZoom < 20)
  {
	SetZoomFactor(NewZoom);
	// Centre at the mouse position
    SetScrollCentre(MousePosition);
  }
}

void CTinyCadView::OnViewZoomOut()
{
  double NewZoom = GetTransform().doubleScale(1.0)/1.3;
  if (NewZoom > 0.25)
    SetZoomFactor(NewZoom);
}

//////// The TOOLS menu /////////



// Get an object
void CTinyCadView::OnSelectGet()
{
  CLibraryStoreSymbol *theSymbol = static_cast<CMainFrame*>(AfxGetMainWnd())->GetSelectSymbol();
  if (theSymbol==NULL) 
  {
	return;
  }  

  GetCurrentDocument()->SelectSymbol( theSymbol );
}








//////// The Help Menu ////////

void CTinyCadView::OnAbout()
{
	CDlgAbout().DoModal();
}





////// The EDIT Menu //////



 void CTinyCadView::OnEditCut()
{
	if (GetCurrentDocument()->GetEdit()->GetType() != xEditItem)
		return;

	((CDrawEditItem *)GetCurrentDocument()->GetEdit())->ReleaseSelection();
	OnEditCopy();
	GetCurrentDocument()->BeginNewChangeSet();
	GetCurrentDocument()->SelectDelete();
	Invalidate();
}


// Paste...
void CTinyCadView::OnEditPaste()
{
	OpenClipboard();
	if (IsClipboardAvailable())
	{
		GetCurrentDocument()->BeginNewChangeSet();
		GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

		// Now open the file
		CStreamClipboard stream;

		if (stream.ReadFromClipboard( ClipboardFormat ))
		{
			GetCurrentDocument()->SelectObject( NULL );

			if (GetCurrentDocument()->Import(stream))
			{
				GetCurrentDocument()->PostPaste();
				CDrawBlockImport *pImport = new CDrawBlockImport(GetCurrentDocument());
				GetCurrentDocument()->SelectObject(pImport); 
				pImport->Import();
			}
			else
			{
				GetCurrentDocument()->SelectObject( new CDrawEditItem(GetCurrentDocument()) );	
			}
		}

	}
	else if (::IsClipboardFormatAvailable( CF_ENHMETAFILE ))
	{
		// It is a meta-file, we paste these as a new type of object
		HENHMETAFILE clipboard_data = (HENHMETAFILE)::GetClipboardData( CF_ENHMETAFILE );
		CDrawMetaFile *pObject = new CDrawMetaFile( GetCurrentDocument() );
		pObject->setMetaFile( clipboard_data );
		GetCurrentDocument()->AddImage( pObject );
	}
	else if (::IsClipboardFormatAvailable( CF_BITMAP ))
	{
		// Create a PNG image with this data
		CBitmap bitmap;
		bitmap.Attach( (HBITMAP)::GetClipboardData( CF_BITMAP ) ); 

		CDrawMetaFile *pObject = new CDrawMetaFile( GetCurrentDocument() );
		if (pObject->setBitmap( bitmap ))
		{
			GetCurrentDocument()->AddImage( pObject );
		}
	}
	else
	{
		AfxMessageBox(_T("Cannot paste"));
	}

	CloseClipboard();
}

void CTinyCadView::OnDestroyClipboard()
{
	CWnd::OnDestroyClipboard();	
}

BOOL CTinyCadView::IsClipboardAvailable()
{
	return ::IsClipboardFormatAvailable(ClipboardFormat);
}

void CTinyCadView::OnEditDuplicate()
{
	if (GetCurrentDocument()->GetEdit()->GetType() != xEditItem)
		return;
	OnEditCopy();
	OnEditPaste();
}

void CTinyCadView::OnEditCopy()
{
	if (GetCurrentDocument()->GetEdit()->GetType() != xEditItem)
		return;

	OpenClipboard();
	EmptyClipboard();

	CClientDC ref_dc(this);
	HENHMETAFILE hmeta = GetCurrentDocument()->CreateMetafile( ref_dc, NULL, false );

	if (hmeta)
	{
		SetClipboardData(CF_ENHMETAFILE,hmeta);
	}

	// Save the selected area
	CStreamClipboard stream;
	CXMLWriter xml( &stream );
	if (GetCurrentDocument()->IsSelected())
	{
		GetCurrentDocument()->SaveXML(xml,TRUE,TRUE);
	}
	else
	{
		GetCurrentDocument()->SaveXML(xml,TRUE,FALSE);
	}
	stream.SaveToClipboard(ClipboardFormat);

	CloseClipboard();
}


void CTinyCadView::OnEditCopyto() 
{
	if (GetCurrentDocument()->IsSelected())
	{
		GetCurrentDocument()->Save(TRUE,TRUE);
	}
}

void CTinyCadView::OnEditRotateLeft() 
{
	ChangeDir(2);
}


void CTinyCadView::OnEditRotateRight() 
{
	ChangeDir(3);
}


void CTinyCadView::OnEditFlip() 
{
	ChangeDir(4);
}



void CTinyCadView::OnSelectArc() 
{ 
	g_EditToolBar.m_DrawPolyEdit.SetArcAngle();
	GetCurrentDocument()->SelectObject(new CDrawPolygon(GetCurrentDocument())); 
}

void CTinyCadView::OnSelectPolygon() 
{ 
	g_EditToolBar.m_DrawPolyEdit.SetLineAngle();
	GetCurrentDocument()->SelectObject(new CDrawPolygon(GetCurrentDocument())); 
}

void CTinyCadView::OnSelectHierachical() 
{ 
	// Drop the current drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument())); 

	// Create a new symbol to work with...
	CDrawHierarchicalSymbol *pSymbol = new CDrawHierarchicalSymbol(GetCurrentDocument());

	if (pSymbol->SelectFile())
	{
		GetCurrentDocument()->SelectObject(pSymbol); 
	}
	else
	{
		delete pSymbol;
	}
}

void CTinyCadView::OnSelectJunction()
{ 
	if (!GetCurrentDocument()->GetOptions()->GetAutoJunc())
	{
		GetCurrentDocument()->SelectObject(new CDrawJunction(GetCurrentDocument())); 
	}
	else
	{
		AfxMessageBox( IDS_NOJUNCTION );
	}
}

