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
#include "TinyCadMultiSymbolDoc.h"
#include "HeaderStamp.h"

////// The Symbol Field class //////

// Save this symbol field into an archive
void CSymbolField::Save(CStream &ar)
{
	DWORD ft = field_type;
	ar << field_name << field_value << field_default << field_pos << ft;
}

void CSymbolField::SaveXML(CXMLWriter &xml, bool simple)
{
	DWORD ft = field_type;
	xml.addTag(_T("FIELD"));
	xml.addTag(_T("NAME"), field_name );
	xml.addTag(_T("DEFAULT"), field_default );
	xml.addTag(_T("FT"), ft );
	if (!simple)
	{
		xml.addTag(_T("VALUE"), field_value );
		xml.addTag(_T("POS"), CDPoint(field_pos) );
	}
	xml.closeTag();
}


// Load this symbol field from an archive
void CSymbolField::Load(CStream &ar)
{
	DWORD ft;
	CPoint fp;
	ar >> field_name >> field_value >> field_default >> fp >> ft;
	field_pos = CDPoint(fp.x,fp.y);
	field_type = (SymbolFieldType)ft;
}

void CSymbolField::LoadXML(CXMLReader &xml)
{
	DWORD ft = field_type;
	CString tag;

	while (xml.nextTag( tag ))
	{
		if (tag == _T("NAME"))
		{
			xml.getChildData( field_name );
		}
		else if (tag == _T("VALUE"))
		{
			xml.getChildData( field_value );
		}
		else if (tag == _T("DEFAULT"))
		{
			xml.getChildData( field_default );
		}
		else if (tag == _T("POS"))
		{
			xml.getChildData( field_pos );
		}
		else if (tag == _T("FT"))
		{
			xml.getChildData( ft );
			field_type = static_cast<SymbolFieldType>(ft);
		}
	}
}

////// The Symbol Record class //////

// The constructor
CSymbolRecord::CSymbolRecord()
{
	name_type = default_show;
	ref_type = default_show;
    fields_loaded = FALSE;
    NameID = -1;
}


// Save this symbol into an XML file
void CSymbolRecord::SaveXML(CXMLWriter &xml)
{
	xml.addTag( _T("NAME") ); 
	xml.addAttribute( _T("type"), name_type );
	xml.addChildData( name );
	xml.closeTag();

	xml.addTag( _T("REF") );
	xml.addAttribute( _T("type"), ref_type );
	xml.addChildData( reference );
	xml.closeTag();

	xml.addTag( _T("DESCRIPTION"), description );
	
	for (unsigned int i = 0; i < fields.size(); i++)
	{
		fields[i].SaveXML( xml, true );
	}
}

// Load this symbol from an XML file
void CSymbolRecord::LoadXML( CXMLReader &xml)
{
	int nt = 0;
	int rt = 0;
	CString tag_name;
	while (xml.nextTag( tag_name ))
	{
		if (tag_name == _T("NAME"))
		{
			xml.getChildData( name );
			xml.getAttribute( _T("type"), nt );
			name_type = static_cast<SymbolFieldType>(nt);
		}
		else if (tag_name == _T("REF"))
		{
			xml.getChildData( reference );
			xml.getAttribute( _T("type"), rt );
			ref_type = static_cast<SymbolFieldType>(rt);
		}
		else if (tag_name == _T("DESCRIPTION"))
		{ 
			xml.getChildData( description );
		}
		else if (tag_name == _T("FIELD"))
		{
			CSymbolField f;
			xml.intoTag();			
			f.LoadXML( xml );
			xml.outofTag();
			fields.push_back( f );
		}
	}
}






////// The Symbol class //////

// The constructor
CLibraryStoreNameSet::CLibraryStoreNameSet(CLibraryStore *NewLib)
{
  lib=NewLib;
  FilePos = -1;
}

// The destructor
CLibraryStoreNameSet::~CLibraryStoreNameSet()
{
}

CString CLibraryStoreNameSet::GetLibName()
{
  return lib->m_name;
}



