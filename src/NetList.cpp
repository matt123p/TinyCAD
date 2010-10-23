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
#include "registry.h"
#include "diag.h"
#include "LineUtils.h"
#include "TinyCadMultiDoc.h"

#include <iostream>
#include <fstream>
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"


void CImportFile::assignContext(const CollectionMemberReference<CImportFile *> parent, CString myReference)
{
	m_parent = parent;
	CImportFile *pparent = parent.getObjectWithDefault(NULL);
	if (pparent == NULL)
	{
		m_RefContext = myReference;
	}
	else
	{
		m_RefContext = pparent->m_RefContext;
		m_RefContext += "/";
		m_RefContext += myReference;
	}
}

/// Construction/destruction

CNetList::CNetList()
{
	m_CurrentNet = 1;
	m_prefix_references = FALSE;
	m_follow_imports = TRUE;
	m_prefix_import = FALSE;
	m_err_file = NULL;
	m_errors = 0;
}


CNetList::~CNetList()
{
	/// Delete the imports...
	clear();	
}

/**
 * Clear out the imports etc..
 */
void CNetList::clear()
{
	fileCollection::iterator fi = m_imports.begin();
	while (fi != m_imports.end())
	{
		delete *fi;
		++ fi;
	}
	m_imports.clear();
	m_nodes.clear();
	if (m_err_file)
	{
		fclose( m_err_file );
	}
	m_err_file = NULL;
}


/**
 * Write the to error file.
 * 
 * @param str
 */
void CNetList::writeError( const _TCHAR *str, ... )
{
	va_list argptr;

	TCHAR buffer[ 1024 ];
	++ m_errors;

	va_start( argptr, str );
	_vsntprintf_s( buffer, sizeof( buffer), str, argptr ); 
	_ftprintf( m_err_file, _T("%s"), buffer );
}

/**
 * Open the error file.
 * 
 * @param filename
 */
void CNetList::createErrorFile( const TCHAR *filename )
{
	/// Open the filename for the results
	m_err_filename = filename;
	int brk = m_err_filename.ReverseFind('\\');
	if (brk)
	{
		m_err_filename = m_err_filename.Left( brk );
	}
	m_err_filename += _T(".txt");

	errno_t err;

	err = _tfopen_s(&m_err_file, m_err_filename,_T("w"));
	m_errors = 0;
	if (!m_err_file)
	{
		Message(IDS_CANNOTOPEN);
		return;
	}
}

/**
 * Open the error file in a text view.
 * 
 * @param force
 */
void CNetList::reopenErrorFile( bool force )
{
	_ftprintf( m_err_file, _T("\n%d %s found\n"), m_errors,
		m_errors == 1 ? _T("error") : _T("errors") );
	fclose( m_err_file );

	if (force || (m_errors > 0))
	{
		CTinyCadApp::EditTextFile( m_err_filename );
	}
}

/** This method assigns netlist numbers which may or may not get concatenated later with
 * an alpha prefix to form a net name.  
 * 
 * Many users have requested that net number be preserved and never get reused or reassigned once
 * assigned initially.  This method would be involved in fixing this problem.
 * 
 * @return the next available net number
 */
int CNetList::GetNewNet()
{
	return m_CurrentNet++;
}



/**
 * Add a node to the tree (if necessary), return value of the netlist.
 * 
 * @param ins
 * @return
 */ 
int CNetList::Add(CNetListNode &ins)
{
	/// Is this node already in the tree?
	int found = m_nodes[ ins.m_a ];
  
	/// Has this node already been assigned a net-list index?
	if (ins.m_NetList == -1) 
	{
		/// No, and it must have a (visible) point, so we can add without checking for prior connections...
		if (found != 0 && ins.m_a.hasValue()) 
		{
			ins.m_NetList = found;
		}
		else
		{
			// new netlists and hidden power pins always get a new netlist number.
			ins.m_NetList = GetNewNet();
			m_nodes[ins.m_a] = ins.m_NetList;
		}

		m_nets[ins.m_NetList].push_back( ins );

		return ins.m_NetList;
	} 
	else 
	{
		/// If this node was already found, but with a different netlist
		/// number, then first we must join the two netlists together
		if (found != 0 && found != ins.m_NetList) 
		{
			/// The two nets must be joined
			int NewList = ins.m_NetList;
			int OldList = found;

			/// Get the old list
			nodeVector::iterator OldNetList = m_nets[OldList].begin();

			/// Make the old list's net-list index the same as the new list
			while (OldNetList != m_nets[OldList].end()) 
			{
				(*OldNetList).m_NetList = NewList;
				m_nodes[(*OldNetList).m_a] = NewList;
				++ OldNetList;
			}


			/// Now concatenate the old lists onto the new list
			std::copy(m_nets[OldList].begin(), m_nets[OldList].end(), std::back_inserter(m_nets[NewList]));

			/// Delete the old list
			netCollection::iterator it = m_nets.find(OldList);
			m_nets.erase( it );

			ins.m_NetList = NewList;
		}

		/// Add this node to the netlist
		m_nets[ins.m_NetList].push_back( ins );
		m_nodes[ins.m_a] = ins.m_NetList;

		return ins.m_NetList;
	}
}

/** 
 * Used only during debugging the netlister.  This method formats
 * the netlist objects and outputs them to the debugger output.
 */
void CNetList::dumpNetListObjects()
{
	/// Scan each netlist
	TRACE("\nEntering CNetList::DumpNetListObjects():\n");
    netCollection::iterator ni = m_nets.begin();
	while (ni != m_nets.end())
	{
		int net = (*ni).first;
		TRACE("  ==>Dumping objects for net=%d\n", net);

		nodeVector &v = (*ni).second;

		/// Update the nodes in the netlist
		nodeVector::iterator vi = v.begin();
		while (vi != v.end())
		{
			enum ObjType objectType;
			CString objectNameString;


			CNetListNode &node = *vi;

			if (node.m_parent) {
				objectType = node.m_parent->GetType();
				switch(objectType) {
				case xAnnotation: objectNameString = _T("xAnnotation");
					break;
				case xSymbol: objectNameString = _T("xSymbol");
					break;
				case xSymbolEx: objectNameString = _T("xSymbolEx");
					break;
				case xSymbolEx2: objectNameString = _T("xSymbolEx2");
					break;
				case xBusName: objectNameString = _T("xBusName");
					break;
				case xNoConnect: objectNameString = _T("xNoConnect");
					break;
				case xJunction: objectNameString = _T("xJunction");
					break;
				case xLabel: objectNameString = _T("xLabel");
					break;
				case xWire: objectNameString = _T("xWire");
					break;
				case xBus: objectNameString = _T("xBus");
					break;
				case xPower: objectNameString = _T("xPower");
					break;
				case xPin: objectNameString = _T("xPin");
					break;
				case xLabelEx: objectNameString = _T("xLabelEx");
					break;
				case xLabelEx2: objectNameString = _T("xLabelEx2");
					break;
				case xPinEx: objectNameString = _T("xPinEx");
					break;
				case xLine: objectNameString = _T("xLine");
					break;
				case xHierarchicalSymbol: objectNameString = _T("xHierarchicalSymbol");
					break;
				default: objectNameString = _T("Unknown");
					break;
				}
			}
			if (node.m_parent && node.m_parent->GetType() == xWire) {
				TRACE("    ==>Object:  xWire node\n");
			}
			else if (node.m_parent && node.m_parent->GetType() == xLabelEx2){
				TRACE("    ==>Object:  xLabelEx2=\"%S\" on net=%d\n", static_cast<CDrawLabel*>(node.m_parent)->GetValue(), node.m_NetList);
			}
			else if (node.m_parent && node.m_parent->GetType() == xPower) {
				CString powerLabel = get_power_label((CDrawPower *) node.m_parent);
				TRACE("    ==>Object:  xPower=\"%S\" on m_net=%d\n",powerLabel,node.m_NetList);
			}
			else {
				TRACE("    ==>Object:  %S (obj type=%d) on m_net=%d, net label=\"%S\", refdes=\"%S\", pin number=\"%S\"\n",
					objectNameString, node.m_parent->GetType(),node.m_NetList, node.getLabel(), node.m_reference, node.m_pin);
			}

			++ vi;
		}

		++ ni;
	}
	TRACE("Exiting CNetList::DumpNetListObjects()\n\n");
}

/** 
 * Tell all of the wires what network they are associated with.
 */
void CNetList::WriteWires()
{
	/// Scan each netlist
	//TRACE("\nEntering CNetList::WriteWires():  Updating the wires in this design\n");
    netCollection::iterator ni = m_nets.begin();
	while (ni != m_nets.end()) 
	{
		int net = (*ni).first;
//		TRACE("  ==>Updating wires for net=%d\n",net);

		nodeVector &v = (*ni).second;

		/// Update the nodes in the netlist
		nodeVector::iterator vi = v.begin();
		while (vi != v.end())
		{
//			enum ObjType objectType;	//Used for debugging only
//			CString objectNameString;	//Used for debugging only


			CNetListNode &node = *vi;
#if 0	//enable if you want to debug the netlist
			if (node.m_parent) {
				objectType = node.m_parent->GetType();
				switch(objectType) {
				case xAnnotation: objectNameString = _T("xAnnotation");
					break;
				case xSymbol: objectNameString = _T("xSymbol");
					break;
				case xSymbolEx: objectNameString = _T("xSymbolEx");
					break;
				case xSymbolEx2: objectNameString = _T("xSymbolEx2");
					break;
				case xBusName: objectNameString = _T("xBusName");
					break;
				case xNoConnect: objectNameString = _T("xNoConnect");
					break;
				case xJunction: objectNameString = _T("xJunction");
					break;
				case xLabel: objectNameString = _T("xLabel");
					break;
				case xWire: objectNameString = _T("xWire");
					break;
				case xBus: objectNameString = _T("xBus");
					break;
				case xPower: objectNameString = _T("xPower");
					break;
				case xPin: objectNameString = _T("xPin");
					break;
				case xLabelEx: objectNameString = _T("xLabelEx");
					break;
				case xLabelEx2: objectNameString = _T("xLabelEx2");
					break;
				case xPinEx: objectNameString = _T("xPinEx");
					break;
				case xLine: objectNameString = _T("xLine");
					break;
				case xHierarchicalSymbol: objectNameString = _T("xHierarchicalSymbol");
					break;
				default: objectNameString = _T("Unknown");
					break;
				}
			}
#endif
			if (node.m_parent && node.m_parent->GetType() == xWire) {

				//This is the only statement in this function that actually does work
				static_cast<CDrawLine*>(node.m_parent)->setNetwork( net );

//				TRACE("    ==>Updating xWire node\n");
			}
#if 0	//enable if you want to debug the netlist
			else if (node.m_parent && node.m_parent->GetType() == xLabelEx2){
				TRACE("    ==>Skipping xLabelEx2=\"%S\" on net=%d\n", static_cast<CDrawLabel*>(node.m_parent)->GetValue(), node.m_NetList);
			}
			else if (node.m_parent && node.m_parent->GetType() == xPower) {
				CString powerLabel = get_power_label((CDrawPower *) node.m_parent);
				TRACE("    ==>Skipping xPower=\"%S\" on m_net=%d\n",powerLabel,node.m_NetList);
			}
			else {
				TRACE("    ==>Skipping %S node (type=%d) on m_net=%d, refdes=\"%S\", pin number=\"%S\"\n",
					objectNameString, node.m_parent->GetType(),node.m_NetList, node.m_reference, node.m_pin);
			}
#endif
			++ vi;
		}

		++ ni;
	}
	//TRACE("Exiting CNetList::WriteWires()\n\n");
}

