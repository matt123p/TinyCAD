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
#include <set>
#include <map>

#include "TinyCadView.h"
#include "TinyCad.h"
#include "special.h"
#include "library.h"
#include "net.h"
#include "TinyCadRegistry.h"
#include "diag.h"
#include "DlgPCBExport.h"
#include "MultiSheetDoc.h"
#include "TinyCadMultiDoc.h"

extern CDlgERCListBox theERCListBox;




////// Generate the net list for this design //////


void CTinyCadView::OnSpecialNet()
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.net"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy_s(ext, _T(".net"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space, _T(".net"));
		#endif
	}

	CDlgPCBExport dlg;
	dlg.m_Filename = szFile;

	if (dlg.DoModal() != IDOK)
		return;

	// Generate the net list file
	CNetList netlist;
	netlist.m_prefix_references = dlg.m_Prefix;
	netlist.WriteNetListFile( dlg.m_type, static_cast<CTinyCadMultiDoc*>(GetDocument()), dlg.m_Filename );

	// Now open the netlist for the user
	CTinyCadApp::EditTextFile( dlg.m_Filename );
}


////// Generate the SPICE output for this design //////


void CTinyCadView::OnSpecialCreatespicefile() 
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile,GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile,'.');
	if (!ext)
	{
		_tcscpy_s(szFile,_T("output.net"));
	}
	else
	{
		#ifdef USE_VS2003
			_tcscpy(ext, _T(".net"));
		#else
			size_t remaining_space = &szFile[255] - ext + 1;
			_tcscpy_s(ext, remaining_space, _T(".net"));
		#endif
	}

	CFileDialog dlg( FALSE, _T("*.net"), szFile, OFN_HIDEREADONLY,
		_T("SPICE (*.net)|*.net|All files (*.*)|*.*||"), AfxGetMainWnd() ); 

	if (dlg.DoModal() != IDOK)
	{
		return;
	}


	// Generate the SPICE file
	CNetList netlist;
	netlist.WriteSpiceFile( static_cast<CTinyCadMultiDoc*>(GetDocument()), dlg.GetPathName() );
}



////// Check the design rules for this design //////


////// The ERCBox dialog //////


BOOL CDlgERCBox::OnInitDialog()
{

  // Copy the errortest into the dialog
  CheckDlgButton(ERC_DUPREF,theErrorTest.DupRef);
  CheckDlgButton(ERC_UNCONNECT,theErrorTest.UnConnect);
  CheckDlgButton(ERC_NOCONNECT,theErrorTest.NoConnect);

  CheckDlgButton(ERC_POWER,theErrorTest.Power);
  CheckDlgButton(ERC_OUTPUTTOPWR,theErrorTest.OutputPwr);
  CheckDlgButton(ERC_OUTPUT,theErrorTest.Output);
  CheckDlgButton(ERC_NOUTPUT,theErrorTest.NoOutput);
  CheckDlgButton(ERC_UNCONNECTED,theErrorTest.UnConnected);

  return TRUE;
}


void CDlgERCBox::OnOK()
{

  // Copy the dialog into the errortest
  theErrorTest.DupRef		=IsDlgButtonChecked(ERC_DUPREF)!=0;
  theErrorTest.UnConnect	=IsDlgButtonChecked(ERC_UNCONNECT)!=0;
  theErrorTest.NoConnect	=IsDlgButtonChecked(ERC_NOCONNECT)!=0;

  theErrorTest.Power		=IsDlgButtonChecked(ERC_POWER)!=0;
  theErrorTest.OutputPwr	=IsDlgButtonChecked(ERC_OUTPUTTOPWR)!=0;
  theErrorTest.Output		=IsDlgButtonChecked(ERC_OUTPUT)!=0;
  theErrorTest.NoOutput		=IsDlgButtonChecked(ERC_NOUTPUT)!=0;
  theErrorTest.UnConnected	=IsDlgButtonChecked(ERC_UNCONNECTED)!=0;

  EndDialog( IDOK );
}




