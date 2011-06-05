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

	// Do ERC Check
	DoSpecialCheck(false);

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile, GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile, '.');
	if (!ext)
	{
		_tcscpy_s(szFile, _T("output.net"));
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

	if (dlg.DoModal() != IDOK) return;

	// Generate the net list file
	CNetList netlist;
	netlist.m_prefix_references = dlg.m_Prefix;
	netlist.WriteNetListFile(dlg.m_type, static_cast<CTinyCadMultiDoc*> (GetDocument()), dlg.m_Filename);

	// Now open the netlist for the user
	CTinyCadApp::EditTextFile(dlg.m_Filename);
}

////// Generate the SPICE output for this design //////


void CTinyCadView::OnSpecialCreatespicefile()
{
	// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	// Do ERC Check
	DoSpecialCheck(false);

	// Get the file in which to save the network
	TCHAR szFile[256];

	_tcscpy_s(szFile, GetDocument()->GetPathName());
	TCHAR* ext = _tcsrchr(szFile, '.');
	if (!ext)
	{
		_tcscpy_s(szFile, _T("output.net"));
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

	CFileDialog dlg(FALSE, _T("*.net"), szFile, OFN_HIDEREADONLY, _T("SPICE (*.net)|*.net|All files (*.*)|*.*||"), AfxGetMainWnd());

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	// Generate the SPICE file
	CNetList netlist;
	netlist.WriteSpiceFile(static_cast<CTinyCadMultiDoc*> (GetDocument()), dlg.GetPathName());
//	TRACE("Spice netlist file path = %S\n",dlg.GetPathName());
	// Now open the Spice netlist for the user
	CTinyCadApp::EditTextFile(dlg.GetPathName());
}

////// Check the design rules for this design //////


////// The ERCBox dialog //////


BOOL CDlgERCBox::OnInitDialog()
{

	// Copy the error test into the dialog
	CheckDlgButton(ERC_DUPREF, theErrorTest.DupRef);
	CheckDlgButton(ERC_UNCONNECT, theErrorTest.UnConnect);
	CheckDlgButton(ERC_NOCONNECT, theErrorTest.NoConnect);

	CheckDlgButton(ERC_POWER, theErrorTest.Power);
	CheckDlgButton(ERC_OUTPUTTOPWR, theErrorTest.OutputPwr);
	CheckDlgButton(ERC_OUTPUT, theErrorTest.Output);
	CheckDlgButton(ERC_NOUTPUT, theErrorTest.NoOutput);
	CheckDlgButton(ERC_UNCONNECTED, theErrorTest.UnConnected);
	CheckDlgButton(ERC_MULTIPLENETNAMES, theErrorTest.MultipleNetNames);
	CheckDlgButton(ERC_NONCASEDISTINCT, theErrorTest.NonCaseDistinctNetNames);
	CheckDlgButton(ERC_UNASSIGNEDREFDES, theErrorTest.UnAssignedRefDes);

	return TRUE;
}

void CDlgERCBox::OnOK()
{

	// Copy the dialog into the error test
	theErrorTest.DupRef = IsDlgButtonChecked(ERC_DUPREF) != 0;
	theErrorTest.UnConnect = IsDlgButtonChecked(ERC_UNCONNECT) != 0;
	theErrorTest.NoConnect = IsDlgButtonChecked(ERC_NOCONNECT) != 0;

	theErrorTest.Power = IsDlgButtonChecked(ERC_POWER) != 0;
	theErrorTest.OutputPwr = IsDlgButtonChecked(ERC_OUTPUTTOPWR) != 0;
	theErrorTest.Output = IsDlgButtonChecked(ERC_OUTPUT) != 0;
	theErrorTest.NoOutput = IsDlgButtonChecked(ERC_NOUTPUT) != 0;
	theErrorTest.UnConnected = IsDlgButtonChecked(ERC_UNCONNECTED) != 0;
	theErrorTest.MultipleNetNames = IsDlgButtonChecked(ERC_MULTIPLENETNAMES) != 0;
	theErrorTest.NonCaseDistinctNetNames = IsDlgButtonChecked(ERC_NONCASEDISTINCT) != 0;
	theErrorTest.UnAssignedRefDes = IsDlgButtonChecked(ERC_UNASSIGNEDREFDES) != 0;

	EndDialog(IDOK);
}

////// the menu entry point for this special function //////


// This table progressively determines the type of the netlist, or, if the type is greater than ERR_BASE, then the error number to be generated for this combination of types.
// The code uses this table as a type of progressive state machine in order to allow objects to be analyzed in any order.  Each successive evaluation
// of current net type and current object type determines the next net type of this table.  Certain net types lock in and force the net type to stay that type.
// For example, if a number of input pins are first examined, the net type will progress from unknown to nInput to nInput to nInput.  Then when an output pin
// is examined, the net type will convert to nOutput and stay that way even if additional nInput pins are found.  If a second nOutput pin is found, however, the
// net type will become ERR_OUTPUT, a preprocessor macro that will be used to select an error message from the resource table.  Once a net type becomes one
// of the message id types, then the state machine locks into this value and essentially stops evaluating any additional connections that might be present.
//
// djl TODO:  Differentiate between no-connect pins and no-connect markers - they really are different!
// A no-connect pin causes errors only if connected to any other pin or net object.
// A no-connect marker should tell the ERC checker to ignore any unconnected pin errors for this pin or net, but shouldn't change the type of the net.
// Presently, if 2 or more output pins are connected together and at least one of them also has no-connect marker placed on it, then no error messages
// will be generated when they really should.  No-connect markers should only indicate that the marked pin is intended to be left floating.
//
const int ErcTable[7 /*theNetType*/][7/*theNodeType*/] = {
// Net:down, Node:across	> Unknown obj	Passive Pin,	Input Pin,		Output Pin,			TriState/BiDir Pin	Power Pin,		NoConnect Pin or Marker
/* Unknown Net*/			{nUnknown,		nPassive,		nInput,			nOutput,			nBiDir,				nPower,			nNoConnect },
/* Passive Net*/			{nPassive,		nPassive,		nPassive,		nOutput,			nBiDir,				nPower,			ERR_NOCONNECT },
/* Input Net*/				{nInput,		nPassive,		nInput,			nOutput,			nBiDir,				nPower,			ERR_NOCONNECT },
/* Output Net*/				{nOutput,		nOutput,		nOutput,		ERR_OUTPUT,			ERR_OUTPUTBIDIR,	ERR_POWERBIDIR,	ERR_NOCONNECT },
/* Tri-State/BiDir Net*/	{nBiDir,		nBiDir,			nBiDir,			ERR_OUTPUTBIDIR,	nBiDir,				ERR_POWERBIDIR,	ERR_NOCONNECT },
/* Power Net*/				{nPower,		nPower,			nPower,			ERR_OUTPUTTOPWR,	ERR_POWERBIDIR,		nPower,			ERR_NOCONNECT },
/* NoConnect Net*/			{nNoConnect,	ERR_NOCONNECT,	ERR_NOCONNECT,	ERR_NOCONNECT,		ERR_NOCONNECT, 		ERR_NOCONNECT,	ERR_NOCONNECT }
};

void CTinyCadView::OnSpecialCheck()
{
	/// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	CDlgERCBox theDialog;
	static union
	{
		ErrorTest e;
		WORD i;
	} theErrorTest;

	theErrorTest.i = (WORD) CTinyCadRegistry::GetInt("ERC", 0xffff);

	/// Get the user's options
	theDialog.SetErrorTest(theErrorTest.e);
	if (theDialog.DoModal() != IDOK)
	{
		return;
	}

	theErrorTest.e = theDialog.GetErrorTest();
	CTinyCadRegistry::Set("ERC", theErrorTest.i);

	DoSpecialCheck();
}

int CTinyCadView::DoSpecialCheck(bool alwaysShowList)
{
	typedef std::map<CString, int> stringCollection;
	CString formattedBuffer;

	/// Get rid of any drawing tool
	GetCurrentDocument()->SelectObject(new CDrawEditItem(GetCurrentDocument()));

	static union
	{
		ErrorTest e;
		WORD i;
	} theErrorTest;

	theErrorTest.i = (WORD) CTinyCadRegistry::GetInt("ERC", 0xffff);

	/// Set the Busy icon
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	/// Generate the netlist
	CNetList netlist;
	netlist.m_follow_imports = false;
	CTinyCadMultiDoc *pDoc = static_cast<CTinyCadMultiDoc*> (GetDocument());
	netlist.MakeNet(pDoc);
	netCollection *nets = &netlist.m_nets;

	/// Delete all the errors which are currently in the design
	//	theERCListBox.Close();

	std::vector<CString> errorList;
	int CurrentError = 0;

	/// Scan the design for unassigned references
	if ( (theErrorTest.e).UnAssignedRefDes)
	{
//		TRACE("Scanning for unassigned reference designators...\n");
		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++)
		{
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd())
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3)
				{
					CString ref = static_cast<CDrawMethod *> (pointer)->GetField(CDrawMethod::Ref);
//					TRACE("  ==>Examining string \"%S\"\n",ref);
					if (ref.Find(_T('?'), 0) != -1)
					{
						// We have an unassigned reference designator
						CString buffer;
						buffer.LoadString(ERR_UNASSIGNEDREFDES);
						formattedBuffer.Format(_T("%s:  [refdes=%s, page=\"%s\", XY=(%g,%g)]\n"), buffer, ref, pointer->m_pDesign->GetSheetName(), pointer->m_point_a.x / 5, pointer->m_point_a.y / 5);
						pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i), static_cast<CDrawMethod *> (pointer)->GetFieldPos(CDrawMethod::Ref), CurrentError++));
						errorList.push_back(formattedBuffer);