/**
 * Link together several netlists.
 * This is done by linking together:
 * nets that have nodes that have the same filename and same label name
 * 
 * @param nets
 */
void CNetList::Link( linkCollection& nets )
{
	//TRACE("CNetList::Link():  Entering the net node linker.  nets.size()=%d\n", nets.size());
	/// Get rid of any old data
	m_CurrentNet = 1;
	m_nodes.erase( m_nodes.begin(), m_nodes.end() );
	m_nets.erase( m_nets.begin(), m_nets.end() );

	/// Here is a list of known nodes and their netlist number
	typedef std::map<CString,int> stringCollection;
	stringCollection	labels;

	/// Here is a list of known linking information
	linkMap	map;

	/// The linkmap will contain for each netlist a map
	/// of old net number to new net number...
	map.resize( nets.size() );	//size the map to be 1 map entry for each net in the specified netlist

	/// Now scan the nodes to generate the superset of
	/// labels and their netlists

	//TRACE("\n\n\n  ==>Pass 1:  Scanning all netlist nodes to generate the superset of labels and their linking information.\n");
	linkCollection::iterator i = nets.begin();
	while ( i != nets.end() )	//this loop traverses a collection of collections of nets organized by which sheet they originated on
	{
		CNetList &n = *i;
		// Dump the current net
//		TRACE("\n  ==>Pass 1:  Traversing linkCollection:  m_CurrentNet = %d, n.m_CurrentNet = %d, follow imports = %S\n",
//				m_CurrentNet,
//				n.m_CurrentNet,
//				(n.m_follow_imports) ? _T("True"):_T("False"));
		/// Scan each netlist
		netCollection::iterator ni = n.m_nets.begin();
		while (ni != n.m_nets.end())	//this loop traverses a collection of nets originally found on a single sheet
		{
			nodeVector &v = (*ni).second;
			//int old_netlist = (*ni).first;
			int new_netlist = 0;

//			TRACE("\n    ==>Pass 1:  Traversing node in netlist:  node old_netlist = %d, node new_netlist = %d\n", old_netlist, new_netlist);
			/// Scan each node
			nodeVector::iterator vi = v.begin();
			while (vi != v.end())	//this loop traverses a collection of schematic objects associated with a single net originally from a single sheet
			{
				CNetListNode &node = *vi;
//				TRACE("      ==>Pass 1:  Traversing node vector:  Netlist %d Node name=\"%S\" from sheet %d, refdes=\"%S\", pin=\"%S\"\n",
//					old_netlist, node.getLabel(), node.m_sheet, node.m_reference, node.m_pin);

				/// Look for nodes with labels
				if (!node.getLabel().IsEmpty())
				{
					CString label_name = node.getLabel();

					/// Do we already have an entry for this label?
					stringCollection::iterator s = labels.find( label_name );
					//TRACE("        ==>Associated label=\"%S\"\n", label_name);
					if (s != labels.end())
					{
						/// Get netlist of label
						int q = (*s).second;
//						TRACE("        ==>Found an entry for label \"%S\" in labels table associated with netlist %d\n",
//							label_name,
//							(*s).second);

						/// Have we already assigned a netlist to this node?
						if (new_netlist == 0)
						{
							/// Yes... so this node must become that netlist
							new_netlist = q;
						}

						// Update netlist of label
						if (new_netlist != q)
						{
							// Update all labels of old netlist with new netlist
							stringCollection::iterator li = labels.begin();
							while (li != labels.end())
							{							
								if ( (*li).second == q ) 
								{
									labels[ (*li).first ] = new_netlist;
								}
								li++;
							}
						}
					}
					else
					{
						/// No, so we must insert a new label node..
						if (new_netlist == 0)
						{
							new_netlist = m_CurrentNet ++;
						}
						
						labels[ label_name ] = new_netlist;
					}

				}

				++ vi;
			}

			++ ni;
		}

		++ i;
	}


	/// Now scan the nets and generate a
	/// mapping to the super net
	int index = 0;
	i = nets.begin();
	while ( i != nets.end() )
	{
		CNetList &n = *i;

		/// Scan each netlist
		netCollection::iterator ni = n.m_nets.begin();
		while (ni != n.m_nets.end())
		{
			nodeVector &v = (*ni).second;
			int old_netlist = (*ni).first;
			int new_netlist = 0;

			/// Scan each node
			nodeVector::iterator vi = v.begin();
			while (vi != v.end())
			{
				CNetListNode &node = *vi;

				if (!node.getLabel().IsEmpty())
				{
					/// We only link at label level...
					CString label_name = node.getLabel();

					/// Get netlist of label (sould always be found)
					stringCollection::iterator s = labels.find( label_name );
					if (s != labels.end())
					{
						int q = (*s).second;

						/// Have we already assigned a netlist to this node?
						if (new_netlist == 0)
						{
							/// Yes... so this node must become that netlist
							new_netlist = q;
						}
					}
				}

				++ vi;
			}

			/// Did we find a new netlist number for this node?
			if (new_netlist == 0)
			{
				new_netlist = m_CurrentNet ++;
//				TRACE("    ==>Added new_netlist=%d for this node.\n", new_netlist);
			}

			/// Now write it into the map
			map[ index ][ old_netlist ] = new_netlist;
//			TRACE("    ==>Mapping new_netlist=%d onto map[index=%d][old_netlist=%d]\n", new_netlist, index, old_netlist);

			++ ni;
		}

		++ index;	//prepare to process the collection of nets from the next schematic sheet
		++ i;	//prepare to process the collection of nets from the next schematic sheet
	}

//	Uncomment for debugging the netlist
//	TRACE("\n\n\nDump of netlist objects after pass 1 (nets.size() = %d):\n", nets.size());
//	CNetList::dumpNetListObjects();
//	TRACE("Dump complete.\n\n\n");


	/// Now build a super net of all of the
	/// netlists linked together...
	index = 0;
	i = nets.begin();
	int sheetIndex = 0;
	//TRACE("\n\n\n  ==>Pass 2:  Now building a super net of all of the netlists linked together from each schematic page (nets.size()=%d\n",nets.size());
	while ( i != nets.end() )	//this loop traverses a collection of collections of nets organized by which sheet they originated on
	{
		++sheetIndex;	//used for debugging only
		CNetList &n = *i;
//		TRACE("\n  ==>Pass 2:  Traversing linkCollection, sheetIndex = %d, nets.size() = %d, node count = %d:  Current net = %d, follow imports = %S\n",
//				sheetIndex,
//				nets.size(),
//				n.m_nets.size(),
//				n.m_CurrentNet,
//				(n.m_follow_imports) ? _T("True"):_T("False"));

//Uncomment for debugging the netlist
//		TRACE("\n\n\nDump of netlist objects in the middle of pass 2 (sheetIndex = %d, nets.size() = %d, node count = %d) just prior to processing current net=%d:\n",
//				sheetIndex,
//				nets.size(),
//				n.m_nets.size(),
//				n.m_CurrentNet);
//		CNetList::dumpNetListObjects();
//		TRACE("Dump complete.\n\n\n");


		/// Scan each netlist in the collection of nets from a specific schematic sheet
		netCollection::iterator ni = n.m_nets.begin();
		while (ni != n.m_nets.end())	//this loop traverses a collection of nets originally found on a single sheet.  Note that these nets may actually be net fragments at this point.
		{
			nodeVector &v = (*ni).second;	//get the list of schematic objects for this net
			int old_netlist = (*ni).first;	//get the net number for this net
			int new_netlist = map[ index ][ old_netlist ];
//			TRACE("\n    ==>Pass 2 (sheetIndex = %d):  Traversing node in netlist:  node old_netlist = %d, node new_netlist = %d, m_CurrentNet = %d\n", sheetIndex, old_netlist, new_netlist, n.m_CurrentNet);

			/// Update the nodes in the netlist
			nodeVector::iterator vi = v.begin();
//			TRACE("      ==>Pass 2 (sheetIndex = %d):  Updating the nodes in new_netlist=%d\n", sheetIndex, new_netlist);
			while (vi != v.end())
			{
//				CNetListNode &node = *vi;
//				TRACE("      ==>Pass 2:  Traversing node vector:  old_netlist=%d, new_netlist=%d, Node name=\"%S\" from sheet %d, refdes=\"%S\", pin=\"%S\"\n",
//					old_netlist, new_netlist, node.getLabel(), node.m_sheet, node.m_reference, node.m_pin);
				vi->m_NetList = new_netlist;
				++ vi;
			}

			/// Now concatenate the old lists onto the new list
//			TRACE("    ==>Pass 2 (sheetIndex=%d, nets.size()=%d, node count=%d):  Now concatenating all of the nodes in this netlist together.  n.m_CurrentNet = %d\n",
//					sheetIndex,
//					nets.size(),
//					n.m_nets.size(),
//					n.m_CurrentNet);

			std::copy(v.begin(), v.end(), std::back_inserter(m_nets[new_netlist]));

//Uncomment to debug the netlist
//			TRACE("\n\n\nDump of netlist objects in the middle of pass 2 (sheetIndex=%d, nets.size()=%d, node count=%d) just after the concatenation of all of the nodes. n.m_CurrentNet=%d:\n",
//					sheetIndex,
//					nets.size(),
//					n.m_nets.size(),
//					n.m_CurrentNet);
//			CNetList::dumpNetListObjects();
//			TRACE("Dump complete.\n\n\n");

			++ ni;
		}

		++ index;
		++ i;
	}
	//TRACE("  ==>Pass 2:  Finished building the super netlist.  nets.size() = %d\n\n\n", nets.size());

//Uncomment to debug the netlist
//	TRACE("\n\n\nDump of netlist objects after pass 2:\n");
//	CNetList::dumpNetListObjects();
//	TRACE("Dump complete.\n\n\n");
}

