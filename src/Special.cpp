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
#include "special.h"
#include "library.h"
#include "TinyCad.h"
#include "DlgBOMExport.h"
#include "BOMGenerator.h"

////// The auto anotate special function //////

AnotateSetup theASetup;

//Compare positions of two objects
static bool DORefComp(CDrawingObject* o1,CDrawingObject* o2)
{
   CDPoint c1,c2;
   c1.x=(o1->m_point_a.x+o1->m_point_b.x)/2;
   c1.y=(o1->m_point_a.y+o1->m_point_b.y)/2;
   c2.x=(o2->m_point_a.x+o2->m_point_b.x)/2;
   c2.y=(o2->m_point_a.y+o2->m_point_b.y)/2;
   if(c1.y<c2.y)return true;
   if(c1.y>c2.y)return false;
   if(c1.x<c2.x)return true;
   return false;   
}

// Auto anotate the design
void CTinyCadView::OnSpecialAnotate()
{
  CDlgAnotateBox theDialog(this,theASetup);

  // Get rid of any drawing tool
  GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

  // Do the dialog
  int action = theDialog.DoModal();
  
  if (action == IDC_REF_PAINTER)
  {
	  theASetup = theDialog.v;
	  GetCurrentDocument()->SelectObject(new CDrawRefPainter(GetCurrentDocument(), theASetup.startval ));
	  return;
  }
  else if (action !=IDOK)
  {
	return;
  }

  theASetup = theDialog.v;

  // Set the busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );
  

  // Now add/remove references
  CDrawMethod *thisMethod;
  CSymbolRecord *thisSymbol;
  int value=0;
  int part=0;
  BOOL IsSet,IsMatch,MissingSymbols=FALSE;
  
	for (int whichPass = 0; whichPass < 2; whichPass++)
	{
		int sheet = theASetup.all_sheets ? 0 : GetDocument()->GetActiveSheetIndex();
      //copy all objects to new vector
      std::vector<CDrawingObject*> objs;
      objs.assign(GetDocument()->GetSheet(sheet)->GetDrawingBegin(), GetDocument()->GetSheet(sheet)->GetDrawingEnd());
      //sort it according to the objects positions
      std::sort(objs.begin(),objs.end(),DORefComp);
		do
		{
			//drawingIterator 
         std::vector<CDrawingObject*>::iterator it = objs.begin();
			while (it != objs.end()) 
			{
				CDrawingObject *pointer = *it;
				// Look for method objects
				ObjType objType = pointer->GetType();
				if (objType == xMethodEx3 || objType == xHierarchicalSymbol) 
				{
					thisMethod = static_cast<CDrawMethod *>(pointer);
					thisSymbol = thisMethod->GetSymbolData();

					// If there is no symbol then cannot modify this symbol!
					if (thisMethod->IsNoSymbol()) 
					{
						MissingSymbols = TRUE;
						++ it;
						continue;
					}

					// Has this symbol got a reference?
					IsSet   = thisMethod->HasRef();

					switch (theASetup.reference)
					{
					case AnotateSetup::ALL:		// All references
						IsMatch = TRUE;
						break;
					case AnotateSetup::UNNUMBERED:		// Un-numbered references
						IsMatch = !thisMethod->HasRef();
						break;
					case AnotateSetup::MATCHING:		// References that match...
						IsMatch = theASetup.matchval == thisSymbol->reference;
						break;
					}

					if (IsMatch)
					{

						// First pass  - we remove references if necessary,
						// Second pass - we add refences back in...
						//
						if (whichPass == 0)
						{
							// Remove any matching references (if necessary)
							if (IsSet && (theASetup.value!=AnotateSetup::SPECIFIED 
								|| thisMethod->GetRefVal()>=theASetup.startval) ) 
							{
								thisMethod->RemoveReference();
							}
						}
						else
						{
							// Now add back any references
							if (theASetup.action == AnotateSetup::ADD) 
							{
								/* OK, why the special case? 
								if (theASetup.reference != 1)
								{
									value = (theASetup.value == 0) ? 1 : theASetup.startval;
								}
								*/
								value = (theASetup.value == AnotateSetup::SPECIFIED) 
									? theASetup.startval : 1;
								thisMethod->AddReference( value, theASetup.all_sheets );
							}
						}
					}
				}

				++ it;
			}
			++ sheet;
		} while ( theASetup.all_sheets && sheet < GetDocument()->GetNumberOfSheets() );
	}

  // Where there any missing symbols?
  if (MissingSymbols)
	Message(IDS_MISSMETH,MB_ICONEXCLAMATION);


  // Restore the correct cursor
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );


  // Ensure the window is re-drawn
  GetCurrentDocument()->SetModifiedFlag( TRUE );
  Invalidate();  
}



// The Anotate Setup values class
AnotateSetup::AnotateSetup()
{
  action=ADD;
  reference=ALL;
  value=DEFAULT;
  matchval="U?";
  startval=1;
  all_sheets=false;
}





////// The Parts List (Bill of Materials) special function //////


void CTinyCadView::OnSpecialBom()
{
  // Get rid of any drawing tool
  GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));


	// Get the file in which to save the network
	TCHAR szFile[256];
	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.txt"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy(ext, _T(".txt"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space, _T(".txt"));
		#endif
	}

	// Get the file name for the parts list
	CDlgBOMExport dlg(this);
	dlg.m_Filename = szFile;

	if (dlg.DoModal() != IDOK)
	{
	  return;
	}

  FILE *theFile;
  errno_t err;

  err = _tfopen_s(&theFile, dlg.m_Filename,_T("w"));
  if ((theFile == NULL) || (err != 0)) 
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  // Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );


  CBOMGenerator	bom;
  bom.GenerateBomForDesign( dlg.m_All_Sheets != 0, 
	  dlg.m_All_Attrs != 0, dlg.m_Prefix != 0, dlg.m_Hierarchical != 0, GetDocument() );

  // Now generate the output file
  bom.WriteToFile( theFile, dlg.m_type == 1 );

  fclose(theFile);

  // Restore the normal cursor
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );

  // Where there any errors?
  if (bom.GetMissingRef())
	Message(IDS_MISSREF);

  CTinyCadApp::EditTextFile( dlg.m_Filename );
}