//						TRACE("  ==>%S\n",buffer);
					}
				}
				++it;
			}
		}
	}

	/// Scan the design for non-case distinct net names
	/// Scan the design for multiple net names on the same net
	if ( (theErrorTest.e).MultipleNetNames || (theErrorTest.e).NonCaseDistinctNetNames)
	{
		typedef std::map<CString, nodeVector::iterator> nodeVectorCollection;
		std::map<CString, CNetListNode> allNetNames; //stores each net name encountered along with a copy of its node vector contents used later for error messages
		std::map<CString, CString> firstNetName; //stores the first net name encountered indexed by its lower case only equivalent - used later if multiple uncased net names found when the first one found is no longer easily available

		netCollection::iterator ni = nets->begin();
		while (ni != nets->end())
		{
			stringCollection netNames; //Every net name assigned to this net will be collected in this temporary collection to help find multiple net names
			nodeVectorCollection netNameNodes;
			CString buffer;
			CString formattedBuffer;

			int net = (*ni).first;
			nodeVector &v = (*ni).second;

			nodeVector::iterator vi = v.begin(); //Traverse the nodes in the netlist
			while (vi != v.end())
			{
				CString lcLabelName;
				CString labelName;
				CNetListNode &node = *vi;

				//Net names are either an explicit label (xLabelEx2) or implied from a power pin (xPower)
				if (node.m_parent && ( (node.m_parent->GetType() == xPower) || (node.m_parent->GetType() == xLabelEx2)))
				{
					labelName = (node.m_parent->GetType() == xPower) ? netlist.get_power_label((CDrawPower *) node.m_parent) : static_cast<CDrawLabel*> (node.m_parent)->GetValue();
					lcLabelName = labelName;
					lcLabelName.MakeLower();

					if (netNames.find(labelName) == netNames.end())
					{
						//This is a new net name label for this node - add it to the associative arrays used to later format error messages when needed.

						//Furthermore, if this net name is unique in this particular net, then it should also be unique in the entire design.
						//Put this net name (in lower case) in the flattened net name list as well as a way to determine if it is
						//already in this design with some other combination of alphabet case.  The original case of the net name is preserved
						//in the firstNetName associative array, the net number is preserved in the netNames associative array, and the node iterator
						//is preserved in the netNameNodes associative array.  This information is only kept for the first occurrence of a net name
						//because it is not known at this time if another object analyzed later will be in conflict with this one and this descriptive 
						//information will then be needed to format the error messages.

						netNames[labelName] = net; //This is a new net name label for this node - add it to the list
						netNameNodes[labelName] = vi; //This particular map stores only a copy of the node vector iterator - useful only for decoding information contained in this particular net while this net is being scanned
						if (firstNetName.find(lcLabelName) == firstNetName.end())
						{
							//firstNetName is indexed by the lc net name, so it must be carefully checked to be sure that it is safe to put it in this list - it may not be the first
							firstNetName[lcLabelName] = labelName; //Retain this mixed case net name for later reference in error messages
						}

						if (allNetNames.find(lcLabelName) == allNetNames.end())
						{
							//This net name in lower case only is not yet in the list so put it in
							allNetNames[lcLabelName] = *vi; //Retain a copy of the netlist node for later use in error messages
						}
						else
						{
							//While the lower case version of the labelName should not have already been in this list, it apparently is,
							//so generate a warning that net names that are not case distinct are present in this design.
//							TRACE("      Warning:  Net name \"%S\" on net #%d is already in this design as \"%S\" on net #%d.  The two nets differ only in character case and identify non-connected nets.\n", labelName, node.m_NetList, firstNetName.find(lcLabelName)->second, allNetNames[lcLabelName].m_NetList);
							buffer.LoadString(ERR_NONDISTINCTNET);
							formattedBuffer.Format(_T("%s:  \"%s\", \"%s\""), buffer, labelName, firstNetName[lcLabelName]);
							pDoc->GetSheet(node.m_sheet - 1)->Add(new CDrawError(pDoc->GetSheet(node.m_sheet - 1), node.m_a, CurrentError++));
							errorList.push_back(formattedBuffer);

							//In addition, add an error marker for the first occurrence of this non-distinct net name.  The first occurrence did not
							//generate an error at the time, but a copy of it's node contents was stored in the allNetNames associative array so it is still possible to retrieve this information.
							CString firstLabelName;
							firstLabelName = firstNetName[lcLabelName]; //restore original label name from when it was first saved
							buffer.LoadString(ERR_NONDISTINCTNET);
							formattedBuffer.Format(_T("%s:  \"%s\", \"%s\""), buffer, firstLabelName, labelName);
							pDoc->GetSheet(allNetNames[lcLabelName].m_sheet - 1)->Add(new CDrawError(pDoc->GetSheet(allNetNames[lcLabelName].m_sheet - 1), allNetNames[lcLabelName].m_a, CurrentError++));
							errorList.push_back(formattedBuffer);

						}
					}
				}

				++vi;
			}

			if (netNames.size() > 1)
			{
				//Note:  A non-case distinct net name will always produce a multiple net name warning, unless multiple net name warnings are turned off
				if ( (theErrorTest.e).MultipleNetNames)
				{ //record the multiple net name warnings
//					TRACE("    Warning:  Net node %d contains %d different net names\n",net, netNames.size());
					buffer.LoadString(ERR_MULTIPLENETNAMES); //This is the base error message string
//					TRACE("    ==>Base msg=%S\n",buffer);

					//Now unpack and format the multiple net names
					nodeVectorCollection::iterator nv_it = netNameNodes.begin();
					CString stuff = (*nv_it).first;
					formattedBuffer.Format(_T("%s:  [\"%s\""), buffer, (*nv_it).first); //Get the base message and the first net name
//					TRACE("       First formatted msg = %S\n", formattedBuffer);

					while (++nv_it != netNameNodes.end())
					{
						buffer.Format(_T(",\"%s\""), (*nv_it).first);
						formattedBuffer += buffer; //concatenate the next net name onto the end
//						TRACE("       Next formatted msg = %S\n", formattedBuffer);
					}
					formattedBuffer += ']';

					//Now unpack and identify each net name label that is a duplicate, using the error message string that contains all of the net names
					for (nv_it = netNameNodes.begin(); nv_it != netNameNodes.end(); nv_it++)
					{
						CNetListNode &node = * ( (*nv_it).second);
						pDoc->GetSheet(node.m_sheet - 1)->Add(new CDrawError(pDoc->GetSheet(node.m_sheet - 1), node.m_a, CurrentError++));
						errorList.push_back(formattedBuffer);
//						TRACE("  ==>%S\n",formattedBuffer);
					}
				}
			}
			++ni;
		}
	}

	/// Scan the design for duplicated references
	if ( (theErrorTest.e).DupRef)
	{
		std::set<CString> refs;
		CString last = "";

		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++)
		{
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd())
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3)
				{
					CString ref = static_cast<CDrawMethod *> (pointer)->GetField(CDrawMethod::Ref);

					if (refs.find(ref) != refs.end())
					{
						// We have a duplicate...
						CString buffer;
						buffer.LoadString(ERR_DUPREF);
						formattedBuffer.Format(_T("%s:  [Object=\"%s\", RefDes=%s, Page #%d]\n"), buffer, pointer->GetName(), ref, i + 1);
						pDoc->GetSheet(i)->Add(new CDrawError(pDoc->GetSheet(i), static_cast<CDrawMethod *> (pointer)->GetFieldPos(CDrawMethod::Ref), CurrentError++));
						errorList.push_back(formattedBuffer);
					}
					else
					{
						refs.insert(ref);
					}
				}

				++it;
			}
		}
	}

	/// Scan the design for missing sub-parts
	if ( (theErrorTest.e).UnConnected)
	{
		typedef struct
		{
			int ppp;
			int parts;
			CDPoint point;
			CTinyCadDoc* pDesign;
			int sheet;
			CString name;
		} partref;

		typedef std::map<CString, partref> refList;
		refList refs;

		for (int i = 0; i < pDoc->GetNumberOfSheets(); i++)
		{
			drawingIterator it = pDoc->GetSheet(i)->GetDrawingBegin();
			while (it != pDoc->GetSheet(i)->GetDrawingEnd())
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3)
				{
					int ppp = static_cast<CDrawMethod *> (pointer)->GetSymbolData()->ppp;
					if (ppp > 1)
					{
						// Count all parts per reference
						CString partReferenceString = static_cast<CDrawMethod *> (pointer)->GetFieldByName("Ref");
						refList::iterator ref = refs.find(partReferenceString);
						if (ref == refs.end())
						{
							partref pr;
							pr.ppp = ppp;
							pr.parts = 1;
							pr.pDesign = pDoc->GetSheet(i);
							pr.sheet = i;
							pr.point = static_cast<CDrawMethod *> (pointer)->GetFieldPos(CDrawMethod::Ref);
							pr.name = static_cast<CDrawMethod *> (pointer)->GetName();

							refs[partReferenceString] = pr;
						}
						else
						{
							ref->second.parts++;
						}
					}
				}

				++it;
			}
		}

		// Check if all part in the package are present.
		for (refList::iterator ref_it = refs.begin(); ref_it != refs.end(); ref_it++)
		{
			if (ref_it->second.ppp != ref_it->second.parts)
			{
				partref& ref = ref_it->second;
				CString buffer = _T("Not all parts in this package are in this design");
				formattedBuffer.Format(_T("%s:  [Object=\"%s\", RefDes=%s, Page #%d]\n"), buffer, ref.name, ref_it->first, ref.sheet + 1);
				pDoc->GetSheet(ref.sheet)->Add(new CDrawError(ref.pDesign, ref.point, CurrentError++));
				errorList.push_back(formattedBuffer);
			}
		}
	}

	/// Scan netlist to determine the type of each object contained on each net.  Determine if the object type and the net type are compatible