/**
 * Perform the work of making a netlist from this design... 
 * 
 * @param pDesign
 */
void CNetList::MakeNet( CTinyCadMultiDoc *pDesign )
{
	linkCollection nets;
	Counter file_counter;

	//CTinyCadMultiDoc*	aDesign = pDesign;
	CollectionMemberReference<CImportFile *> impref;

	CImportFile *f = new CImportFile(pDesign);
	f->setFileNameIndex(file_counter.next());
	m_imports.push_back( f );

	// For each member of m_imports, we generate a netlist for all of its sheets.
	// This includes the root design. Recursion happens here because
	// new members may be added to m_imports as a result of the
	// MakeNetForSheet call. So we can't cache size() or any of its
	// members.
//	TRACE("\n\nEntering CNetList::MakeNet():  Making nets for every sheet in this design\n");
	for ( unsigned int ip = 0; ip < m_imports.size(); ++ ip )
	{
		/// Now create the nets for it
		int base = nets.size();
		int newSheets = m_imports[ip]->getDesign()->GetNumberOfSheets();
		nets.resize( base +  newSheets);
//		TRACE("Started with %d sheets, then added %d new sheets resulting in %d sheets total\n",
//			base,
//			newSheets,
//			nets.size());

		/// Generate a netlist for every sheet in this imported design
		for (int i = 0; i < newSheets; i++)
		{
			nets[i + base].m_prefix_references = m_prefix_references;
			nets[i + base].m_prefix_import = m_prefix_import;
			nets[i + base].MakeNetForSheet( m_imports, ip, i, file_counter);
		}

		// don't repeat this process if we're not following imports
		if (!m_follow_imports)
			break;
	}

	/// Now link the nets together
//	TRACE("\nLinking nets from all sheets together\n\n");
	Link( nets );

	/// ... and write the results into the design for ease of use...
	WriteWires();
//	TRACE("Leaving CNetList::MakeNet():  Finished making nets for every sheet in this design\n");
}


/**
 * Connections between nets (the nodes) are at:
 * .. the end of wires
 * .. the point of pins
 * .. the point of power
 * .. the point of labels
 * .. the point of no-connects
 *
 * To generate a netlist:
 * 1. Search design for nodes.
 * - Insert nodes into node tree and either assign net list to node or generate a new net list
 * - If wire joins two different netlists, then join the two netlists together
 * 2. Find Junctions
 * - Find which wires junctions lie on and connect their netlists
 * 3. Find Labels
 * - Find which wire each label lies on and connect their netlists
 * Perform the work of making a netlist from this design...
 */

/** 
 * Perform the work of making a netlist from a single sheet in this design...
 * 
 * @param imports
 * @param import_index -- the current item within the imports vector
 * @param sheetZeroIndex -- the zero-based index of the sheet within the design file
 * @param file_counter -- a counter used to set the file index of any new design files
 */