////// the menu entry point for this special function //////


// This table determines the type of the netlist
//
const int ErcTable[7][7] = {
// Next > Unknown		Passive,		Input,			Output,				TriState/BiDir		Power,			NoConnect,			
		  nUnknown,		nPassive,		nInput,			nOutput,			nBiDir,				nPower,			nNoConnect,		// Unknown
		  nPassive,		nPassive,		nPassive,		nOutput,			nBiDir,				nPower,			nNoConnect,		// Passive
		  nInput,		nPassive,		nInput,			nOutput,			nBiDir,				nPower,			nNoConnect,		// Input
		  nOutput,		nOutput,		nOutput,		ERR_OUTPUT,			ERR_OUTPUTBIDIR,	ERR_POWERBIDIR,	nNoConnect,		// Output
		  nBiDir,		nBiDir,			nBiDir,			ERR_OUTPUTBIDIR,	nBiDir,				ERR_POWERBIDIR,	nNoConnect,		// BiDir
		  nPower,		nPower,			nPower,			ERR_OUTPUTTOPWR,	ERR_POWERBIDIR,		nPower,			nNoConnect,		// Power
		  nNoConnect,	ERR_NOCONNECT,	ERR_NOCONNECT,	ERR_NOCONNECT,		ERR_NOCONNECT, 		ERR_NOCONNECT,	nNoConnect,		// NoConnect
};

