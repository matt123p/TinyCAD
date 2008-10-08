/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002 Matt Pyne.

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
#include "tinycad.h"
#include "LibraryDb.h"
#include "DbLibNameSet.h"
#include "StreamDb.h"
#include "TinyCadSymbolDoc.h"
#include "DbAttributeSet.h"
#include "TinyCadMultiSymbolDoc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLibraryDb::CLibraryDb()
{

}

CLibraryDb::~CLibraryDb()
{

}


// Attach this library to a file
void CLibraryDb::Attach(const TCHAR *filename)
{
	m_name=filename;


	try
	{
		if (!m_database.IsOpen())
		{
			m_database.Open( m_name + ".mdb" );
		}

		// Now read the Symbols from this database
		CDbLibNameSet name_set( &m_database );
		name_set.Open();

		while (!name_set.IsEOF())
		{

			// Is this a new symbol?
			bool is_new = m_Symbols.find( name_set.m_SymbolID ) == m_Symbols.end();
			CLibraryStoreNameSet &nwSymbol = m_Symbols[name_set.m_SymbolID];

			if (is_new)
			{
				// Yes!
				nwSymbol.Blank();
				nwSymbol.lib = this;
				nwSymbol.FilePos = name_set.m_SymbolID;
				nwSymbol.ppp = static_cast<BYTE>(name_set.m_ppp);
			}

			if (name_set.m_Type == 0)
			{
				CLibraryStoreSymbol r;
				r.name = name_set.m_Name;
				r.NameID = name_set.m_NameID;
				r.reference = name_set.m_Reference;
				r.description = name_set.m_Description;
				r.name_type = static_cast<SymbolFieldType>(name_set.m_ShowName);
				r.ref_type = static_cast<SymbolFieldType>(name_set.m_ShowRef);

				if (is_new)
				{
					recordCollection records;
					records.push_back( r );
					nwSymbol.SetRecords( records );
				}
				else
				{
					nwSymbol.PushBackRecord( r );
				}


				// Now add to our map of m_Symbols
				// m_Symbols[ r.name ] = nwSymbol;
			}
			
			name_set.MoveNext();
		}
	}
	catch( CException *e)
	{
		CString s;
		CString msg;
		e->GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot open library %s.\r\n%s"),
			m_name, msg );
		AfxMessageBox( s );
		e->Delete();
		return;
	}
}


// Write a symbol to this library
void CLibraryDb::Store( CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document )
{
	// Set the busy cursor
	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_WAIT ) );

	// First clear out all of the old names...
	if (nwSymbol->FilePos != -1)
	{
		CDbLibNameSet name_set( &m_database );
		name_set.m_strFilter.Format(_T("[SymbolID]=%d"),nwSymbol->FilePos);
		name_set.Open();

		CString sql;
		while (!name_set.IsEOF())
		{
			sql.Format( _T("DELETE FROM [Attribute] WHERE [NameID]=%d"), name_set.m_NameID );
			m_database.Execute( sql );
			name_set.Delete();
			name_set.MoveNext();
		}
	}

	// Write the symbol data into the methods file
	CStreamDb stream( &m_database, FALSE, nwSymbol->FilePos, nwSymbol->orientation );
	CXMLWriter xml( &stream );
	document.SaveXML(xml);
	stream.Flush();

	// Recover the new symbol id
	nwSymbol->FilePos = stream.m_set.m_SymbolID;

	// Do this for each of the names in the symbol set
	for (int i =0; i < nwSymbol->GetNumRecords(); i++)
	{
		CSymbolRecord &r = nwSymbol->GetRecord( i );

		// Write back the name...
		CDbLibNameSet name_set( &m_database );

		name_set.Open();
		name_set.AddNew();

		name_set.m_Name = r.name;
		name_set.m_SymbolID = nwSymbol->FilePos;
		name_set.m_Type = 0;
		name_set.m_Reference = r.reference;
		name_set.m_ppp = nwSymbol->ppp;
		name_set.m_Description = r.description ;
		name_set.m_ShowName = static_cast<int>(r.name_type);
		name_set.m_ShowRef = static_cast<int>(r.ref_type);

		name_set.Update();

		// Now recover the name this was associated with...
		name_set.MoveLast();

		// First delete the old attributes
		CString sql;
		sql.Format( _T("DELETE FROM [Attribute] WHERE [NameID]=%d"), name_set.m_NameID );
		m_database.Execute( sql );

		// Now write back the attributes...
		CDbAttributeSet attr_set( &m_database );
		attr_set.Open();
		std::vector<CSymbolField>::iterator it = r.fields.begin();
		while (it != r.fields.end())
		{
			attr_set.AddNew();
			attr_set.m_NameID = name_set.m_NameID;
			attr_set.m_AttName = (*it).field_name;
			attr_set.m_AttValue = (*it).field_default;
			attr_set.m_DisplayFlags = static_cast<int>((*it).field_type);
			attr_set.Update();
			++ it;
		}
	}

	// Inform the design it has been saved
	document.SetModifiedFlag(FALSE);

	// Re-load the library now it has changed
	ReRead();

	SetCursor( AfxGetApp()->LoadStandardCursor( IDC_ARROW ) );
}