void CNetList::MakeNetForSheet (fileCollection &imports, int import_index, int sheetZeroIndexed, Counter& file_counter)
//( fileCollection &imports, int file_index_id, int &file_name_index, int sheet, CTinyCadDoc *pDesign )
//  m_imports[ip]->m_file_name_index, file_name_index, i+1, m_imports[ip]->m_pDesign->GetSheet( i ) );
{
  int sheetOneIndexed = sheetZeroIndexed+1;
//  TRACE("\n\nEntering CNetList::MakeNetForSheet() for page %d\n",sheetOneIndexed);

  // lookup these values from the imports list
  int file_index_id = imports[import_index]->getFileNameIndex();
  CTinyCadDoc *pDesign = imports[import_index]->getDesign()->GetSheet(sheetZeroIndexed);
  // sheet is 1-indexed
  CollectionMemberReference<CImportFile *> referenceToMe(imports, import_index);

  /// Get rid of any old data
  m_CurrentNet = 1;
  m_nodes.erase( m_nodes.begin(), m_nodes.end() );
  m_nets.erase( m_nets.begin(), m_nets.end() );

  /// Here is some temporary data for this function
  typedef std::map<CString,int> stringCollection;
  stringCollection	Powers;
  stringCollection	Connected;

  //Prefetch iterator begin and end to speed things up
  drawingIterator itBegin = pDesign->GetDrawingBegin();
  drawingIterator itEnd = pDesign->GetDrawingEnd();

  /// Search for nodes, and build the node tree
  drawingIterator it = itBegin;
  for (;it != itEnd; ++ it ) 
  {
	CDrawingObject *ObjPtr = *it;
	stringCollection::iterator found;
	int hold;
	CDPoint tr;

	switch (ObjPtr->GetType()) 
	{
		case xHierarchicalSymbol:
			{
				CDrawHierarchicalSymbol *pSymbol = static_cast<CDrawHierarchicalSymbol*>(ObjPtr);
//				TRACE("Found xHierarchicalSymbol with name=[%S], value=[%S], ref=[%S]\n\n",pSymbol->GetName(),pSymbol->GetRef());
				/// Try and stop recursion by limiting the number of imports
				if (imports.size() > 100)
				{
					AfxMessageBox( IDS_RECURSION );
					continue;
				}

				/// Push back this filename into the list of extra imports

				CString partReferenceString = pSymbol->GetFieldByName("Ref");
				CImportFile *f = new CImportFile(referenceToMe, partReferenceString);
				f->setFileNameIndex(file_counter.next());
				if (f->Load( pSymbol->GetFilename() ) )
				{
					imports.push_back( f );

					/// Now search the symbol for pins to link the other symbols to
					drawingCollection method;
					pSymbol->ExtractSymbol(tr,method);

					drawingIterator it = method.begin();
					drawingIterator itEnd = method.end();
					while ( it != itEnd ) 
					{
						CDrawingObject *pointer = *it;

						if (pointer->GetType()==xPinEx) 
						{
							CDrawPin *thePin = static_cast<CDrawPin*>(pointer);

							/// This in effect labels the node with the new node name...
							CNetListNode n( f->getFileNameIndex(), sheetOneIndexed, thePin, thePin->GetActivePoint(pSymbol) );
							n.setLabel( thePin->GetPinName() );
							n.m_reference = 
								// pSymbol->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheetOneIndexed);
								get_reference_path(pSymbol, imports[file_index_id], false);
							n.m_pin = thePin->GetNumber();
							n.m_pMethod = pSymbol;
							Add(n);
						}

						++ it;
					}
				}
				else
				{
					delete f;
				}
			}
			break;
		case xMethodEx3:
			#define thePin ((CDrawPin*)pointer)
			#define theMethod ((CDrawMethod*)ObjPtr)
//			TRACE("Found xMethodEx3 (Symbol) at (%g,%g).  Symbol name=[%S], Symbol ref=[%S].\n",
//				theMethod->m_point_a.x, theMethod->m_point_a.y,
//				theMethod->GetName(), theMethod->GetRef());
			{
				drawingCollection method;
				((CDrawMethod *)ObjPtr)->ExtractSymbol(tr,method);

				CString myRefDes = get_reference_path(theMethod, imports[file_index_id], false);

				drawingIterator it = method.begin();
				drawingIterator itEnd = method.end();
				while ( it != itEnd ) 
				{
					CDrawingObject *pointer = *it;

					if (pointer->GetType()==xPinEx && !(thePin->IsPower()) ) 
					{
						//Only process pins that are NOT power pins here.  Power pins are handled later.
//						TRACE("  ==>Symbol name=[%S], ref=[%S]:  This pin is NOT a power pin.  PinName=[%S], Name=[%S], Pin number=[%S].\n",
//							theMethod->GetName(), theMethod->GetRef(),
//							thePin->GetPinName(), thePin->GetName(), thePin->GetNumber());

						CNetListNode n( file_index_id, sheetOneIndexed, thePin,thePin->GetActivePoint(theMethod));
						n.m_reference = myRefDes;

						n.m_pin = thePin->GetNumber();
						n.m_pMethod = theMethod;
						Add(n);
					}

					++ it;
				}

				/// Has this symbol had it's power connected?
				if (Connected.find(myRefDes) == Connected.end()) 
				{
//					TRACE("  ==>Seeing if reference designator [%S] has any pins of type=power defined\n",myRefDes);
					Connected[ myRefDes ] = TRUE;

					drawingCollection method;
					((CDrawMethod *)ObjPtr)->ExtractSymbol(tr,method);
					drawingIterator it = method.begin();
					drawingIterator itEnd = method.end();
					while (it!=itEnd) 
					{
						CDrawingObject *pointer = *it;

						if (pointer->GetType()==xPinEx && thePin->IsPower()) 
						{
							// Hidden power pins will get an uninitialized CDPoint node point.
							// The Add method will never connect to any other uninitialized CDPoint node point
							// and thus hidden power pins will correctly never connect to anything by their node coordinate.
							CNetListNode n(file_index_id, sheetOneIndexed, thePin,thePin->GetActivePoint(theMethod) );
							// Set netlist label name to invisible symbol power pin name
//							TRACE("  ==>Found a power pin in this symbol.  Setting netlist %d's label to power pin name=[\"%S\"]\n",
//								n.m_NetList,
//								thePin->GetPinName());
							n.setLabel( thePin->GetPinName() );
							n.m_reference = 
								// theMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheetOneIndexed);
								get_reference_path(theMethod, imports[file_index_id], false);

							n.m_pin = thePin->GetNumber();
							n.m_pMethod = theMethod;


							/// Look up the netlist this power pin belongs to
							found = Powers.find( thePin->GetPinName() );
							if (found != Powers.end())
								n.m_NetList = (*found).second;
							hold = Add(n);
							if (found == Powers.end())
								Powers[thePin->GetPinName()] = hold;
						}

						++ it;
					}
//					TRACE("  ==>Finished checking this symbol for power pins\n");
				}
			}			
			break;
		case xNoConnect:
			Add(CNetListNode(file_index_id, sheetOneIndexed, ObjPtr,ObjPtr->m_point_a));
//			TRACE("Found xNoConnect at (%g,%g).\n",ObjPtr->m_point_a.x, ObjPtr->m_point_a.y);
			break;
		case xJunction:
			Add(CNetListNode(file_index_id, sheetOneIndexed, ObjPtr,ObjPtr->m_point_a));
//			TRACE("Found xJunction at (%g,%g).\n",ObjPtr->m_point_a.x, ObjPtr->m_point_a.y);
			break;			
		case xPower:
			{
			CNetListNode n(file_index_id, sheetOneIndexed, ObjPtr,ObjPtr->m_point_a);

			CString powerLabel = get_power_label((CDrawPower *)ObjPtr);
//			TRACE("Found xPower=[%S] at (%g,%g)\n",powerLabel,ObjPtr->m_point_a.x, ObjPtr->m_point_a.y);
			n.setLabel( powerLabel );

			/// Does this power item exist?
			found = Powers.find( powerLabel );
			if (found != Powers.end())
				n.m_NetList = (*found).second;
			hold = Add(n);
			if (found == Powers.end())
				Powers[ powerLabel ] = hold;
			}
			break;
		case xWire:
//			TRACE("Found xWire at (%g,%g) (%g,%g)\n",
//				ObjPtr->m_point_a.x, ObjPtr->m_point_a.y,
//				ObjPtr->m_point_b.x, ObjPtr->m_point_b.y);

			{
				CNetListNode n(file_index_id, sheetOneIndexed, ObjPtr,ObjPtr->m_point_a);
				hold = Add(n);
			}
			{
				CNetListNode n(file_index_id, sheetOneIndexed, ObjPtr,ObjPtr->m_point_b);
				n.m_NetList = hold;
				Add(n);
			}
			break;
	}
  }

  /// Search for junctions and connect together
//  TRACE("Searching for junctions to connect together\n");
  it = itBegin;
  while (it != itEnd) 
  {
	CDrawingObject *ObjPtr = *it;

	/// Search for junctions
	if (ObjPtr->GetType() == xJunction) 
	{
		/// Find out which netlist was assigned to this point
		CDPoint a = ObjPtr->m_point_a;
		int NetNumber = m_nodes[ a ];
//		TRACE("  ==>Found a junction at (%g,%g).  Will now look for wires that cross this junction\n",a.x, a.y);

		/// Look for wires which cross this junction
		drawingIterator search_it = itBegin;
		while (search_it != itEnd) 
		{
			CDrawingObject *search = *search_it;

			/// Find the wires
			/// If the wire has an end at this junction then it is already connected
			if ((search->GetType()==xWire) 
			  &&(search->m_point_a!=a)
			  &&(search->m_point_b!=a))
			{
				/// Is this point on this wire?
				CLineUtils l( search->m_point_a, search->m_point_b );
				double distance_along_a;

				if (l.IsPointOnLine( a, distance_along_a ))
				{
					CNetListNode n(file_index_id, sheetOneIndexed, NULL,search->m_point_a);
					n.m_NetList = NetNumber;
					NetNumber = Add(n);
//					TRACE("  ==>Found line segment with coordinates (%g,%g : %g,%g) that intersects junction at (%g,%g).  Adding this node to net number=[%d]\n",
//						search->m_point_a.x, search->m_point_a.y,
//						search->m_point_b.x, search->m_point_b.y,
//						a.x, a.y,
//						NetNumber);
				}
			}

			++ search_it;
		}
	}
	
	++ it;
  }

  /// Search for labels and connect to their respective lines
  stringCollection labels;
//  TRACE("Searching for labels to match with wires\n");
  it = itBegin;
  while (it != itEnd) 
  {
	CDrawingObject *ObjPtr = *it;

	/// Search for labels
	if (ObjPtr->GetType() == xLabelEx2) 
	{
		CDPoint a = static_cast<CDrawLabel*>(ObjPtr)->GetLabelPoint();
//		TRACE("Found label.  Value=[%S], Name=[%S], ObjType=[%d/0x%x] (enum value).  Looking for wires that connect to it\n",
//			static_cast<CDrawLabel*>(ObjPtr)->GetValue(),
//			static_cast<CDrawLabel*>(ObjPtr)->GetName(),
//			(int) (static_cast<CDrawLabel*>(ObjPtr)->GetType()),
//			(int) (static_cast<CDrawLabel*>(ObjPtr)->GetType()));

		/// Search for a wire this label is connect to
		/// Only attempt to connect to a single wire
		drawingIterator search_it = itBegin;
		while (search_it != itEnd) 
		{
			CDrawingObject *search = *search_it;
			if (search->GetType()==xWire && search->IsInside(a.x,a.x,a.y,a.y)) 
			{
				a = search->m_point_a;	//Overwrite label point "a" with first point on newly found wire
//				TRACE("  ==>Found a wire with coordinates (%g,%g : %g,%g) that connects to label=[%S].  Stopping search after first find!\n",
//					search->m_point_a.x, search->m_point_a.y,
//					search->m_point_b.x, search->m_point_b.y,
//					static_cast<CDrawLabel*>(ObjPtr)->GetValue());
				break;
			}
			
			++ search_it;
		}

		/// Look up this label
		CNetListNode n(file_index_id, sheetOneIndexed, ObjPtr,a);	//Find the net list node that this wire is a member of
//		TRACE("Assigning label [%S] to netlist name=[%S], number=%d\n",((CDrawLabel *)ObjPtr)->GetValue(),n.getLabel(),n.m_NetList);

		n.setLabel(  ((CDrawLabel *)ObjPtr)->GetValue() );

		/// Has this label already been assigned a netlist?
		stringCollection::iterator found = labels.find(((CDrawLabel *)ObjPtr)->GetValue());
		if (found!=labels.end()) 
		{
//			TRACE("Label [%S] has already been assigned to netlist %d via net name=[%S], net number=[%d].\n",
//				((CDrawLabel *)ObjPtr)->GetValue(),
//				(*found).second,
//				n.getLabel(),
//				n.m_NetList);
			n.m_NetList = (*found).second;
			n.setLabel( _T("") );
		}

		int hold = Add(n);

		/// If there was no netlist write it back...
		if (found == labels.end()) {

			assert(labels[ ((CDrawLabel *)ObjPtr)->GetValue()] == 0);

			labels[ ((CDrawLabel *)ObjPtr)->GetValue()] = hold;
//			TRACE("Label=[%S] has not yet had a netlist assigned to it.  Holding netlist=%d in labels[%S] until later.  After assignment, netlist=%d\n",
//				((CDrawLabel *)ObjPtr)->GetValue(),
//				hold,
//				((CDrawLabel *)ObjPtr)->GetValue(),
//				labels[ ((CDrawLabel *)ObjPtr)->GetValue()]);
		}
	}
	
	++ it;
  }  

  /// Our work with the nodes map is complete, so we can discard it now...
  m_nodes.erase( m_nodes.begin(), m_nodes.end() );

//  TRACE("Leaving CNetList::MakeNetForSheet() for page %d\n\n",sheetOneIndexed);
}


/**
 * Create netlist and output as a SPICE file.
 * 
 * @param type
 * @param pDesign
 * @param filename
 */
void CNetList::WriteNetListFile( int type, CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
	switch (type)
	{
	case 1:
		WriteNetListFilePADS( pDesign, filename );
		break;
	case 2:
		WriteNetListFileEagle( pDesign, filename );
		break;
	case 3:
		WriteNetListFileProtel( pDesign, filename );
		break;
	case 4:
		WriteNetListFilePCB( pDesign, filename, true );
		break;
	case 5:
		WriteNetListFilePCB( pDesign, filename, false );
		break;
	case 6:
		WriteNetListFileXML( pDesign, filename );
		break;
	default:
		WriteNetListFileTinyCAD( pDesign, filename );
		break;
	}
}

/**
 * Create netlist and output as a Protel PCB script.
 * 
 * @param pDesign
 * @param filename
 */
void CNetList::WriteNetListFileProtel( CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
  FILE *theFile;
  errno_t err;
  err = _tfopen_s(&theFile, filename,_T("w"));
  if ((theFile == NULL) || (err != 0))
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  /// Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	/// Get the net list
	MakeNet( pDesign );

	/// Keep track of the references that we have output...
	std::set<CString>	referenced;

	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

  		/// Generate a component for every sheet in this design
		for (int i = 0; i < dsn->GetNumberOfSheets(); i++)
		{
			drawingIterator it = dsn->GetSheet(i)->GetDrawingBegin();
			while (it != dsn->GetSheet(i)->GetDrawingEnd()) 
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3) 
				{
					CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);
					CString Ref  = 
						// pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->getFileNameIndex(),i+1);
						get_reference_path(pMethod, *fi, false);

					/// Do we need to output this part?
					if (referenced.find( Ref ) == referenced.end())
					{
						referenced.insert( Ref );

						CString Package = "This part has no 'Package' attribute";
						CString Name = pMethod->GetName();

						for (int i = 2; i < pMethod->GetFieldCount(); i++)
						{
							if (pMethod->GetFieldName(i).CompareNoCase(_T("package")) == 0)
							{
								Package = pMethod->GetField(i);
							}
						}

						_ftprintf(theFile,_T("[\n%s\n%s\n%s\n\n\n\n]\n"), 
							Ref, Package, Name);
					}
				}

				++ it;
			}
		}
	}

  int Label = 0;

  netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		CString theLine,theLabel;

		if (nv_it != (*nit).second.end()) 
		{
			theLine = "";
			BOOL first = TRUE, Labeled = FALSE;
			int len = 0;
			int count = 0;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (!theNode.getLabel().IsEmpty() && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}

				if (!theNode.m_reference.IsEmpty())
				{
					CString add;
					add.Format(_T("%s-%s"), theNode.m_reference, theNode.m_pin );
					len += add.GetLength();
					if (len > 127)
					{
						theLine += _T("\n");
						len = add.GetLength();
						first = TRUE;
					}

					if (first)
					{
						first = FALSE;
					}
					else
					{
						theLine += _T("\n");
						len ++;
					}


					theLine += add;
					++ count;
						
				}
			}

			if (count > 1) 
			{
				_ftprintf(theFile,_T("(\n"));
				if (Labeled)
					_ftprintf(theFile,_T("%s"), theLabel);
				else
					_ftprintf(theFile,_T("N%06d"),Label++);
				_ftprintf(theFile,_T("\n%s\n)\n"), theLine);
			}
		}

		++ nit;
	}


  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
  fclose(theFile);
}