// Save this symbol into an archive
void CLibraryStoreNameSet::Save(CStream &theArchive)
{
	CSymbolRecord &r = GetRecord(0);

	DWORD flags = r.name_type << 3 | r.ref_type;
	DWORD number_of_fields = r.fields.size() | (flags << 16);
	theArchive << r.name << r.description << r.reference << ppp << number_of_fields; 

	for (unsigned int i = 0; i < r.fields.size(); i++)
	{
	  r.fields[i].Save( theArchive );
	}
}


// Save this NameSet as an XML export
void CLibraryStoreNameSet::SaveXML( CXMLWriter &xml )
{
	xml.addTag( _T("PPP"), ppp );
	xml.addTag( _T("ORIENTATION"), orientation );

	for (int i =0; i < GetNumRecords(); i++)
	{
		xml.addTag(_T("DETAILS"));
		CSymbolRecord &r = GetRecord(i);
		r.SaveXML( xml );
		xml.closeTag();
	}
}

// Load this NameSet as an XML export
void CLibraryStoreNameSet::LoadXML(CTinyCadMultiSymbolDoc *pDesign, CXMLReader &xml)
{
	int nt = 0;
	int rt = 0;
	CString tag;

	while (xml.nextTag( tag ))
	{
		if (tag == _T("PPP"))
		{
			xml.getChildData( ppp );
		}
		else if (tag == _T("ORIENTATION"))
		{
			xml.getChildData( orientation );
		}
		else if (tag == _T("DETAILS"))
		{
			CLibraryStoreSymbol r;
			xml.intoTag();
			r.LoadXML( xml );
			xml.outofTag();

			PushBackRecord( r );
		}
		else if (tag == _T("TinyCAD"))
		{
			pDesign->LoadXML(xml, false );
		}
		else if (tag == _T("TinyCADSheets"))
		{
			pDesign->LoadXML(xml, true );
		}
	}
}


// load this symbol from an archive
void CLibraryStoreNameSet::Load(CStream &theArchive)
{
	Blank();
	CSymbolRecord &r = GetRecord(0);


	DWORD number_of_fields;
	FilePos = 0;
	theArchive >> r.name >> r.description >> r.reference >> ppp >> number_of_fields; 
	DWORD flags = number_of_fields >> 16;
	number_of_fields = number_of_fields & 0xffff;
	for (unsigned int i = 0; i < number_of_fields; i++)
	{
	  CSymbolField sf;
	  sf.Load( theArchive );
	  r.fields.push_back( sf );
	}

	r.name_type = static_cast<SymbolFieldType>(flags >> 3);
	r.ref_type = static_cast<SymbolFieldType>(flags & 0x7);
}

// load this symbol from an archive
void CLibraryStoreNameSet::OldLoad3(CStream &theArchive)
{
	CString dummy;
	Blank();
	CSymbolRecord &r = GetRecord(0);

    theArchive >> r.name >> r.description >> r.reference >> ppp >> FilePos >> dummy; 
}


// load this symbol from an archive
void CLibraryStoreNameSet::OldLoad1(CStream &theArchive)
{
	Blank();
	CSymbolRecord &r = GetRecord(0);

	theArchive >> r.name >> r.description >> r.reference >> ppp >> FilePos; 
}


// the old version of load this symbol from an archive
void CLibraryStoreNameSet::OldLoad2(CStream &theArchive)
{
	Blank();
	CSymbolRecord &r = GetRecord(0);

	theArchive >> r.name >> r.reference >> ppp >> FilePos; 
	r.description = "";
}


CStream *CLibraryStoreNameSet::GetMethodArchive() 
{ 
	return lib->GetMethodArchive( this ); 
}

CDesignFileSymbol *CLibraryStoreNameSet::GetDesignSymbol( CTinyCadDoc *pDesign, int index )
{
	CDesignFileSymbol *psymbol = new CDesignFileSymbol;
    CSymbolRecord &r = GetRecord(index);

	psymbol->description = r.description;
	psymbol->name = r.name;
	psymbol->ppp = ppp;
	psymbol->reference = r.reference;
	psymbol->name_type = r.name_type;
	psymbol->ref_type = r.ref_type;
	psymbol->fields = r.fields;
	psymbol->FilePos = FilePos;

	CStream *stream=GetMethodArchive();
	if (!stream)
	{
		delete psymbol;
		return NULL;
	}

	if (! psymbol->LoadSymbol( pDesign, *stream ) )
	{
		delete psymbol;
		delete stream;
		return NULL;
	}

	// Get the default orientation
	orientation = stream->GetOrientation();
	delete stream;

	return psymbol;
}