void CTinyCadView::OnSpecialCheck()
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	CDlgERCBox theDialog;

	static union { ErrorTest e; WORD i; } theErrorTest;

	theErrorTest.i = CTinyCadRegistry::GetInt("ERC", 0xffff);


	// Get the user's options
	theDialog.SetErrorTest(theErrorTest.e);
	if (theDialog.DoModal() != IDOK)
	{
		return;
	}

	theErrorTest.e = theDialog.GetErrorTest();  
	CTinyCadRegistry::Set("ERC",theErrorTest.i);

	// Set the Busy icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	// Generate the netlist
	CNetList netlist;
	netlist.m_follow_imports = false;
	CTinyCadMultiDoc *pDoc = static_cast<CTinyCadMultiDoc*>(GetDocument());
	netlist.MakeNet( pDoc );
	netCollection *nets = &netlist.m_nets;

	// Delete all the errors which are currently in the design
	theERCListBox.Close();
	// GetCurrentDocument()->DeleteErrors();
	theERCListBox.Open( pDoc );
	int CurrentError = 0;

	// Scan the design for duplicated references
	if ((theErrorTest.e).DupRef)
	{
		std::set<CString>	refs;

		CString last = "";

		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++)
		{
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd()) 
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType()==xMethodEx3) 
				{
					CString ref = static_cast<CDrawMethod *>(pointer)->GetField(CDrawMethod::Ref);

					if (refs.find( ref ) != refs.end())
					{
						// We have a duplicate...
						CString buffer;
						buffer.LoadString( ERR_DUPREF );
						pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i),static_cast<CDrawMethod *>(pointer)->GetFieldPos(CDrawMethod::Ref),CurrentError++));
						theERCListBox.AddString(buffer);
					}
					else
					{
						refs.insert( ref );
					}
				}

				++ it;
			}
		}
	}


	// Scan netlist to determine the type of each net
	netCollection::iterator nit = nets->begin();

	while (nit != nets->end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		int theNetType = nUnknown;
		CString lastPower = "";
		int connections = 0;

		CDPoint pos;
		int sheet = 0;

		while (theNetType < ERR_BASE && nv_it != (*nit).second.end()) 
		{
			CNetListNode& theNode = *nv_it;

			CDrawingObject* pObject = theNode.m_parent;
			
			if (pObject != NULL) 
			{
				// Determine the type of this node
				int node_type = nUnknown;
				switch (pObject->GetType()) 
				{
				case xPower:
					node_type = nPower;
					if (lastPower=="")
					{
						lastPower = static_cast<CDrawPower *>(pObject)->GetValue();
					}
					else 
					{
						if ( lastPower != static_cast<CDrawPower *>(pObject)->GetValue() ) 
						{
							theNetType = ERR_POWER;
							node_type = nUnknown;
						}
					}
					connections ++;
					pos = pObject->m_point_a;
					sheet = theNode.m_sheet;
					break;
				case xNoConnect:
					node_type = 1;
					connections ++;
					pos = pObject->m_point_a;
					sheet = theNode.m_sheet;
					break;
				case xPin:
				case xPinEx:
					{
						CDrawPin *pPin = static_cast<CDrawPin*>( pObject );
						switch(pPin->GetElec()) 
						{
						case 0:		// Input
							node_type = nInput; 
							break;
						case 1:		// Output
							node_type = nOutput; 
							break;
						case 2:		// Tristate
							node_type = nBiDir; 
							break;
						case 3:		// Open Collector
							node_type = nBiDir; 
							break;
						case 4:		// Passive
							node_type = nPassive; 
							break;
						case 5:		// Input/Output
							node_type = nBiDir; 
							break;
						case 6:		// Not Connected
							node_type = nNoConnect;
							break;
						}
						
						if (pPin->IsPower()) 
						{
							node_type = nPower;
						}

						pos = pPin->GetActivePoint(theNode.m_pMethod);
						sheet = theNode.m_sheet;
						connections ++;
					}
					break;
				}
				
				theNetType = (node_type!=nUnknown) ? ErcTable[theNetType][node_type] : theNetType;
			}

			++ nv_it;
		}

		int ErrorNumber = 0;

		if (connections == 1)
		{
			theNetType = ERR_UNCONNECT;
		}
	

		switch (theNetType) 
		{
			case nUnknown:
				if (connections > 0)
				{
					ErrorNumber = ERR_UNCONNECTED;
				}
				break;
			case nInput:
				ErrorNumber = ERR_NOUTPUT;
				break;
			case nNoConnect:
				if (connections > 2)
				{
					theNetType = ERR_UNCONNECT;
				}
				break;
			default:
				ErrorNumber = theNetType;
				break;
		}

		// Is this error to be reported?
		switch (ErrorNumber) 
		{
		case ERR_UNCONNECT:
			if (!((theErrorTest.e).UnConnect))
				ErrorNumber = -1;
			break;
		case ERR_POWER:
			if (!((theErrorTest.e).Power))
				ErrorNumber = -1;
			break;
		case ERR_NOCONNECT:
			if (!((theErrorTest.e).NoConnect))
				ErrorNumber = -1;
			break;
		case ERR_NOUTPUT:
			if (!((theErrorTest.e).NoOutput))
				ErrorNumber = -1;
			break;
		case ERR_DUPREF:
			if (!((theErrorTest.e).DupRef))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUT:
			if (!((theErrorTest.e).Output))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUTTOPWR:
			if (!((theErrorTest.e).OutputPwr))
				ErrorNumber = -1;
			break;
		case ERR_UNCONNECTED:
			if (!((theErrorTest.e).UnConnected))
				ErrorNumber = -1;
			break;
		case ERR_OUTPUTBIDIR:
			if (!((theErrorTest.e).Output))
				ErrorNumber = -1;
			break;
		case ERR_POWERBIDIR:
			if (!((theErrorTest.e).OutputPwr))
				ErrorNumber = -1;
			break;
		}

		if (ErrorNumber >= ERR_BASE) 
		{
			CString buffer;
			buffer.LoadString( ErrorNumber );
			pDoc->GetSheet(sheet-1)->Add(new CDrawError(pDoc->GetSheet(sheet-1),pos,CurrentError++));
			theERCListBox.AddString(buffer);
		}

		++ nit;
	}


	// Were any errors detected?
	if (CurrentError == 0) 
	{
		CString buffer;
		buffer.LoadString( ERR_NOERROR );
		theERCListBox.AddString(buffer);
	}

	// Set the normal icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );

	// Re-Draw the window
	Invalidate();

}