/**
 * Create netlist and output as a PCB file (PADS-PCB).
 * 
 * @param pDesign
 * @param filename
 */
void CNetList::WriteNetListFilePADS( CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
  FILE *theFile;
  errno_t err;
  err = _tfopen_s(&theFile, filename,_T("w"));
  if ((theFile == NULL) || (err != 0))
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  /// Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

  /// Get the net list
  MakeNet( pDesign );

  _ftprintf(theFile,_T("*PADS-PCB*\n"));


  _ftprintf(theFile,_T("*PART*\n"));
  

  /// Keep track of the references that we have output...
  std::set<CString>	referenced;

	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

		/// Generate a component for every sheet in this design
		for (int i = 0; i < dsn->GetNumberOfSheets(); i++)
		{

			drawingIterator it = dsn->GetSheet(i)->GetDrawingBegin();
			while (it != dsn->GetSheet(i)->GetDrawingEnd()) 
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3) 
				{
					CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);
					CString Ref = get_reference_path(pMethod, *fi, false);
						// pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->getFileNameIndex(),i+1);

					/// Do we need to output this part?
					if (referenced.find( Ref ) == referenced.end())
					{
						referenced.insert( Ref );

						CString Package = _T("This part has no 'Package' attribute");
						for (int i = 2; i < pMethod->GetFieldCount(); i++)
						{
							if (pMethod->GetFieldName(i).CompareNoCase(_T("package")) == 0)
							{
								Package = pMethod->GetField(i);
							}
						}


						/// Pad to correct length...
						do
						{
							Ref = Ref + _T(" ");
						}
						while (Ref.GetLength() < 8);
						_ftprintf(theFile,_T("%s%s\n"), Ref, Package);
					}
				}
				
				++ it;
			}
		}
	}

  _ftprintf(theFile,_T("\n*NET*\n"));
  int Label = 0;

  netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		CString theLine,theLabel;

		if (nv_it != (*nit).second.end()) 
		{
			theLine = _T("");
			BOOL first = TRUE, Labeled = FALSE;
			int len = 0;
			int count = 0;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (!theNode.getLabel().IsEmpty() && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}

				if (!theNode.m_reference.IsEmpty())
				{
					CString add;
					add.Format(_T("%s.%s"), theNode.m_reference, theNode.m_pin );
					len += add.GetLength();
					if (len > 127)
					{
						theLine += _T("\n");
						len = add.GetLength();
						first = TRUE;
					}

					if (first)
					{
						first = FALSE;
					}
					else
					{
						theLine += " ";
						len ++;
					}


					theLine += add;
					++ count;
					
				}
			}

			if (count > 1) 
			{
				_ftprintf(theFile,_T("*SIGNAL*  "));
				if (Labeled)
					_ftprintf(theFile,_T("%s"), theLabel);
				else
					_ftprintf(theFile,_T("N%06d"),Label++);
				_ftprintf(theFile,_T("\n%s\n"), theLine);
			}
		}

		++ nit;
	}

	_ftprintf(theFile,_T("*END*\n"));


  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
  fclose(theFile);
}


/**
 * Create netlist and output as a PCB file
 * 
 * @param pDesign
 * @param filename
 */
void CNetList::WriteNetListFileTinyCAD( CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
  FILE *theFile;
  errno_t err;
  err = _tfopen_s(&theFile, filename,_T("w"));
  if ((theFile == NULL) || (err != 0))
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  // Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

  // Get the net list
  MakeNet( pDesign );

  _ftprintf(theFile,NetComment _T(" ====+  Net List for %s  +====\n\n"), pDesign->GetPathName() );


  _ftprintf(theFile,NetComment _T(" ======+ The component list\n\n"));

  // Keep track of the references that we have output...
  std::set<CString>	referenced;

  	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

		/// Generate a component for every sheet in this design
		for (int i = 0; i < dsn->GetNumberOfSheets(); i++)
		{
			drawingIterator it = dsn->GetSheet(i)->GetDrawingBegin();
			while (it != dsn->GetSheet(i)->GetDrawingEnd()) 
			{
			CDrawingObject *pointer = *it;

			if (pointer->GetType() == xMethodEx3) 
			{
				CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);
				CString Name = pMethod->GetField(CDrawMethod::Name);
				CString Ref  = 
					get_reference_path(pMethod, (*fi), false);
					//pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->getFileNameIndex(),i+1);

				/// Do we need to output this part?
				if (referenced.find( Ref ) == referenced.end())
				{
					referenced.insert( Ref );

					_ftprintf(theFile,_T("COMPONENT '%s' = %s\n"),Ref,Name);

					/// Now write it it's "other" references
					for (int i = 2; i < pMethod->GetFieldCount(); i++)
					{
						_ftprintf(theFile,_T("\tOPTION '%s' = %s\n"),pMethod->GetFieldName(i), pMethod->GetField(i) );
					}
				}
			}
			
			++ it;
			}
		}
	}

  _ftprintf(theFile,_T("\n\n") NetComment _T(" ======+ The net list\n\n"));
  int Label = 0;

  netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		CString theLine,theLabel;

		if (nv_it != (*nit).second.end()) 
		{
			theLine = "";
			BOOL first = TRUE,PrintLine=FALSE, Labeled = FALSE;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (!theNode.getLabel().IsEmpty() && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}

				if (!theNode.m_reference.IsEmpty())
				{
					CString add;
					add.Format(_T("(%s,%s)"), theNode.m_reference, theNode.m_pin );
					if (theNode.getLabel() != add)
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							theLine += _T(",");
						}

						theLine += add;
						PrintLine=TRUE;
					}
				}
			}
			if (PrintLine) {
				_ftprintf(theFile,_T("NET  "));
				if (Labeled)
				{
					_ftprintf(theFile,_T("'%s'"),theLabel);
				}
				else
				{
					_ftprintf(theFile,_T("'N%06d'"),Label++);
				}
				_ftprintf(theFile,_T(" =  %s\n"),theLine);
			}
		}

		++ nit;
	}


  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
  fclose(theFile);
}

/**
 * Create netlist and output as a Eagle PCB script.
 * 
 * @param pDesign
 * @param filename
 */
void CNetList::WriteNetListFileEagle( CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
  FILE *theFile;
  errno_t err;
  err = _tfopen_s(&theFile, filename,_T("w"));
  if ((theFile == NULL) || (err != 0))
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  /// Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

  /// Get the net list
  MakeNet( pDesign );

  int y_pos = 1;
  int x_pos = 1;

  /// Keep track of the references that we have output...
  std::set<CString>	referenced;

  	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

		/// Generate a component for every sheet in this design
		for (int i = 0; i < dsn->GetNumberOfSheets(); i++)
		{
			drawingIterator it = dsn->GetSheet(i)->GetDrawingBegin();
			while (it != dsn->GetSheet(i)->GetDrawingEnd()) 
			{
			CDrawingObject *pointer = *it;

			if (pointer->GetType() == xMethodEx3) 
			{
				CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);
				CString Ref  = get_reference_path(pMethod, *fi, false);
					//pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->getFileNameIndex(),i+1);

				/// Do we need to output this part?
				if (referenced.find( Ref ) == referenced.end())
				{
					referenced.insert( Ref );

					CString Package = _T("This part has no 'Package' attribute");
					for (int i = 2; i < pMethod->GetFieldCount(); i++)
					{
						if (pMethod->GetFieldName(i).CompareNoCase(_T("package")) == 0)
						{
							Package = pMethod->GetField(i);
						}
					}


					_ftprintf(theFile,_T("ADD '%s' %s R0 (0.%d 0.%d);\n"), Ref, Package, x_pos, y_pos );

					++ x_pos;
					if (x_pos == 10)
					{
						++ y_pos;
						x_pos = 1;
					}
				}

			}
			
			++ it;
			}
		}
	}

  _ftprintf(theFile,_T("\n\n"));

  netCollection::iterator nit = m_nets.begin();
  int Label = 0;

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		CString theLine,theLabel;

		if (nv_it != (*nit).second.end()) 
		{
			theLine = "";
			//BOOL first = TRUE;
			BOOL PrintLine=FALSE;
			BOOL Labeled = FALSE;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (!theNode.getLabel().IsEmpty() && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}

				if (!theNode.m_reference.IsEmpty())
				{
					CString add;
					add.Format(_T("   %s %s\n"), theNode.m_reference, theNode.m_pin );
					theLine += add;
					PrintLine=TRUE;
				}
			}
			if (PrintLine) {
				_ftprintf(theFile,_T("SIGNAL "));
				if (Labeled)
					_ftprintf(theFile,_T("%s"), theLabel);
				else
					_ftprintf(theFile,_T("N%06d"),Label++);
				_ftprintf(theFile,_T("\n%s   ;\n"), theLine);
			}
		}

		++ nit;
	}




  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
  fclose(theFile);
}

/**
 * Create netlist and output as an XML file
 * 
 * @param pDesign
 * @param filename
 */
namespace RXML
{
	typedef rapidxml::xml_node<TCHAR> node;
	typedef rapidxml::xml_attribute<TCHAR> attribute;
	typedef rapidxml::xml_document<TCHAR> document;
}
void CNetList::WriteNetListFileXML( CTinyCadMultiDoc *pDesign, const TCHAR *filename)
{
	std::basic_ofstream<TCHAR> outfile;
	outfile.open (filename);
	/// !!! jms -- TODO -- needs to handle file open error

    /// Set the Busy icon
    SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	rawWriteNetListFileXML(pDesign, outfile);

    SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
	outfile.close();
}