void CLibraryStoreNameSet::Blank()
{
	m_records.clear();

	CLibraryStoreSymbol r;
	r.description = "New symbol";
	FilePos = -1;
	r.name = "New symbol";
	ppp = 1;
	r.reference = "U?";
	r.NameID = -1;
	r.m_index = 0;
	r.m_pParent = this;
	orientation = 0;

	m_records.push_back( r );
}


CLibraryStoreSymbol& CLibraryStoreNameSet::GetRecord( int index )
{
	CLibraryStoreSymbol &r = m_records[ index ];

	// Enforce the parent parameters...
	r.m_index = index;
	r.m_pParent = this;

	return r;
}

int	CLibraryStoreNameSet::GetNumRecords()
{
	return m_records.size();
}

void CLibraryStoreNameSet::SetRecords( recordCollection& records )
{
	m_records = records;
}

void CLibraryStoreNameSet::PushBackRecord( CLibraryStoreSymbol &r )
{
	r.m_index = m_records.size();
	r.m_pParent = this;
	m_records.push_back( r );
}


///// The symbol as contained by the library set /////


CLibraryStoreSymbol::CLibraryStoreSymbol()
{
	m_index = 0;
	m_pParent = NULL;
}

CDesignFileSymbol *CLibraryStoreSymbol::GetDesignSymbol( CTinyCadDoc *pDesign )
{
	return m_pParent->GetDesignSymbol( pDesign, m_index );
}




///// The symbol as contained by the design /////

CDesignFileSymbol::CDesignFileSymbol()
{
	m_pDesign = NULL;
	FilePos = -1;
	ppp = 1;
	m_heterogeneous = false;
}

// The Destructor
CDesignFileSymbol::~CDesignFileSymbol()
{
	// Delete the primary method
	symbolCollection::iterator s = m_methods.begin();
	while (s != m_methods.end())
	{
		drawingIterator it = (*s).begin();
		while (it != (*s).end()) 
		{
			delete *it;
			++ it;
		}

		++s;
	}

	// Now delete the cached methods too...
	FilteredSymbolCollection::iterator i = m_filter_cache.begin();
	while (i != m_filter_cache.end())
	{
		drawingIterator it = i->second.begin();
		while (it != i->second.end()) 
		{
			delete *it;
			++ it;
		}

		++ i;
	}	
}
	
// Create default blank symbol
void CDesignFileSymbol::CreateNoSymbol( CTinyCadDoc *pDesign )
{
	// Create the no symbol object
	CDrawSquare *Sq = new CDrawSquare(pDesign,xSquareEx3);
	Sq->m_point_a=CDPoint(0,0);
	Sq->m_point_b=CDPoint(-40,-40);
	m_methods.resize( 1 );
	m_methods[0].push_back( Sq );
	m_pDesign = pDesign;
	name = "No Symbol Defined";
	reference="?";
	ppp = 1;  
	m_heterogeneous = false;
}


// Save this symbol into an archive
void CDesignFileSymbol::SaveXML(CXMLWriter &xml)
{
	xml.addTag( _T("NAME" )); 
	xml.addAttribute( _T("type"), name_type );
	xml.addChildData( name );
	xml.closeTag();

	xml.addTag( _T("REF" ));
	xml.addAttribute( _T("type"), ref_type );
	xml.addChildData( reference );
	xml.closeTag();

	xml.addTag( _T("DESCRIPTION"), description );
	xml.addTag( _T("PPP"), ppp );
	
	// Save the fields associated with this symbol
	for (unsigned int i = 0; i < fields.size(); i++)
	{
		fields[i].SaveXML( xml, false );
	}

	// Save the reference points we have generated for this symbol
	FilteredSymbolCollection::iterator s = m_filter_cache.begin();
	while (s != m_filter_cache.end())
	{
		CDesignFileSymbolFilter f = (s->first);

		// Only store one ref point for homogeneous symbols
		if (s->first.m_part == 0 || IsHeterogeneous() )
		{
			xml.addTag( _T("REF_POINT") );
			xml.addAttribute( _T("power"), f.m_include_power_pins );
			xml.addAttribute( _T("part"), f.m_part );
			xml.addAttribute( _T("pos"), f.m_reference_point );
			xml.closeTag();
		}

		++ s;
	}	


	// Save the symbol's outline
	if (m_heterogeneous)
	{
		xml.addTag( _T("TinyCADSheets") );
		symbolCollection::iterator s = m_methods.begin();
		while (s != m_methods.end())
		{
			m_pDesign->SaveXML(xml, *s, FALSE,FALSE,FALSE);
			++s;
		}
		xml.closeTag();
	}
	else
	{
		m_pDesign->SaveXML(xml, m_methods[0], FALSE,FALSE,FALSE);
	}
}