// Delete a symbol from the database
void CLibraryDb::DeleteSymbol( int SymbolID )
{
	// Build a deletion list...
	CDbLibNameSet name_set( &m_database );
	name_set.m_strFilter.Format(_T("[SymbolID]=%d"),SymbolID);
	name_set.Open();

	CString sql;
	while (!name_set.IsEOF())
	{
		sql.Format( _T("DELETE FROM [Attribute] WHERE [NameID]=%d"), name_set.m_NameID );
		m_database.Execute( sql );
		name_set.Delete();
		name_set.MoveNext();
	}

	// delete this name...
	sql.Format( _T("DELETE FROM [Symbol] WHERE [SymbolID]=%d"), SymbolID );
	m_database.Execute( sql );
}

// Delete a symbol from this library
void CLibraryDb::DeleteSymbol( CLibraryStoreNameSet &symbol )
{
	// Delete the symbol from this library...
	DeleteSymbol( symbol.FilePos );

	// Now re-read with the changes...
	ReRead();
}



#if 0
// Write back a symbol collection and rebuild methods file
// (i.e. partial tidy)
void CLibraryDb::SaveSymbolCollection( symbolCollection &temp_m_Symbols )
{
	std::set<int> del_set;

	// Now read the m_Symbols from this database
	CDbLibNameSet name_set( &m_database );
	name_set.Open();

	while (!name_set.IsEOF())
	{
		// Find this name in the set...
	    symbolCollection::iterator it = temp_m_Symbols.begin();
	    while (it != temp_m_Symbols.end()) 
		{
			CLibraryStoreNameSet& thisSymbol = it->second;

			// Do this for each of the names in the symbol set
			for (int i =0; i < thisSymbol.GetNumRecords(); i++)
			{
				CSymbolRecord &r = thisSymbol.GetRecord( i );

				if (r.NameID == name_set.m_NameID)
				{
					// Found it - so update it...
					name_set.Edit();
					name_set.m_Name = r.name;
					name_set.m_SymbolID = thisSymbol.FilePos;
					name_set.m_Type = 0;
					name_set.m_Reference = r.reference;
					name_set.m_ppp = thisSymbol.ppp;
					name_set.m_Description = r.description ;
					name_set.m_ShowName = static_cast<int>(r.name_type);
					name_set.m_ShowRef = static_cast<int>(r.ref_type);
					name_set.Update();
					break;
				}
			}

			++ it;
		}

		// Did we find it?
		if (it == temp_m_Symbols.end())
		{
			// No, so tag it for deletion delete it...
			del_set.insert( name_set.m_SymbolID );
		}


		name_set.MoveNext();
	}

	// Now delete the m_Symbols we have tagged for deletion
	std::set<int>::iterator itd = del_set.begin();
	while (itd != del_set.end())
	{
		DeleteSymbol( *itd );
		++ itd;
	}

	ReRead();
}
#endif

// Do any idle time tasks...
void CLibraryDb::OnIdle()
{
}

