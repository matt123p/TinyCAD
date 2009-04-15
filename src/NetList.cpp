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
		/// No, so we can add without checking for prior connections...
		if (found != 0) 
		{
			ins.m_NetList = found;
		}
		else
		{
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
 * Tell all of the wires what network they are associated with.
 */
void CNetList::WriteWires()
{
	/// Scan each netlist
    netCollection::iterator ni = m_nets.begin();
	while (ni != m_nets.end()) 
	{
		int net = (*ni).first;
		nodeVector &v = (*ni).second;

		/// Update the nodes in the netlist
		nodeVector::iterator vi = v.begin();
		while (vi != v.end())
		{
			CNetListNode &node = *vi;
			if (node.m_parent && node.m_parent->GetType() == xWire)
			{
				static_cast<CDrawLine*>(node.m_parent)->setNetwork( net );
			}

			++ vi;
		}

		++ ni;
	}
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
	/// Get rid of any old data
	m_CurrentNet = 1;
	m_nodes.erase( m_nodes.begin(), m_nodes.end() );
	m_nets.erase( m_nets.begin(), m_nets.end() );

	/// Here is a list of known nodes and their netlist number
	typedef std::map<CString,int> stringCollection;
	stringCollection	labels;

	/// Here is a list of known linking information
	typedef std::map<int,int> intCollection;
	typedef std::vector< intCollection > linkMap;
	linkMap	map;

	/// Here is a map of netlists that are linked together
	intCollection	linked_netlists;

	/// The linkmap will contain for each netlist a map
	/// of old net number to new net number...
	map.resize( nets.size() );

	/// Now scan the nodes to generate the superset of
	/// labels and their linking information
	int index = 0;
	linkCollection::iterator i = nets.begin();
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

					/// Do we already have an entry for this label?
					stringCollection::iterator s = labels.find( label_name );
					if (s != labels.end())
					{
						int q = (*s).second;

						while (linked_netlists.find(q) != linked_netlists.end())
						{
							q = linked_netlists[ q ];
						}

						/// Have we already assigned a netlist to this node?
						if (new_netlist != 0)
						{
							int z = new_netlist;
							while (linked_netlists.find(z) != linked_netlists.end())
							{
								int hold = linked_netlists[ z ];
								if (z != q)
								{
									linked_netlists[ z ] = q;
								}
								else
								{
									/// No need to link a list to itself!
									linked_netlists.erase( q );
								}
								z = hold;
							}

							/// We will have to link these two nodes together...
							if (new_netlist != q)
							{
								linked_netlists[ new_netlist ] = q;
							}
						}
						else
						{
							/// Yes... so this node must become that netlist
							new_netlist = q;
						}
					}
					else
					{
						/// No, so we must insert a new node..
						if (new_netlist == 0)
						{
							new_netlist = m_CurrentNet ++;
						}
						
						labels[ label_name ] = new_netlist;
					}

				}

				++ vi;
			}

			/// Did we find a new netlist number for this node?
			if (new_netlist == 0)
			{
				new_netlist = m_CurrentNet ++;
			}

			/// Now write it into the map
			map[ index ][ old_netlist ] = new_netlist;

			++ ni;
		}

		++ index;
		++ i;
	}


	/// Now build a super net of all of the
	/// netlists linked together...
	index = 0;
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
			int new_netlist = map[ index ][ old_netlist ];

			/// Check to see if these netlists are linked together...
			while (linked_netlists.find(new_netlist) != linked_netlists.end())
			{
				new_netlist = linked_netlists[ new_netlist ];
			}

			/// Update the nodes in the netlist
			nodeVector::iterator vi = v.begin();
			while (vi != v.end())
			{
				CNetListNode &node = *vi;
				vi->m_NetList = new_netlist;
				++ vi;
			}

			/// Now concatenate the old lists onto the new list
			std::copy(v.begin(), v.end(), std::back_inserter(m_nets[new_netlist]));

			++ ni;
		}

		++ index;
		++ i;
	}
}

/**
 * Perform the work of making a netlist from this design... 
 * 
 * @param pDesign
 */