// Load this symbol from an archive
void CDesignFileSymbol::LoadXML(CTinyCadDoc *pDesign, CXMLReader &xml)
{
	int nt = 0;
	int rt = 0;
	CString tag;
	m_pDesign = pDesign;

	while (xml.nextTag( tag ))
	{
		if (tag == _T("NAME"))
		{
			xml.getChildData( name );
			xml.getAttribute( _T("type"), nt );
			name_type = static_cast<SymbolFieldType>(nt);
		}
		else if (tag == _T("REF"))
		{
			xml.getChildData( reference );
			xml.getAttribute( _T("type"), rt );
			ref_type = static_cast<SymbolFieldType>(rt);
		}
		else if (tag == _T("DESCRIPTION"))
		{ 
			xml.getChildData( description );
		}
		else if (tag == _T("PPP"))
		{
			xml.getChildData( ppp );
		}
		else if (tag == _T("FIELD"))
		{
			CSymbolField f;
			xml.intoTag();			
			f.LoadXML( xml );
			xml.outofTag();
			fields.push_back( f );
		}
		else if (tag == _T("REF_POINT"))
		{
			CDesignFileSymbolFilter f;
			drawingCollection drawing;

			xml.getAttribute( _T("power"), f.m_include_power_pins );
			xml.getAttribute( _T("part"), f.m_part );
			xml.getAttribute( _T("pos"), f.m_reference_point );

		  	m_filter_cache.insert(std::pair<CDesignFileSymbolFilter,drawingCollection>(f,drawing));
		}
		else if (tag == _T("TinyCAD"))
		{
			m_methods.resize( 1 );
			m_pDesign->ReadFileXML(xml, FALSE, m_methods[0], TRUE );
			m_heterogeneous = false;
		}
		else if (tag == _T("TinyCADSheets" ))
		{
			xml.intoTag();
			m_methods.resize( 0 );

			CString next;
			int index = 0;
			while (xml.nextTag( next ))
			{
				if (next == "TinyCAD")
				{
					m_methods.resize( index + 1 );
					m_pDesign->ReadFileXML(xml, FALSE, m_methods[index], TRUE );
					++ index;
				}
			}
			xml.outofTag();
			m_heterogeneous = true;
		}
	}
}



// Load this symbol from an archive
void CDesignFileSymbol::Load(CTinyCadDoc *pDesign, CStream &theArchive)
{
	CString dummy;
	theArchive >> name >> description >> reference >> ppp >> dummy; 

	DWORD number_of_fields = 0;
	if (!dummy.IsEmpty())
	{
		number_of_fields = _ttoi( dummy );
	}

    DWORD flags = number_of_fields >> 16;
    number_of_fields = number_of_fields & 0xffff;

	for (unsigned int i = 0; i < number_of_fields; i++)
	{
	  CSymbolField sf;
	  sf.Load( theArchive );
	  fields.push_back( sf );
	}

	name_type = static_cast<SymbolFieldType>(flags >> 3);
	ref_type = static_cast<SymbolFieldType>(flags & 0x7);

	LoadSymbol( pDesign, theArchive );
}