//	TRACE("\n\n\nScanning netlist for object type compatibility\n");
	netCollection::iterator nit = nets->begin();
	while (nit != nets->end())
	{
		nodeVector::iterator nv_it = (*nit).second.begin();
		CString netObjectName;
		CString netObjectRefDes;
		CString netObjectSheetName;
		CString netObjectXY;

		int theNetType = nUnknown;
		CString lastPower = "";
		int connections = 0;

		CDPoint pos;
		int sheet = 0;

		while (theNetType < ERR_BASE && nv_it != (*nit).second.end())
		{
			CNetListNode& theNode = *nv_it;
			//CNetListNode& savedNode = theNode;

			CDrawingObject* pObject = theNode.m_parent;

			if (pObject != NULL)
			{
				//Keep a few identifying items around to help format intelligible error messages after the second object is found

				// Determine the interpreted type of this node
				int theNodeType = nUnknown; //nUnknown is a simplified net type, not really a node type
				switch (pObject->GetType())
				{
					case xPower:
						theNodeType = nPower;
						if (lastPower == "")
						{
							lastPower = static_cast<CDrawPower *> (pObject)->GetValue();
						}
						else
						{
							if (lastPower != static_cast<CDrawPower *> (pObject)->GetValue())
							{
								theNetType = ERR_POWER;
								theNodeType = nUnknown;
							}
						}
						// power symbols should not increment the number of connections
						pos = pObject->m_point_a; //This will be used to locate this object on the sheet for the error message
						sheet = theNode.m_sheet;

						netObjectName.Format(_T("Obj=%s"), pObject->GetName());
						netObjectRefDes = "RefDes=N/A";
						netObjectSheetName.Format(_T("Sheet=#%d"), theNode.m_sheet);
						netObjectXY.Format(_T("XY=(%g,%g)"), theNode.m_a.x / 5, theNode.m_a.y / 5);
						break;
					case xNoConnect: //This is a schematic level NoConnect marker, not a NoConnect pin
						theNodeType = nPassive; //A no-connect marker forces the net type to passive only once.  A passive net can have no errors.
						connections++;
						pos = pObject->m_point_a;
						sheet = theNode.m_sheet;
						netObjectName.Format(_T("Obj=%s"), pObject->GetName());
						netObjectRefDes = "RefDes=N/A";
						netObjectSheetName.Format(_T("Sheet=#%d"), theNode.m_sheet);
						netObjectXY.Format(_T("XY=(%g,%g)"), theNode.m_a.x / 5, theNode.m_a.y / 5);
						break;
					case xPin:
					case xPinEx:
					{
						CDrawPin *pPin = static_cast<CDrawPin*> (pObject);
						switch (pPin->GetElec())
						{
							case 0: // Input
								theNodeType = nInput;
								break;
							case 1: // Output
								theNodeType = nOutput;
								break;
							case 2: // Tristate
								theNodeType = nBiDir;
								break;
							case 3: // Open Collector
								theNodeType = nBiDir;
								break;
							case 4: // Passive
								theNodeType = nPassive;
								break;
							case 5: // Input/Output
								theNodeType = nBiDir;
								break;
							case 6: // Not Connected
								theNodeType = nNoConnect;
								break;
						}

						if (pPin->IsPower())
						{
							theNodeType = nPower;
						}

						pos = pPin->GetActivePoint(theNode.m_pMethod);
						sheet = theNode.m_sheet;
						connections++;

						netObjectName.Format(_T("Obj=%s"), pObject->GetName());
						netObjectRefDes.Format(_T("RefDes=%s, Pin Number=%s, Pin Name=\"%s\""), theNode.m_reference, pPin->GetNumber(), pPin->GetPinName());
						netObjectSheetName.Format(_T("Sheet=\"%s\""), theNode.m_pMethod->m_pDesign->GetSheetName());
						netObjectXY.Format(_T("XY=(%g,%g)"),pos.x/5, pos.y/5);
					}
						break;
					default:
						theNodeType = nUnknown; //all other node types will be treated as an onknown net type.  The majority of these are net connection lines.
				}

				if (theNetType < ERR_BASE)
				{ //Once an error index has been assigned to theNetType, no further evaluations are possible
					assert((theNetType >= 0) && (theNetType < 7));
					assert((theNodeType >= 0) && (theNodeType < 7));
//					TRACE("ErcTable[net type = %d][node type = %d] = %d, connections = %d\n", theNetType, theNodeType, ErcTable[theNetType][theNodeType],connections);

					theNetType = ErcTable[theNetType][theNodeType]; //Since new net type is partially a function of the old net type, this constitutes a state machine
				}
				else
				{
//					TRACE("Error index %d has been assigned, skipping further evaluation...\n", theNetType);
					break;
				}
			}

			++nv_it;
		}

		int ErrorNumber = 0;

		if (connections == 1 && theNetType != nNoConnect)
		{
			//If connections is equal to 1, then the type of the net is the same as the type of the pin
			theNetType = ERR_UNCONNECT;
		}

		switch (theNetType)
		//Note that the "theNetType" will either contain the real net type, or it will contain an error message number that is >= ERR_BASE
		{
			case nUnknown:
				if (connections > 0)
				{
					//If after scanning all connected objects when there is more than 1 object and a net type could not be determined, 
					//then issue the ERR_UNCONNECTED message.  This can be caused by stray net lines, but no pins.
					ErrorNumber = ERR_UNCONNECTED;
				}
				break;
			case nInput:
				// A net type of Input can only occur if at least one pin was an input and no other pins of types capable of driving an output are present
				ErrorNumber = ERR_NOUTPUT;
				break;
			default:
				// Most errors AND error free nets will occur here
				ErrorNumber = theNetType;
				break;
		}

		/// Is this error to be reported?  If not, overwrite the error number with -1 so that it will be less than ERR_BASE
		switch (ErrorNumber)
		{
			case ERR_UNCONNECT:
				if (! ( (theErrorTest.e).UnConnect)) ErrorNumber = -1;
				break;
			case ERR_POWER:
				if (! ( (theErrorTest.e).Power)) ErrorNumber = -1;
				break;
			case ERR_NOCONNECT:
				if (! ( (theErrorTest.e).NoConnect)) ErrorNumber = -1;
				break;
			case ERR_NOUTPUT:
				if (! ( (theErrorTest.e).NoOutput)) ErrorNumber = -1;
				break;
			case ERR_DUPREF:
				if (! ( (theErrorTest.e).DupRef)) ErrorNumber = -1;
				break;
			case ERR_OUTPUT:
				if (! ( (theErrorTest.e).Output)) ErrorNumber = -1;
				break;
			case ERR_OUTPUTTOPWR:
				if (! ( (theErrorTest.e).OutputPwr)) ErrorNumber = -1;
				break;
			case ERR_UNCONNECTED:
				if (! ( (theErrorTest.e).UnConnected)) ErrorNumber = -1;
				break;
			case ERR_OUTPUTBIDIR:
				if (! ( (theErrorTest.e).Output)) ErrorNumber = -1;
				break;
			case ERR_POWERBIDIR:
				if (! ( (theErrorTest.e).OutputPwr)) ErrorNumber = -1;
				break;
		}

		if (ErrorNumber >= ERR_BASE)
		{
			CString buffer;
			buffer.LoadString(ErrorNumber);
			formattedBuffer.Format(_T("%s:  [%s, %s, %s, %s]"), buffer, netObjectName, netObjectRefDes, netObjectSheetName, netObjectXY);
			pDoc->GetSheet(sheet - 1)->Add(new CDrawError(pDoc->GetSheet(sheet - 1), pos, CurrentError++));
			errorList.push_back(formattedBuffer);
		}
//		TRACE("\n");	//Next net
		++nit;
	}

	/// Were any errors detected?
	if (CurrentError == 0 && alwaysShowList)
	{
		CString buffer;
		buffer.LoadString(ERR_NOERROR);
		errorList.push_back(buffer);
	}

	if (CurrentError > 0 || alwaysShowList)
	{
		theERCListBox.Open(pDoc, this);
		for (unsigned int i = 0; i < errorList.size(); i++)
		{
			theERCListBox.AddString(errorList[i]);
		}
	}

	/// Set the normal icon
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	/// Re-Draw the window
	Invalidate();

	return CurrentError;
}