void CNetList::rawWriteNetListFileXML( CTinyCadMultiDoc *pDesign, std::basic_ofstream<TCHAR>& outfile)
{
	RXML::document doc;

	// xml declaration
	RXML::node *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute(_T("version"), _T("1.0")));
	decl->append_attribute(doc.allocate_attribute(_T("encoding"), _T("utf-8")));
	doc.append_node(decl);

	RXML::node *root = doc.allocate_node(rapidxml::node_element, _T("netlist"));
	doc.append_node(root);
	RXML::node *x_components = doc.allocate_node(rapidxml::node_element, _T("parts"));
	RXML::node *x_nets = doc.allocate_node(rapidxml::node_element, _T("nets"));
	root->append_node(x_components);
	root->append_node(x_nets);

	// Get the net list
	MakeNet( pDesign );

	RXML::attribute *attr = doc.allocate_attribute(_T("design"), doc.allocate_string(pDesign->GetPathName()));
	root->append_attribute(attr);

	// Keep track of the references that we have output...
	std::set<CString>	referenced;

  	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

		/// Generate a component for every sheet in this design
		for (int i = 0; i < dsn->GetNumberOfSheets(); i++)
		{
			drawingIterator it = dsn->GetSheet(i)->GetDrawingBegin();
			while (it != dsn->GetSheet(i)->GetDrawingEnd()) 
			{
			CDrawingObject *pointer = *it;

			if (pointer->GetType() == xMethodEx3 || pointer->GetType() == xHierarchicalSymbol) 
			{
				CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);
				CString Name = pMethod->GetField(CDrawMethod::Name);
				CString Ref  = 
					get_reference_path(pMethod, (*fi), false);
					//pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->getFileNameIndex(),i+1);
				
				/*
				TinyCAD does not have a reference back to the library for which this symbol came from?!?!?
				CDesignFileSymbol *pSymbol = pMethod->GetSymbolData();				
				*/

				// Do we need to output this part?
/*				if (referenced.find( Ref ) == referenced.end())
				{
					referenced.insert( Ref );
					*/
					RXML::node *x_component;

					if (pointer->GetType() == xHierarchicalSymbol)
					{
						CDrawHierarchicalSymbol *pSymbol = static_cast<CDrawHierarchicalSymbol*>(pointer);
				
						CString Name = pSymbol->GetFilename();
						x_component = doc.allocate_node(rapidxml::node_element, _T("hierarchical-part"));
						x_component->append_attribute(
							doc.allocate_attribute(_T("filename"), doc.allocate_string(Name)));
					}
					else
					{
						x_component = doc.allocate_node(rapidxml::node_element, _T("part"));
					}
					x_components->append_node(x_component);

					x_component->append_attribute(doc.allocate_attribute(_T("ref"), doc.allocate_string(Ref)));
					x_component->append_attribute(doc.allocate_attribute(_T("name"), doc.allocate_string(Name)));

					/// Now write attributes other than Ref and Name (which are always 1st and 2nd)
					int n = pMethod->GetFieldCount();
					for (int i = 2; i < n; i++)
					{
						RXML::node *x_component_attribute = 
							doc.allocate_node(rapidxml::node_element, _T("attribute"), doc.allocate_string(pMethod->GetField(i)));
						x_component_attribute->append_attribute(doc.allocate_attribute(_T("name"), 
							doc.allocate_string(pMethod->GetFieldName(i))));
						x_component->append_node(x_component_attribute);
					}
				// }
			}
			
			++ it;
			}
		}
	}

  int Label = 0;

  netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();


		RXML::node *x_net = doc.allocate_node(rapidxml::node_element, _T("net"));
		x_nets->append_node(x_net);

		TCHAR *netname = NULL;

		while (nv_it != (*nit).second.end()) 
		{
			CNetListNode& theNode = *nv_it;
			++ nv_it;

			if (netname == NULL && !theNode.getLabel().IsEmpty())
			{
				netname = doc.allocate_string(theNode.getLabel());
			}

			if (theNode.m_parent != NULL && theNode.m_parent->GetType() == xPinEx)
			{
				CDrawPin *pPin = static_cast<CDrawPin *>(theNode.m_parent);

				bool isHierarchicalPin = (theNode.m_pMethod != NULL && theNode.m_pMethod->GetType() == xHierarchicalSymbol);
				RXML::node *x_pin = doc.allocate_node(rapidxml::node_element, 
					isHierarchicalPin ? _T("hierarchical-pin") : _T("pin"));
				x_net->append_node(x_pin);
				if (!theNode.m_reference.IsEmpty())
				{
					x_pin->append_attribute(
						doc.allocate_attribute(_T("part"), doc.allocate_string(theNode.m_reference)));
				}
				x_pin->append_attribute(
					doc.allocate_attribute(_T("name"), doc.allocate_string(pPin->GetPinName())));
				x_pin->append_attribute(
					doc.allocate_attribute(_T("number"), doc.allocate_string(pPin->GetNumber())));
				const TCHAR *etype = CDrawPin::GetElectricalTypeName(pPin->GetElec());
				if (etype != NULL)
				{
					x_pin->append_attribute(doc.allocate_attribute(_T("type"), etype));
					// don't need to allocate_string since these types are fixed in memory
				}
			}
		}

		if (netname != NULL)
		{
			x_net->append_attribute(doc.allocate_attribute(_T("name"), netname));
		}

		TCHAR buf[12];
		_stprintf_s(buf,12,_T("%d"), Label++);
		x_net->append_attribute(doc.allocate_attribute(_T("number"), 
				doc.allocate_string(buf)));

		++ nit;
	}

	// 3. output the XML document.
	outfile << doc;
}


/**
 * Create netlist and output as a PCB file
 * 
 * @param pDesign
 * @param filename
 * @param unixOutputFile - a flag indicating whether this file should be written using wide byte characters (for Unix systems) or wide characters (for Windows/DOS systems)
 */
void CNetList::WriteNetListFilePCB( CTinyCadMultiDoc *pDesign, const TCHAR *filename, bool unixOutputFile)
{
  #define TMP_SIZE 5000
  char tmp [TMP_SIZE];
  size_t convertedCount;
  FILE *theFile;
  errno_t err;

  if (unixOutputFile)
  {
    wcstombs_s(&convertedCount, tmp, TMP_SIZE, filename, _tcslen(filename));	//convert Unicode string to single byte character string
	err = fopen_s(&theFile, tmp, "wb");	//open in binary mode with ASCII character set
  }
  else
  {
	err = _tfopen_s(&theFile, filename, _T("w"));	//open in translated text mode in Unicode format
  }

  if ((theFile == NULL) || (err != 0))
  {
	Message(IDS_CANNOTOPEN);
	return;
  }

  // Set the Busy icon
  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

  // Get the net list
  MakeNet( pDesign );

  // Keep track of the references that we have output...
  std::set<CString>	referenced;
  int count = 0, Label = 0;
  netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		CString theLine,theLabel;

		if (nv_it != (*nit).second.end()) 
		{
			theLine = _T("");
			BOOL first = TRUE,PrintLine=FALSE, Labeled = FALSE;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (!theNode.getLabel().IsEmpty() && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}

				if (!theNode.m_reference.IsEmpty())
				{
					CString add;
					add.Format(_T("%s-%s "), theNode.m_reference, theNode.m_pin );
					if (theNode.getLabel() != add)
					{
						if (first)
						{
							first = FALSE;
						}
						else
						{
							// theLine += _T(",");
						}
						theLine += add;
						if ( ++ count > 40 )
						{
							count = 0;
							add.Format(_T(" \\\n " ));
							theLine += add;
						}							
						PrintLine=TRUE;
					}
				}
			}
			
			if (PrintLine) {
				count = 0;
				CString aLabel;

				if (Labeled)
				{
					aLabel.Format(_T("%s "),theLabel);
				}
				else
				{
					aLabel.Format(_T("N%06d "), Label++);
				}

				if (unixOutputFile)
				{	//Unix output files are encoded as single byte character strings
					//with no translation of cr/lf characters.  Line endings will be a 
					//single lf character.

					wcstombs_s(&convertedCount, tmp, TMP_SIZE, aLabel, aLabel.GetLength());	//convert Unicode string to single byte character string
					fprintf(theFile," %s ", tmp);	//print the net name label as single byte characters
					
					wcstombs_s(&convertedCount, tmp, TMP_SIZE, theLine, theLine.GetLength());	//convert Unicode string to single byte character string
					fprintf(theFile," %s\n", tmp);	//print the list of net connections as single byte characters
				}
				else
				{	//Output Unicode file for Windows/DOS
					_ftprintf(theFile,_T(" %s  %s\n"), aLabel, theLine);	//print the net label and net connections as Unicode characters
				}
			}
		}

		++ nit;
	}


  SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
  fclose(theFile);
}


/**
 * Create netlist and output as a SPICE file.
 * 
 * @param pDesign
 * @param filename
 */