// Get the Archive to load from
CStream *CLibraryDb::GetMethodArchive( CLibraryStoreNameSet *symbol)
{
	// Is this a brand new symbol?
	if (symbol->FilePos == -1)
	{
		return NULL;
	}

	// Now get the attributes
	// Do this for each of the names in the symbol set
	for (int i =0; i < symbol->GetNumRecords(); i++)
	{
		CSymbolRecord &r = symbol->GetRecord( i );

		if (!r.fields_loaded)
		{
			CDbAttributeSet attr_set( &m_database );
			attr_set.m_strFilter.Format(_T("[NameID]=%d"),r.NameID);
			attr_set.Open();
			r.fields.erase( r.fields.begin(), r.fields.end() );
			while (!attr_set.IsEOF())
			{
				CSymbolField field;
				field.field_name = attr_set.m_AttName;
				field.field_default = attr_set.m_AttValue;
				field.field_type = static_cast<SymbolFieldType>(attr_set.m_DisplayFlags);
				r.fields.push_back( field );

				attr_set.MoveNext();
			}

			r.fields_loaded = TRUE;
		}
	}

	return new CStreamDb( &m_database, TRUE, symbol->FilePos );
}

// Create a new library database
bool CLibraryDb::Create( const TCHAR *filename )
{
	m_name=filename;

	// Now create the database
	try
	{
		if (m_database.IsOpen())
		{
			m_database.Close();
		}

		m_database.Create( m_name + ".mdb" );
	}
	catch( CException *e)
	{
		CString s;
		CString msg;
		e->GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot create library %s.\r\n%s"),
			m_name, msg );
		AfxMessageBox( s );
		e->Delete();
		return false;
	}

	// .. and create the default tables
	try
	{
		/////////////////////// NAME TABLE /////////////////////// 

		CDaoTableDef name_table( &m_database );
		name_table.Create( _T("Name") );
		name_table.CreateField(_T("NameID"), dbLong, 0, dbAutoIncrField );
		name_table.CreateField(_T("Name"), dbText, 128 );
		name_table.CreateField(_T("SymbolID"), dbLong, 0 );
		name_table.CreateField(_T("Type"), dbLong, 0 );
		name_table.CreateField(_T("Reference"), dbText, 255 );
		name_table.CreateField(_T("ppp"), dbLong, 0 );
		name_table.CreateField(_T("Description"), dbText, 255 );
		name_table.CreateField(_T("ShowName"), dbLong, 0 );
		name_table.CreateField(_T("ShowRef"), dbLong, 0 );

		// Now create the primary key (what an effort!)
		CDaoIndexInfo name_index;
		CDaoIndexFieldInfo name_index_field;

		name_index_field.m_bDescending = FALSE;
		name_index_field.m_strName = _T("NameID");

		name_index.m_strName = _T("Name_PK");
		name_index.m_bPrimary = TRUE;
		name_index.m_pFieldInfos = &name_index_field;
		name_index.m_nFields = 1;
		name_index.m_bUnique = FALSE;
		name_index.m_bClustered = FALSE;
		name_index.m_bIgnoreNulls = FALSE;
		name_index.m_bRequired = FALSE;
		name_index.m_bForeign = FALSE;
		name_index.m_lDistinctCount = 0;
		
		name_table.CreateIndex( name_index );

		// Now add the table to the database..
		name_table.Append();




		/////////////////////// ATTRIBUTE TABLE /////////////////////// 
		
		CDaoTableDef attr_table( &m_database );
		attr_table.Create( _T("Attribute") );
		attr_table.CreateField(_T("AttributeID"), dbLong, 0, dbAutoIncrField );
		attr_table.CreateField(_T("NameID"), dbLong, 0 );
		attr_table.CreateField(_T("AttName"), dbText, 64 );
		attr_table.CreateField(_T("AttValue"), dbMemo, 0 );	// Note: change to v2 of library!
		attr_table.CreateField(_T("ShowAtt"), dbLong, 0 );

		// Now create the primary key (what an effort!)
		CDaoIndexInfo attr_index;
		CDaoIndexFieldInfo attr_index_field;

		attr_index_field.m_bDescending = FALSE;
		attr_index_field.m_strName = _T("AttributeID");

		attr_index.m_strName = "Attribute_PK";
		attr_index.m_bPrimary = TRUE;
		attr_index.m_pFieldInfos = &attr_index_field;
		attr_index.m_nFields = 1;
		attr_index.m_bUnique = FALSE;
		attr_index.m_bClustered = FALSE;
		attr_index.m_bIgnoreNulls = FALSE;
		attr_index.m_bRequired = FALSE;
		attr_index.m_bForeign = FALSE;
		attr_index.m_lDistinctCount = 0;
		
		attr_table.CreateIndex( attr_index );

		// Now add the table to the database..
		attr_table.Append();



		/////////////////////// SYMBOL TABLE /////////////////////// 
		
		CDaoTableDef symbol_table( &m_database );
		symbol_table.Create( _T("Symbol") );
		symbol_table.CreateField(_T("SymbolID"), dbLong, 0, dbAutoIncrField );
		symbol_table.CreateField(_T("Data"), dbLongBinary, 0 );
		symbol_table.CreateField(_T("DrawRotate"), dbLong, 0 );
		symbol_table.CreateField(_T("DefRotate"), dbLong, 0 );
		symbol_table.CreateField(_T("Type"), dbLong, 0 );

		// Now create the primary key (what an effort!)
		CDaoIndexInfo symbol_index;
		CDaoIndexFieldInfo symbol_index_field;

		symbol_index_field.m_bDescending = FALSE;
		symbol_index_field.m_strName = "SymbolID";

		symbol_index.m_strName = "Symbol_PK";
		symbol_index.m_bPrimary = TRUE;
		symbol_index.m_pFieldInfos = &symbol_index_field;
		symbol_index.m_nFields = 1;
		symbol_index.m_bUnique = FALSE;
		symbol_index.m_bClustered = FALSE;
		symbol_index.m_bIgnoreNulls = FALSE;
		symbol_index.m_bRequired = FALSE;
		symbol_index.m_bForeign = FALSE;
		symbol_index.m_lDistinctCount = 0;
		
		symbol_table.CreateIndex( symbol_index );

		// Now add the table to the database..
		symbol_table.Append();




		/////////////////////// PROPERTY TABLE /////////////////////// 
		
		CDaoTableDef prop_table( &m_database );
		prop_table.Create( _T("Property") );
		prop_table.CreateField(_T("PropertyID"), dbLong, 0, dbAutoIncrField );
		prop_table.CreateField(_T("Name"), dbText, 64 );
		prop_table.CreateField(_T("Value"), dbText, 255 );

		// Now create the primary key (what an effort!)
		CDaoIndexInfo prop_index;
		CDaoIndexFieldInfo prop_index_field;

		prop_index_field.m_bDescending = FALSE;
		prop_index_field.m_strName = _T("PropertyID");

		prop_index.m_strName = "Property_PK";
		prop_index.m_bPrimary = TRUE;
		prop_index.m_pFieldInfos = &prop_index_field;
		prop_index.m_nFields = 1;
		prop_index.m_bUnique = FALSE;
		prop_index.m_bClustered = FALSE;
		prop_index.m_bIgnoreNulls = FALSE;
		prop_index.m_bRequired = FALSE;
		prop_index.m_bForeign = FALSE;
		prop_index.m_lDistinctCount = 0;
		
		prop_table.CreateIndex( prop_index );

		// Now add the table to the database..
		prop_table.Append();
	

	}
	catch( CException *e)
	{
		CString s;
		CString msg;
		e->GetErrorMessage( msg.GetBuffer(256), 256, NULL );
		msg.ReleaseBuffer();
		s.Format(_T("Cannot create library %s.\r\n%s"),
			m_name, msg );
		AfxMessageBox( s );
		e->Delete();
		return false;
	}


	return true;
}

// Is an upgrade required before editing this library?
BOOL CLibraryDb::MustUpgrade()
{
	return FALSE;
}


// Upgrade to the latest version of the library system
BOOL CLibraryDb::Upgrade(CLibraryStore *)
{
	AfxMessageBox( IDS_NOUPGRADENEED );
	return FALSE;
}