BOOL CDesignFileSymbol::LoadSymbol( CTinyCadDoc *pDesign, CStream &theArchive )
{
  try
  {
	LONG pos = theArchive.GetPos();
	CHeaderStamp oHeader;
	oHeader.Read( theArchive );
	theArchive.Seek(pos);

	if( oHeader.IsChecked(false) ) 
	{
		// Load in the design
		m_methods.resize( 1 );
		m_heterogeneous = false;
		if (!pDesign->ReadFile( theArchive, FALSE, m_methods[0] ))
		{
			return FALSE;
		}
	}
	else
	{
		CXMLReader xml( &theArchive );
		CString tag;
		xml.nextTag( tag );
		if (tag == "TinyCADSheets")
		{
			xml.intoTag();
			CString tag;
			int index = 0;
			m_methods.resize( 0 );
			while (xml.nextTag( tag ))
			{
				if (tag == "TinyCAD")
				{
					m_methods.resize( index + 1 );
					pDesign->ReadFileXML(xml,TRUE,m_methods[index],TRUE);
					++ index;
				}
			}
			xml.outofTag();
			m_heterogeneous = true;
		}
		else if (tag == "TinyCAD")
		{
			m_methods.resize( 1 );
			pDesign->ReadFileXML(xml,TRUE,m_methods[0],TRUE);
			m_heterogeneous = false;
		}
		else
		{
			// Not much to do about this!
			return FALSE;
		}
	}
  }
  catch (...)
  {
	  // Failed to load the symbol
	  return FALSE;
  }

  m_pDesign = pDesign;
  return TRUE;
}