void CNetList::WriteSpiceFile( CTinyCadMultiDoc *pDesign, const TCHAR *filename )
{
	/// Open the filename for the spice file
	FILE *theFile;
	errno_t err;
	err = _tfopen_s(&theFile, filename,_T("w"));
	if ((theFile == NULL) || (err != 0))
	{
		Message(IDS_CANNOTOPEN);
		return;
	}

	/// Output the standard header comment - expected on line 1 by some Spice engines
	_ftprintf(theFile,_T("* Schematics Netlist *\n"));

	createErrorFile( filename );

	_ftprintf(m_err_file,_T("Results of Spice file generation for %s\n\n"),
		pDesign->GetPathName() );


	/// Set the Busy icon
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	/// Create the net list
	m_prefix_import = TRUE;
	m_prefix_references = TRUE;
	MakeNet( pDesign );

    /**
     * Now we have the net list we must convert it into a file suitable for spice...
     * 
     * The netlist represents a single vector per net, however, spice requires
     * a vector per symbol.  We have to rotate the array, so that we have one
     * entry in our vector for each of the symbols in our drawing....
     */

	typedef std::map<CString,CNetListSymbol>    symbolCollection;
    symbolCollection symbols;
    labelCollection labels;

	netCollection::iterator nit = m_nets.begin();

	while (nit != m_nets.end()) 
	{
		nodeVector::iterator nv_it = (*nit).second.begin();

		while (nv_it != (*nit).second.end()) 
		{
			CNetListNode& theNode = *nv_it;
			++ nv_it;

			/// Is this node a symbol?
			if (!theNode.m_reference.IsEmpty() && theNode.m_pMethod->GetType() == xMethodEx3 )
			{
				/// Yes, so update the pin allocations in the symbol map...
				CNetListSymbol &symbol = symbols[ theNode.m_reference ];

				// store pin name to pin number mapping as well for use with advanced Spice netlists
				if (theNode.m_parent->GetType() == xPinEx)
				{
					CDrawPin *pPin = static_cast<CDrawPin *>(theNode.m_parent);
					symbol.m_pin_name_map [ pPin->GetPinName() ] = pPin->GetNumber();
				}

				symbol.m_pins[ theNode.m_pin ] = theNode.m_NetList;
				symbol.m_pMethod = theNode.m_pMethod;
				symbol.setFileNameIndex(theNode.getFileNameIndex());
				// this was missing in prior versions!
			}

			/// Is this node a label?
			if (!theNode.getLabel().IsEmpty())
			{
				/// Yes, so update the label collection
				labels[ theNode.m_NetList ] = theNode.getLabel();
			}
		}

		++ nit;
	}


	/**
     * Now we scan the symbols array and extract any file imports
     * That we need from the fields of the symbols... 
     */

	symbolCollection::iterator sit = symbols.begin();

	typedef std::set<CString> strings;
	typedef std::vector<strings> string_collection;
	string_collection prolog_lines;
	string_collection epilog_lines;
	prolog_lines.resize( 10 );
	epilog_lines.resize( 10 );

	/// Do this for all of the files in the imports list...
	fileCollection::iterator fi = m_imports.begin();
	for (;fi != m_imports.end(); ++ fi)
	{
		CTinyCadMultiDoc *dsn = pDesign;

		if ((*fi)->getFileNameIndex() != 0)
		{
			dsn = static_cast<CTinyCadMultiDoc *>((*fi)->getDesign());
		}

		/// Generate a component for every sheet in this design
		for (int sheet = 0; sheet < dsn->GetNumberOfSheets(); sheet++)
		{
			drawingIterator it = dsn->GetSheet(sheet)->GetDrawingBegin();
			while (it != dsn->GetSheet(sheet)->GetDrawingEnd()) 
			{
				CDrawingObject *pointer = *it;

				if (pointer->GetType() == xMethodEx3) 
				{
					CDrawMethod *pMethod = static_cast<CDrawMethod *>(pointer);

					/// Search this symbols fields and extract the SPICE_IMPORT field...

					// !!!! JMS -- why does it get the prolog and epilog but not the actual spice contents?

					CString spice_prolog;
					CString spice_epilog;
					int spice_pro_priority = 5;
					int spice_epi_priority = 5;

					for (int j = 0; j < pMethod->GetFieldCount(); j++)
					{
						CString field = pMethod->GetFieldName(j);
						if (field.CompareNoCase(AttrSpiceProlog) == 0)
						{
							CNetListSymbol symbol( (*fi)->getFileNameIndex(), sheet, pMethod );
							spice_prolog = expand_spice( (*fi)->getFileNameIndex(), sheet, symbol, labels, pMethod->GetField(j) );
						}
						else if (field.CompareNoCase(AttrSpiceEpilog) == 0)
						{
							CNetListSymbol symbol( (*fi)->getFileNameIndex(), sheet, pMethod );
							spice_epilog = expand_spice( (*fi)->getFileNameIndex(), sheet, symbol, labels, pMethod->GetField(j) );
						}
						else if (field.CompareNoCase(AttrSpicePrologPri) == 0)
						{
							spice_pro_priority = _tstoi( pMethod->GetField(j) );
							if (spice_pro_priority < 0 || spice_pro_priority > 9)
							{
								spice_pro_priority = 5;
							}
						}
						else if (field.CompareNoCase(AttrSpiceEpilogPri) == 0)
						{
							spice_epi_priority = _tstoi( pMethod->GetField(j) );
							if (spice_epi_priority < 0 || spice_epi_priority > 9)
							{
								spice_epi_priority = 5;
							}
						}
					}


					/// Prologue...
					strings &prolog = prolog_lines[ spice_pro_priority ];
					if (prolog.find( spice_prolog ) == prolog.end())
					{
						/// Not included yet...
						prolog.insert( spice_prolog );
					}

					/// Epilog..
					strings &epilog = epilog_lines[ spice_pro_priority ];
					if (epilog.find( spice_epilog ) == epilog.end())
					{
						/// Not included yet...
						epilog.insert( spice_epilog );
					}
				}
			
				++ it;
			}
		}
	}
 
	/// We have extracted the prologue, so now output it in the order of priority (0 being first...)
	int priority;
    for (priority = 0; priority < 10; priority ++)
	{
		strings &s = prolog_lines[ priority ];
		strings::iterator i = s.begin();
		while (i != s.end())
		{
			_ftprintf( theFile, _T("%s\n"), *i );
			++ i;
		}
	}


	/// We now have the netlist in the form we require it, so
	/// let us now output the symbols in the correct SPICE format...

	sit = symbols.begin();
	while (sit != symbols.end())
	{
		CString reference = (*sit).first;
		CNetListSymbol &symbol = (*sit).second;
		CDrawMethod* pMethod = symbol.m_pMethod;
		int sheet = symbol.m_sheet;
		int file_name_index = symbol.getFileNameIndex();

		/// Here is the data we are going to extract from the symbol's fields...
		
		CString spice = pMethod->GetFieldByName(AttrSpice);
		
		/// Now output the SPICE model line
		if (!spice.IsEmpty())
		{
			_ftprintf(theFile,_T("%s\n"), expand_spice( file_name_index, sheet, symbol, labels, spice ) );
		}
		else
		{
			_ftprintf(theFile,_T("NO_MODEL\n") );
			writeError(_T("%s: %s on sheet %d has no model\n"),
				symbol.m_pMethod->GetRef(), symbol.m_pMethod->GetName(), sheet );
		}
		++ sit;
	}


	/// Now write out the epilog
    for (priority = 9; priority >= 0; priority --)
	{
		strings &s = epilog_lines[ priority ];
		strings::iterator i = s.begin();
		while (i != s.end())
		{
			_ftprintf( theFile,_T("%s\n"), *i );
			++ i;
		}
	}



	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
	fclose(theFile);

	reopenErrorFile( true );
}

/**
 * Expand a spice line.
 * 
 * @param file_name_index
 * @param sheet
 * @param symbol
 * @param labels
 * @param spice
 * @return
 */
CString CNetList::expand_spice( int file_name_index, int sheet, CNetListSymbol &symbol, labelCollection &labels, CString spice )
{
	// NOTE: The spice parameter is expanded using parameter substitution
	//       to have some kind of parameters in it e.g. R $(1) $(2) %(NAME) etc....
	//
	//		 Each of the parameters refers to either a pin name or a field name.
	//       We would then replace each of the parameters with the values from the
	//       fields or the netlist number.
	//
	//		 $ means expand an attribute name into its value
	//		 % means expand a pin name or number into its associated net name
	//		 @ means expand into a net
	//		 ? means expand a conditional macro into either its true clause or its false clause (if both are present)
	//		 \\ is an escape sequence - the next character is to be output literally
	//
	//       Also we haven't dealt with power connections here either.  To determine
	//       If a connection is power connection, use the labels maps.
	//

	CString spice_line;
	enum 
	{
		normal,
		awaiting_pin,
		reading_pin,
		awaiting_net,
		reading_net,
		awaiting_attr,
		reading_attr,
		awaiting_macro,
		reading_macro,
		awaiting_macro_escape,
		awaiting_escape
	} mode;
	mode = normal;
	CString lookup;
	std::vector<CString> macro_strings;
	//CDrawMethod* pMethod = symbol.m_pMethod;
	int brackets = 0;

	for (int i = 0; i < spice.GetLength(); i++)
	{
		TCHAR c = spice.GetAt( i );
		switch (mode)
		{
		case normal:
			{
				if (c == '%')
				{
					mode = awaiting_pin;
				}
				else if (c == '@')
				{
					mode = awaiting_net;
				}
				else if (c == '$')
				{
					mode = awaiting_attr;
				}
				else if (c == '?')
				{
					mode = awaiting_macro;
				}
				else if (c == '\\')
				{
					mode = awaiting_escape;
				}
				else if (c == '\r') 
				{

				    /** 
                     * Each line of the spice template text is terminated with a standard \r\n.
                     * Since every \n that is output to the netlist file is automatically expanded
                     * to a \r\n, it is necessary to filter out the extra \r's here, or 
                     * it will result in a \r\r\n sequence in the netlist file.
                     * character c is effectively thrown away - wait for the \n to output a newline
                     */
				}
				else 
				{
					spice_line += c;	//c may be a newline character (\n).  This will automatically be expanded to a \r\n by the C runtime
				}
			}
			break;
		case awaiting_escape:
			spice_line += c;
			mode = normal;
			break;
		case awaiting_macro_escape:
			lookup += c;
			mode = reading_macro;
			break;
		case awaiting_pin:
			if (c != '(')
			{
				spice_line += c;
				mode = normal;
			}
			else
			{
				lookup = "";
				mode = reading_pin;
			}
			break;
		case awaiting_net:
			if (c != '(')
			{
				spice_line += c;
				mode = normal;
			}
			else
			{
				lookup = "";
				mode = reading_net;
			}
			break;
		case awaiting_attr:
			if (c != '(')
			{
				spice_line += c;
				mode = normal;
			}
			else
			{
				lookup = "";
				mode = reading_attr;
			}
			break;
		case awaiting_macro:
			if (c != '(')
			{
				spice_line += c;
				mode = normal;
			}
			else
			{
				lookup = "";
				brackets = 1;
				mode = reading_macro;
			}
			break;
		case reading_pin:
		case reading_net:
			if (c != ')')
			{
				lookup += c;
			}
			else
			{

				CString r;
				int nodes;
				int net;
				if (get_pin_by_number_or_name( symbol, labels, lookup, nodes, r, net ))
				{
					if (mode == reading_pin)
					{
                        spice_line += r;
					}
					else
					{   
						CString s;
						s.Format(_T("%d"), net );
						spice_line += s;
					}
				}
				else
				{
					spice_line += _T("<pin '") + lookup + _T("' not found>");
					writeError(_T("%s: %s on sheet %d pin '%s' not found\n"),
						symbol.m_pMethod->GetRef(), symbol.m_pMethod->GetName(), sheet, lookup );
				}
				mode = normal;
			}
			break;
		case reading_attr:
			if (c != ')')
			{
				lookup += c;
			}
			else
			{
				/// Insert the correct name from the attribute list
				CString r;
				if (get_attr( file_name_index, sheet, symbol, lookup,r ))
				{
					spice_line += r;
				}
				else
				{
					spice_line += _T("<attr '") + lookup + _T("' not found>");
					writeError(_T("%s: %s on sheet %d attribute '%s' not found\n"),
						symbol.m_pMethod->GetRef(), symbol.m_pMethod->GetName(), sheet, lookup );
				}
				mode = normal;
			}
			break;
		case reading_macro:
			if (c == '\\')
			{
				if (brackets == 1)
				{
					mode = awaiting_macro_escape;
				}
				else
				{
					lookup += '\\';
				}
			}
			else if (c == ',')
			{
				if (brackets == 1)
				{
					macro_strings.push_back( lookup );
					lookup = "";
				}
				else
				{
					lookup += c;
				}
			}
			else if (c == '(')
			{
				brackets ++;
				lookup += c;
			}
			else if (c != ')')
			{
				lookup += c;
			}
			else
			{
				brackets --;
				if (brackets == 0)
				{
					/// We have our macro strings, so check what we need to do...
					macro_strings.push_back( lookup );

					/// Now evaluate the query
					bool r = eval_spice_macro( file_name_index, sheet, symbol, labels, spice_line, macro_strings[0] );

					/// ... and insert the appropriate text
					CString insert;
					if (r && macro_strings.size() > 1)
					{
						insert = macro_strings[1];
					}
					if (!r && macro_strings.size() > 2)
					{
						insert = macro_strings[2];
					}

					/// Empty the macro strings array
					macro_strings.clear();
					lookup.Empty();
					mode = normal;
					
					/// We need to recursively evaluate the macro...
					spice_line += expand_spice( file_name_index, sheet, symbol, labels, insert );
				}
				else
				{
					lookup += c;
				}
			}

			break;
		}
	}

	return spice_line;
}