void CNetList::MakeNet( CTinyCadMultiDoc *pDesign )
{
	linkCollection nets;
	int file_name_index = 0;

	CImportFile *f = new CImportFile;
	f->m_file_name_index = file_name_index;
	m_imports.push_back( f );

	/// Make the nets for our sheets
	nets.resize( pDesign->GetNumberOfSheets() );

	/// Generate a netlist for every sheet in this design
	for (int i = 0; i < pDesign->GetNumberOfSheets(); i++)
	{
		nets[i].m_prefix_references = m_prefix_references;
		nets[i].m_prefix_import = m_prefix_import;
		nets[i].MakeNetForSheet( m_imports, 0, file_name_index, i+1, pDesign->GetSheet( i ) );
	}

	/// Now include from our imports
	if (m_follow_imports)
	{
		for ( unsigned int ip = 1; ip < m_imports.size(); ++ ip )
		{
			/// Now create the nets for it
			int base = nets.size();
			nets.resize( base + m_imports[ip]->m_pDesign->GetNumberOfSheets() );

			/// Generate a netlist for every sheet in this imported design
			for (int i = 0; i < m_imports[ip]->m_pDesign->GetNumberOfSheets(); i++)
			{
				nets[i + base].m_prefix_references = m_prefix_references;
				nets[i + base].m_prefix_import = m_prefix_import;
				nets[i + base].MakeNetForSheet( m_imports, m_imports[ip]->m_file_name_index, file_name_index, i+1, m_imports[ip]->m_pDesign->GetSheet( i ) );
			}
		}
	}

	/// Now link the nets together
	Link( nets );

	/// ... and write the results into the design for ease of use...
	WriteWires();
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
 * @param file_index_id
 * @param file_name_index
 * @param sheet
 * @param pDesign
 */
void CNetList::MakeNetForSheet( fileCollection &imports, int file_index_id, int &file_name_index, int sheet, CTinyCadDoc *pDesign )
{
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

				/// Try and stop recursion by limiting the number of imports
				if (imports.size() > 100)
				{
					AfxMessageBox( IDS_RECURSION );
					continue;
				}

				/// Push back this filename into the list of extra imports
				CImportFile *f = new CImportFile;
				++ file_name_index;
				f->m_file_name_index = file_name_index;
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
							CNetListNode n( file_name_index, sheet, thePin, thePin->GetActivePoint(pSymbol) );
							n.setLabel( thePin->GetPinName() );
							n.m_reference = pSymbol->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheet);
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
			{
				drawingCollection method;
				((CDrawMethod *)ObjPtr)->ExtractSymbol(tr,method);

				drawingIterator it = method.begin();
				drawingIterator itEnd = method.end();
				while ( it != itEnd ) 
				{
					CDrawingObject *pointer = *it;

					if (pointer->GetType()==xPinEx && !(thePin->IsPower()) ) 
					{
						CNetListNode n( file_index_id, sheet, thePin,thePin->GetActivePoint(theMethod));
						n.m_reference = theMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheet);
						n.m_pin = thePin->GetNumber();
						n.m_pMethod = theMethod;
						Add(n);
					}

					++ it;
				}

				/// Has this symbol had it's power connected?
				if (Connected.find(theMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheet)) == Connected.end()) 
				{
					Connected[ theMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheet) ] = TRUE;

					drawingCollection method;
					((CDrawMethod *)ObjPtr)->ExtractSymbol(tr,method);
					drawingIterator it = method.begin();
					drawingIterator itEnd = method.end();
					while (it!=itEnd) 
					{
						CDrawingObject *pointer = *it;

						if (pointer->GetType()==xPinEx && thePin->IsPower()) 
						{
							CNetListNode n(file_index_id, sheet, thePin,thePin->GetActivePoint(theMethod) );
							// Set netlist label name to invisible symbol power pin name
							n.setLabel( thePin->GetPinName() );
							n.m_reference = theMethod->GetRefSheet(m_prefix_references,m_prefix_import,file_index_id,sheet);
							n.m_pin = thePin->GetNumber();
							n.m_pMethod = theMethod;


							/// Look up the netlist this power belongs to
							found = Powers.find( thePin->GetPinName() );
							if (found != Powers.end())
								n.m_NetList = (*found).second;
							hold = Add(n);
							if (found == Powers.end())
								Powers[thePin->GetPinName()] = hold;
						}

						++ it;
					}
				}
			}			
			break;
		case xNoConnect:
			Add(CNetListNode(file_index_id, sheet, ObjPtr,ObjPtr->m_point_a));			
			break;
		case xJunction:
			Add(CNetListNode(file_index_id, sheet, ObjPtr,ObjPtr->m_point_a));
			break;			
		case xPower:
			{
			CNetListNode n(file_index_id, sheet, ObjPtr,ObjPtr->m_point_a);
			n.setLabel( ((CDrawPower *)ObjPtr)->GetValue() );

			/// Does this power item exist?
			found = Powers.find(((CDrawPower *)ObjPtr)->GetValue());
			if (found != Powers.end())
				n.m_NetList = (*found).second;
			hold = Add(n);
			if (found == Powers.end())
				Powers[((CDrawPower *)ObjPtr)->GetValue()] = hold;
			}
			break;
		case xWire:
			{
				CNetListNode n(file_index_id, sheet, ObjPtr,ObjPtr->m_point_a);
				hold = Add(n);
			}
			{
				CNetListNode n(file_index_id, sheet, ObjPtr,ObjPtr->m_point_b);
				n.m_NetList = hold;
				Add(n);
			}
			break;
	}
  }

  /// Search for junctions and connect together
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

		/// Look for wires which cross this junction
		drawingIterator search_it = itBegin;
		while (search_it != itEnd) 
		{
			CDrawingObject *search = *search_it;

			/// Find the wires
			/// If the wire has an end at this junction then it is already connected
			if (search->GetType()==xWire 
			 && search->m_point_a!=a && search->m_point_b!=a)
			{
				/// Is this point on this wire?
				CLineUtils l( search->m_point_a, search->m_point_b );
				double distance_along_a;

				if (l.IsPointOnLine( a, distance_along_a ))
				{
					CNetListNode n(file_index_id, sheet, NULL,search->m_point_a);
					n.m_NetList = NetNumber;
					NetNumber = Add(n);
				}
			}

			++ search_it;
		}
	}
	
	++ it;
  }

  /// Search for labels and connect to their respective lines
  stringCollection labels;
  it = itBegin;
  while (it != itEnd) 
  {
	CDrawingObject *ObjPtr = *it;

	/// Search for junctions
	if (ObjPtr->GetType() == xLabelEx2) 
	{
		CDPoint a = static_cast<CDrawLabel*>(ObjPtr)->GetLabelPoint();

		/// Search for a wire this label is connect to
		/// Only attempt to connect to a single wire
		drawingIterator search_it = itBegin;
		while (search_it != itEnd) 
		{
			CDrawingObject *search = *search_it;
			if (search->GetType()==xWire && search->IsInside(a.x,a.x,a.y,a.y)) 
			{
				a = search->m_point_a;
				break;
			}
			
			++ search_it;
		}

		/// Look up this label
		CNetListNode n(file_index_id, sheet, ObjPtr,a);
		n.setLabel(  ((CDrawLabel *)ObjPtr)->GetValue() );

		/// Has this label already been assigned a netlist?
		stringCollection::iterator found = labels.find(((CDrawLabel *)ObjPtr)->GetValue());
		if (found!=labels.end()) 
		{
			n.m_NetList = (*found).second;
			n.setLabel( _T("") );
		}

		int hold = Add(n);

		/// If there was no netlist write it back...
		if (found == labels.end())
			labels[ ((CDrawLabel *)ObjPtr)->GetValue()] = hold;
	}
	
	++ it;
  }  

  /// Our work with the nodes map is complete, so we can discard it now...
  m_nodes.erase( m_nodes.begin(), m_nodes.end() );

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

		if ((*fi)->m_file_name_index != 0)
		{
			dsn = (*fi)->m_pDesign;
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
					CString Ref  = pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->m_file_name_index,i+1);

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

				if (theNode.getLabel() != "" && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}
				else
				{
					CString add;
					if (!theNode.getLabel().IsEmpty())
					{
					/// NOT FOR Protel:	add = theNode.getLabel();
					}
					else if (!theNode.m_reference.IsEmpty())
					{
						add.Format(_T("%s-%s"), theNode.m_reference, theNode.m_pin );
					}

					if (!add.IsEmpty())
					{
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

		if ((*fi)->m_file_name_index != 0)
		{
			dsn = (*fi)->m_pDesign;
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
					CString Ref  = pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->m_file_name_index,i+1);

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

				if (theNode.getLabel() != _T("") && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}
				else
				{
					CString add;
					if (!theNode.getLabel().IsEmpty())
					{
					/// NOT FOR PADS:	add = theNode.getLabel();
					}
					else if (!theNode.m_reference.IsEmpty())
					{
						add.Format(_T("%s.%s"), theNode.m_reference, theNode.m_pin );
					}

					if (!add.IsEmpty())
					{
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

		if ((*fi)->m_file_name_index != 0)
		{
			dsn = (*fi)->m_pDesign;
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
				CString Ref  = pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->m_file_name_index,i+1);

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

				if (theNode.getLabel() != "" && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}
				else
				{
					CString add;
					if (!theNode.getLabel().IsEmpty())
					{
						add = theNode.getLabel();
					}
					else if (!theNode.m_reference.IsEmpty())
					{
						add.Format(_T("(%s,%s)"), theNode.m_reference, theNode.m_pin );
					}

					if (!add.IsEmpty())
					{
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

		if ((*fi)->m_file_name_index != 0)
		{
			dsn = (*fi)->m_pDesign;
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
				CString Ref  = pMethod->GetRefSheet(m_prefix_references,m_prefix_import,(*fi)->m_file_name_index,i+1);

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
			BOOL first = TRUE,PrintLine=FALSE, Labeled = FALSE;

			while (nv_it != (*nit).second.end()) 
			{
				CNetListNode& theNode = *nv_it;
				++ nv_it;

				if (theNode.getLabel() != _T("") && !Labeled) 
				{
					theLabel = theNode.getLabel();
					Labeled = TRUE;
				}
				else
				{
					CString add;
					if (!theNode.getLabel().IsEmpty())
					{
					}
					else if (!theNode.m_reference.IsEmpty())
					{
						add.Format(_T("   %s %s\n"), theNode.m_reference, theNode.m_pin );
					}

					if (!add.IsEmpty())
					{
						theLine += add;
						PrintLine=TRUE;
					}
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

		if ((*fi)->m_file_name_index != 0)
		{
			dsn = (*fi)->m_pDesign;
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

					CString spice_prolog;
					CString spice_epilog;
					int spice_pro_priority = 5;
					int spice_epi_priority = 5;

					for (int j = 0; j < pMethod->GetFieldCount(); j++)
					{
						CString field = pMethod->GetFieldName(j);
						if (field.CompareNoCase(AttrSpiceProlog) == 0)
						{
							CNetListSymbol symbol( (*fi)->m_file_name_index, sheet, pMethod );
							spice_prolog = expand_spice( (*fi)->m_file_name_index, sheet, symbol, labels, pMethod->GetField(j) );
						}
						else if (field.CompareNoCase(AttrSpiceEpilog) == 0)
						{
							CNetListSymbol symbol( (*fi)->m_file_name_index, sheet, pMethod );
							spice_epilog = expand_spice( (*fi)->m_file_name_index, sheet, symbol, labels, pMethod->GetField(j) );
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
		int file_name_index = symbol.m_file_name_index;

		/// Here is the data we are going to extract from the symbol's fields...
		
		CString spice = "";

		/// Search this symbols fields and extract the SPICE field...

		int i;
		for (i = 0; i < pMethod->GetFieldCount(); i++)
		{
			if (pMethod->GetFieldName(i).CompareNoCase(AttrSpice) == 0)
			{
				spice = pMethod->GetField(i);
			}
		}

		
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
	CDrawMethod* pMethod = symbol.m_pMethod;
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
			r = s;
			return true;
		}
		else
		{
			r = s.Mid(b);
			return true;
		}
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


CImportFile::~CImportFile()
{
	delete m_pDesign;
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