void CDesignFileSymbol::CreateSymbol(CTinyCadDoc *pDesign, drawingCollection &drawing, const CDesignFileSymbolFilter& filter)
{
  // Find the co-ords of the bounding box of this symbol
  CDPoint a=CDPoint(0,0);
  CDPoint b=CDPoint(0,0);
  bool first = true;
 
  BOOL has_pins = FALSE;
  CDPoint pin = CDPoint(0,0);

  drawingCollection& method = m_heterogeneous ? m_methods[ filter.m_part ] : m_methods[0];

  // CDPoint b=method.front()->m_point_a;

  // Make a copy of the symbol from the master method,
  // making any changes required as we go...
  //
  drawingIterator it = method.begin();
  while (it != method.end()) 
  {
      CDrawingObject* pointer = pDesign->Dup( *it );
	  ++ it;

	  CDrawPin *thePin = static_cast<CDrawPin*>(pointer);

	if (pointer->GetType()==xPinEx)
	{

		// Special treatment for power pins...
		if (thePin->IsPower())
		{
			// Do we need to convert it to a normal pin?
			if (filter.m_include_power_pins)
			{
				// Now insert the copy
				thePin->ConvertPowerToNormal();
			}
		}

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


	// Is this the right part in package?
	if (!m_heterogeneous 
		&& pointer->GetType() == xPinEx 
		&& thePin->GetPart() != filter.m_part )
	{
		// No, so don't copy it!
		delete pointer;
	}
	else
	{
		if (first)
		{
			b=pointer->m_point_a;
			first = false;
		}

		a.x=max(a.x,max(pointer->m_point_a.x,pointer->m_point_b.x));
		a.y=max(a.y,max(pointer->m_point_a.y,pointer->m_point_b.y));

		b.x=min(b.x,min(pointer->m_point_a.x,pointer->m_point_b.x));
		b.y=min(b.y,min(pointer->m_point_a.y,pointer->m_point_b.y));

		// Yes, so make a copy to our list...
		drawing.push_back( pointer );
	}
  }

  // If this symbol has pins, then snap the pin to the grid,
  // rather than the bounding rect
  CDPoint snapa,snapb;
  
  // Do we already have a ref-point?
  FilteredSymbolCollection::iterator fx = m_filter_cache.find( filter );
  if (fx != m_filter_cache.end())
  {
	  snapa = fx->first.m_reference_point;
	  m_filter_cache.erase( fx );
  }
  else
  {
	CDPoint pin_offset = CDPoint(0,0);

	// Snap the co-ords of the bounding box to the grid
	snapa = pDesign->m_snap.Snap(a);
	snapb = pDesign->m_snap.Snap(b);

	// Now make sure the pins stay on-grid
	if (has_pins)
	{
		// Snap the co-ords of the pin to the grid
		pin_offset=pin - pDesign->m_snap.Snap(pin);

		snapa = snapa + pin_offset;
		snapb = snapb + pin_offset;
	}

	  
	// Make sure snapping is always positive
	if (snapa.x<a.x)
		snapa.x+=pDesign->m_snap.GetGrid();
	if (snapa.y<a.y)
		snapa.y+=pDesign->m_snap.GetGrid();
	if (snapb.x>b.x)
		snapb.x-=pDesign->m_snap.GetGrid();
	if (snapb.y>b.y)
		snapb.y-=pDesign->m_snap.GetGrid();
  }

  // Now translate so that bottom left hand corner is at 0,0
  it = drawing.begin();
  while (it != drawing.end()) 
  {
	  (*it)->Shift( CDPoint(-snapa.x,-snapa.y) );
  	  ++ it;
  }

    // Set the reference point
    CDesignFileSymbolFilter f = filter;
	f.m_reference_point = snapa;

	// Set the top-right point
	f.m_top_right = GetTr(pDesign, drawing );

  	m_filter_cache[f] = drawing;
}



// Return the methods object
BOOL CDesignFileSymbol::GetMethod( int part, bool include_power_pins, drawingCollection &drawing )
{
	// First check the cache!
	CDesignFileSymbolFilter f( part, include_power_pins );
	FilteredSymbolCollection::iterator it = m_filter_cache.find( f );

	if (it != m_filter_cache.end() && it->second.size() > 0)
	{
		drawing = it->second;
	}
	else
	{
		CreateSymbol( m_pDesign, drawing, f );
	}

	return TRUE;
}

CDPoint CDesignFileSymbol::GetTr( int part, bool include_power_pins )
{
	// Only use the part number if we are a heterogeneous symbol
	if (!IsHeterogeneous())
	{
		part = 0;
	}

	// First check the cache!
	CDesignFileSymbolFilter f( part, include_power_pins );
	FilteredSymbolCollection::iterator it = m_filter_cache.find( f );

	if (it != m_filter_cache.end() && it->second.size() > 0)
	{
		// Top-right point already in the cache so use it!
		return it->first.m_top_right;
	}
	else
	{
		// Drawing not created yet - so create it
		drawingCollection drawing;
		if (m_methods[0].size() > 0)
		{
			CreateSymbol( m_methods[0].front()->m_pDesign, drawing, f );

			// .. and get the top-right point from the cache
			it = m_filter_cache.find( f );
			if (it != m_filter_cache.end())
			{
				return it->first.m_top_right;
			}
		}
	}

	return CDPoint( 0,0 );
}


CDPoint CDesignFileSymbol::GetTr(CTinyCadDoc *pDesign, drawingCollection &drawing )
{
	// Find the co-ords of the bounding box of this symbol
	CDPoint a=CDPoint(0,0);
	CDPoint b=CDPoint(0,0);
	
	if (drawing.size()>0)
	{
		b = drawing.front()->m_point_a;
	}

	CDPoint pin = CDPoint(0,0);

	drawingIterator it = drawing.begin();
	while (it != drawing.end()) 
	{
		CDrawingObject *pointer = *it;
		CDrawPin *thePin = static_cast<CDrawPin*>(pointer);

		if (! (pointer->GetType() == xPinEx && thePin->IsInvisible()) )
		{
			a.x=max(a.x,max(pointer->m_point_a.x,pointer->m_point_b.x));
			a.y=max(a.y,max(pointer->m_point_a.y,pointer->m_point_b.y));

			b.x=min(b.x,min(pointer->m_point_a.x,pointer->m_point_b.x));
			b.y=min(b.y,min(pointer->m_point_a.y,pointer->m_point_b.y));
		}

		++ it;
	}


	  // If this symbol has pins, then snap the pin to the grid,
  // rather than the bounding rect
  CDPoint snapa, snapb;

  // Snap the co-ords of the bounding box to the grid
  snapa = pDesign->m_snap.Snap(a);
  snapb = pDesign->m_snap.Snap(b);
  
  // Make sure snapping is always positive
  if (snapa.x<a.x)
	snapa.x+=pDesign->m_snap.GetGrid();
  if (snapa.y<a.y)
	snapa.y+=pDesign->m_snap.GetGrid();
  if (snapb.x>b.x)
	snapb.x-=pDesign->m_snap.GetGrid();
  if (snapb.y>b.y)
	snapb.y-=pDesign->m_snap.GetGrid();

	// Return top right hand corner
	return CDPoint(snapb.x - snapa.x ,snapb.y - snapa.y );
}