/**
 * Evaluate the Spice macro expression.
 * 
 * @param file_name_index
 * @param sheet
 * @param symbol
 * @param labels
 * @param spice_line
 * @param macro
 * @return
 */
bool CNetList::eval_spice_macro(int file_name_index, int sheet, CNetListSymbol &symbol, labelCollection &labels, CString &spice_line, CString macro )
{
	/// What is this query?
	int brk = macro.Find(_T("("));
	if (brk == -1)
	{
		spice_line += _T("<Unknown query operation: ") + macro + _T(">");
		return false;
	}
	CString op = macro.Left( brk );
	CString value = macro.Mid( brk + 1 );
	if (value.GetAt( value.GetLength() -1 ) == ')')
	{
		value = value.Left( value.GetLength() -1 );
	}

	op = op.Trim();
	value = value.Trim();

	if (op == "defined")
	{
		CString r;
		return get_attr( file_name_index, sheet, symbol, value, r );
	}
	else if (op == "not_defined")
	{
		CString r;
		return !get_attr( file_name_index, sheet, symbol, value, r );
	}
	else if (op == "empty")
	{
		CString r;
		get_attr( file_name_index, sheet, symbol, value, r );
		return r.IsEmpty();
	}
	else if (op == "not_empty")
	{
		CString r;
		get_attr( file_name_index, sheet, symbol, value, r );
		return !r.IsEmpty();
	}
	else if (op == "connected")
	{
		/// Determine if this pin exists and is connected..
		CString r;
		int nodes,net;
		get_pin_by_number( symbol, labels, value, nodes, r, net );

		return nodes > 1;
	}
	else if (op == "not_connected")
	{
		/// Determine if this pin exists and is connected..
		CString r;
		int nodes,net;
		get_pin_by_number( symbol, labels, value, nodes, r, net );

		return nodes == 1;
	}
	else
	{
		spice_line += "<Unknown query operation: " + macro + ">";
		writeError(_T("%s: %s on sheet %d unknown query operand '%s'\n"),
			symbol.m_pMethod->GetRef(), symbol.m_pMethod->GetName(), sheet, macro );
	}
	return false;
}

/**
 * Get a netlist name from a pin number.
 * 
 * @param symbol
 * @param labels
 * @param pin
 * @param nodes
 * @param r
 * @param net
 * @return
 */

bool CNetList::get_pin_by_number_or_name( CNetListSymbol &symbol, labelCollection &labels, CString pin, int &nodes, CString &r, int &net )
{
	// Look up the pin number first, then if not found, see if it can be found as a pin name instead.
	// This maintains strict compatibility with a PSpice extended feature that allows macros to reference
	// either the pin number (very common) or the pin name (a little less common)
	if (get_pin_by_number(symbol, labels, pin, nodes, r, net))
	{
		return true;
	}
	
	pinNameToNumberMap::iterator it = symbol.m_pin_name_map.find(pin);
	
	if (it != symbol.m_pin_name_map.end()) // found!
	{
		CString target_pin_number = it->second;
		return get_pin_by_number(symbol, labels, target_pin_number, nodes, r, net);
	}

	return false;
}

bool CNetList::get_pin_by_number( CNetListSymbol &symbol, labelCollection &labels, CString pin, int &nodes, CString &r, int &net )
{
	// don't iterate through the m_pins map! We can just use find.
	nodes = 0;
	pinCollection::iterator pin_it = symbol.m_pins.find(pin);
	if (pin_it != symbol.m_pins.end())
	{
		CString pin_number = pin_it->first;
		CString s;
		net = pin_it->second;
		if (labels.find( net ) != labels.end())
		{
			s = labels[ net ];
		}
		else
		{
			s.Format(_T("_N_%d"), net );
		}
		r = s;

		/// Count the number of connected nodes
		nodeVector &vn = m_nets[ net ];
		nodeVector::iterator i = vn.begin();
		while (i != vn.end())
		{
			if ((*i).m_pMethod)
			{
				++ nodes;
			}
			++ i;
		}
		return true;
	}

	return false;
}

	// Get a hierarchical reference path from a symbol.
	//
	// Note that file_name_index in all of these functions is misleading.
	// It should really be called "context_instance_index" or something.
	// It is a unique incremental identifier representing the context of a circuit or
	// subcircuit. 0 represents the root (main) circuit. Each instance of a hierarchical
	// design gets a new ID. (so if the root circuit has 2 instances of a hierarchical
	// design, and that design has 2 instances of another hierarchical design,
	// there will be 7 IDs total: 1 for the main circuit, 2 for the 2nd level design,
	// and 4 for the 4 instances of the 3rd level design.
	// 
	// As to the format of this function's output:
	// Suppose the symbol in question is U4 inside a hierarchical circuit H1 
	// which in turn is inside another hierarchical circuit H22 in the root circuit.
	// If forward=true and separator="_", this will return "H22_H1_U1".
    // If forward=false and separator=":", this will return "U1:H1:H22".
    // The default separator is an underscore.

CString CNetList::get_reference_path( 
			const CDrawMethod* psymbol, 
			const CImportFile* pcontext,
			bool forward, 
			TCHAR separator)
{
	CString ref = pcontext->getReferenceContext();
	// The canonical reference for the constant index is forward,
	// with a separator of "/" and an additional "/" at the beginning.

	_TCHAR * canonical_separator = _T("/");
	CString s;
	if (ref.GetLength() > 0)
	{
		int curpos = 0;
		CString tok = ref.Tokenize(canonical_separator, curpos);
		while (tok != "")
		{
			if (forward)
			{
				s += tok;
				s += separator;
			}
			else
			{
				s = separator + tok + s;
			}
			tok = ref.Tokenize(canonical_separator, curpos);
		}

		if (forward)
		{
			s += psymbol->GetRef();
		}
		else
		{
			s = psymbol->GetRef() + s;
		}
	}
	else
	{
		s = psymbol->GetRef();
	}
	return s;
}


/**
 * Get a attribute value from an attribute name.
 * 
 * @param file_name_index
 * @param sheet
 * @param symbol
 * @param attr
 * @param r
 * @return
 */
bool CNetList::get_attr( int file_name_index, int sheet, CNetListSymbol &symbol, CString attr, CString &r )
{
	CDrawMethod* pMethod = symbol.m_pMethod;
	if (attr.CompareNoCase( _T("refnum") ) == 0)
	{
		/// Use the reference number (minus the reference character)
		CString s = pMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_name_index,sheet+1);
		int b = s.FindOneOf(_T("0123456789"));
		if (b != 1)
		{
			r = get_reference_path(pMethod, m_imports[file_name_index], false);
			return true;
		}
		else
		{
			r = s.Mid(b);
			return true;
		}
	}
	else if (attr.CompareNoCase( _T("ref") ) == 0)
	{
		CString ref = m_imports[file_name_index]->getReferenceContext();

		CString s;
		if (ref.GetLength() > 0)
		{
			int curpos = 0;
			_TCHAR * separator = _T("/");
			CString tok = ref.Tokenize(separator, curpos);
			while (tok != "")
			{
				s = "_" + tok + s;
				tok = ref.Tokenize(separator, curpos);
			}
			s = pMethod->GetRef() + s;
		}
		else
			s = pMethod->GetRef();

//			->GetRefSheet(m_prefix_references,m_prefix_import,file_name_index,sheet+1);
		r = s;
		return true;
	}
	else
	{
		for (int j = 0; j < pMethod->GetFieldCount(); j++)
		{
			if (pMethod->GetFieldName(j).CompareNoCase(attr) == 0)
			{
				r = pMethod->GetField(j);
				return true;
			}
		}
	}

	return false;
}


// Get a netlist label name for this power symbol
CString CNetList::get_power_label( CDrawPower *power )
{
	// Use the power value as netlist name where it is available
	if (!power->GetValue().IsEmpty())
	{
	   return power->GetValue();
	}

	// Differentiate between the different power symbol shapes
	// and use fixed/default netlist names.
	switch (power->which)
	{
	case 0: // Bar
		return _T("TCPOWERBAR");
	case 1: // Circle
		return _T("TCPOWERCIRCLE");
	case 2: // Wave
		return _T("TCPOWERWAVE");
	case 3: // Arrow
		return _T("TCPOWERARROW");
	case 4: // Earth
		return _T("TCPOWEREARTH");
	}

	// Just in case... (or rather not a case ;-)
	return _T("TCPOWER");
}


CImportFile::~CImportFile()
{
	if (m_parent.isNotNull())
	{
		delete m_pDesign;
	}

	// we don't delete the design if there's no parent
	// (in which case this is a root design)
}

BOOL CImportFile::Load(const TCHAR *filename)
{
	ASSERT( m_pDesign == NULL );

	CFile f;
	CFileException e;
	if( !f.Open( filename, CFile::modeRead, &e ) ) 
	{
		e.ReportError();
		return FALSE;
	}
	CArchive ar( &f, CArchive::load );
	m_pDesign = new CTinyCadMultiDoc;
	m_pDesign->Serialize( ar );

	return TRUE;
}
