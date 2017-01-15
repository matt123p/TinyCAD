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
//#include "LibraryDb.h"
#include "LibrarySQLite.h"
#include "SQLite/CppSQLite3U.h"
#include "DbLibNameSet.h"
#include "StreamMemory.h"
#include "TinyCadSymbolDoc.h"
#include "DbAttributeSet.h"
#include "TinyCadMultiSymbolDoc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLibrarySQLite::CLibrarySQLite()
{

}

CLibrarySQLite::~CLibrarySQLite()
{

}

// Attach this library to a file
BOOL CLibrarySQLite::Attach(const TCHAR *filename)
{
	m_name = filename;

	try
	{
		if (!m_database.mpDB)
		{
			m_database.open(m_name + ".TCLib");
		}

		CString sql("SELECT * FROM [Name] WHERE [Type]=0");
		CppSQLite3Query q = m_database.execQuery(sql);

		while (!q.eof())
		{
			int symbolID = q.getIntField(_T("SymbolID"));

			// Is this a new symbol?
			bool is_new = m_Symbols.find(symbolID) == m_Symbols.end();
			CLibraryStoreNameSet &nwSymbol = m_Symbols[symbolID];

			if (is_new)
			{
				// Yes!
				nwSymbol.Blank();
				nwSymbol.lib = this;
				nwSymbol.FilePos = symbolID;
				nwSymbol.ppp = static_cast<BYTE> (q.getIntField(_T("ppp")));
				nwSymbol.orientation = q.getIntField(_T("DefRotate"));
			}

			CLibraryStoreSymbol r;
			r.name = q.getStringField(_T("Name"));
			r.NameID = q.getIntField(_T("NameID"));
			r.reference = q.getStringField(_T("Reference"));
			r.description = q.getStringField(_T("Description"));
			r.name_type = static_cast<SymbolFieldType> (q.getIntField(_T("ShowName")));
			r.ref_type = static_cast<SymbolFieldType> (q.getIntField(_T("ShowRef")));

			if (is_new)
			{
				recordCollection records;
				records.push_back(r);
				nwSymbol.SetRecords(records);
			}
			else
			{
				nwSymbol.PushBackRecord(r);
			}

			// Now add to our map of m_Symbols
			// m_Symbols[ r.name ] = nwSymbol;

			q.nextRow();
		}
		return TRUE;
	}
	catch (CppSQLite3Exception& e)
	{
		if (m_database.mpDB)
			m_database.close();

		CString s;
		CString msg(e.errorMessage());
		s.Format(_T("Cannot open library %s.\r\n%s"), m_name, msg);
		AfxMessageBox(s);
		return FALSE;
	}
}

// Write a symbol to this library
void CLibrarySQLite::Store(CLibraryStoreNameSet *nwSymbol, CTinyCadMultiSymbolDoc &document)
{
	// Set the busy cursor
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	try
	{
		m_database.execDML(_T("BEGIN TRANSACTION"));
		CppSQLite3Query q;
		CString sql;

		// Write the symbol data into the methods file
		CStreamMemory stream;
		CXMLWriter xml(&stream);
		document.SaveXML(xml);

		CppSQLite3Statement stmt = m_database.compileStatement(_T(
				"INSERT INTO [Symbol] ( [Data] ) VALUES (?)"));
		stmt.bind(1, stream.GetByteArray().GetData(), (int) (stream.GetByteArray().GetSize()));
		stmt.execDML();

		// Clear out all of the old names...
		// This is done after the INSERT to guarantee a new SymbolID.
		if (nwSymbol->FilePos != -1)
		{
			sql.Format(_T("SELECT NameID FROM [Name] WHERE [SymbolID]=%d"), nwSymbol->FilePos);
			CppSQLite3Query q_name = m_database.execQuery(sql);

			while (!q_name.eof())
			{
				sql.Format(_T("DELETE FROM [Attribute] WHERE [NameID]=%d"), q_name.getIntField(_T("NameID")));
				m_database.execQuery(sql);
				q_name.nextRow();
			}

			sql.Format(_T("DELETE FROM [Name] WHERE [SymbolID]=%d"), nwSymbol->FilePos);
			m_database.execQuery(sql);
		}

		// Recover the new symbol id (from the last INSERT statement)
		nwSymbol->FilePos = static_cast<DWORD> (m_database.lastRowId());

		// Do this for each of the names in the symbol set
		for (int i = 0; i < nwSymbol->GetNumRecords(); i++)
		{
			CSymbolRecord &r = nwSymbol->GetRecord(i);

			CppSQLite3Statement stmt = m_database.compileStatement(_T("INSERT INTO [Name]")
			_T(" ([Name], [SymbolID], [Type], [Reference], [ppp], [Description], [ShowName], [ShowRef], [DefRotate])")
			_T(" VALUES (?,?,?,?,?,?,?,?,?)"));
			stmt.bind(1, r.name);
			stmt.bind(2, static_cast<int> (nwSymbol->FilePos));
			stmt.bind(3, 0);
			stmt.bind(4, r.reference);
			stmt.bind(5, nwSymbol->ppp);
			stmt.bind(6, r.description);
			stmt.bind(7, static_cast<int> (r.name_type));
			stmt.bind(8, static_cast<int> (r.ref_type));
			stmt.bind(9, nwSymbol->orientation);
			stmt.execDML();

			// Recover the new name id
			r.NameID = static_cast<DWORD> (m_database.lastRowId());

			//// First delete the old attributes
			//sql.Format( _T("DELETE FROM [Attribute] WHERE [NameID]=%d"), r.NameID );
			//m_database.execQuery( sql );

			// Now write back the attributes...
			std::vector<CSymbolField>::iterator it = r.fields.begin();
			while (it != r.fields.end())
			{
				CppSQLite3Statement stmt = m_database.compileStatement(_T("INSERT INTO [Attribute]")
				_T(" ([NameID], [AttName], [AttValue], [ShowAtt])")
				_T(" VALUES (?,?,?,?)"));
				stmt.bind(1, static_cast<int> (r.NameID));
				stmt.bind(2, (*it).field_name);
				stmt.bind(3, (*it).field_default);
				stmt.bind(4, static_cast<int> ( (*it).field_type));
				stmt.execDML();

				++it;
			}
		}

		m_database.execDML(_T("COMMIT TRANSACTION"));

		// Inform the design it has been saved
		document.SetModifiedFlag(FALSE);

	} catch (CppSQLite3Exception& e)
	{
		m_database.execDML(_T("ROLLBACK TRANSACTION"));
		CString s;
		CString msg(e.errorMessage());
		s.Format(_T("Error storing symbol into library %s.\r\n%s"), m_name, msg);
		AfxMessageBox(s);
	}

	// Re-load the library now it has changed
	ReRead();
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

// Delete a symbol from the database
void CLibrarySQLite::DeleteSymbol(int SymbolID)
{
	CString sql;

	try
	{
		m_database.execDML(_T("BEGIN TRANSACTION"));

		// delete all attributes for this symbol...
		sql.Format(_T("SELECT NameID FROM [Name] WHERE [SymbolID]=%d"), SymbolID);
		CppSQLite3Query q_name = m_database.execQuery(sql);

		while (!q_name.eof())
		{
			sql.Format(_T("DELETE FROM [Attribute] WHERE [NameID]=%d"), q_name.getIntField(_T("NameID")));
			m_database.execQuery(sql);
			q_name.nextRow();
		}

		// now delete the names...
		sql.Format(_T("DELETE FROM [Name] WHERE [SymbolID]=%d"), SymbolID);
		m_database.execQuery(sql);

		// delete the symbol methods...
		sql.Format(_T("DELETE FROM [Symbol] WHERE [SymbolID]=%d"), SymbolID);
		m_database.execQuery(sql);

		m_database.execDML(_T("COMMIT TRANSACTION"));

	} catch (CppSQLite3Exception& e)
	{
		m_database.execDML(_T("ROLLBACK TRANSACTION"));
		CString s;
		CString msg(e.errorMessage());
		s.Format(_T("Error deleting symbol from library %s.\r\n%s"), m_name, msg);
		AfxMessageBox(s);
	}
}

// Delete a symbol from this library
void CLibrarySQLite::DeleteSymbol(CLibraryStoreNameSet &symbol)
{
	// Delete the symbol from this library...
	DeleteSymbol(symbol.FilePos);

	// Now re-read with the changes...
	ReRead();
}

// Do any idle time tasks...
void CLibrarySQLite::OnIdle()
{
}

// Get the Archive to load from
CStream *CLibrarySQLite::GetMethodArchive(CLibraryStoreNameSet *symbol)
{
	// Is this a brand new symbol?
	if (symbol->FilePos == -1)
	{
		return NULL;
	}

	// Now get the attributes
	// Do this for each of the names in the symbol set
	for (int i = 0; i < symbol->GetNumRecords(); i++)
	{
		CSymbolRecord &r = symbol->GetRecord(i);

		if (!r.fields_loaded)
		{
			CString sql;
			sql.Format(_T("SELECT * FROM [Attribute] WHERE [NameID]=%d"), r.NameID);
			CppSQLite3Query q = m_database.execQuery(sql);

			r.fields.erase(r.fields.begin(), r.fields.end());
			while (!q.eof())
			{
				CSymbolField field;
				field.field_name = q.getStringField(_T("AttName"));
				field.field_default = q.getStringField(_T("AttValue"));
				field.field_type = static_cast<SymbolFieldType> (q.getIntField(_T("ShowAtt")));
				r.fields.push_back(field);

				q.nextRow();
			}

			r.fields_loaded = TRUE;
		}
	}

	CString sql;
	sql.Format(_T("SELECT [Data] FROM [Symbol] WHERE [SymbolID]=%d"), symbol->FilePos);
	CppSQLite3Query q = m_database.execQuery(sql);

	if (!q.eof())
	{
		int len;
		const unsigned char* blob = q.getBlobField(_T("Data"), len);
		CStreamMemory* stream = new CStreamMemory;
		stream->Write(blob, len);
		return stream;
	}

	return new CStreamMemory();
}

// Create a new library database
bool CLibrarySQLite::Create(const TCHAR *filename)
{
	//int iErr = SQLITE_OK;

	m_name = filename;

	m_database.close();

	// Now create the database
	try
	{
		m_database.open(m_name + ".TCLib");
	} catch (CppSQLite3Exception& e)
	{
		CString s;
		CString msg(e.errorMessage());
		s.Format(_T("Cannot create library %s.\r\n%s"), m_name, msg);
		AfxMessageBox(s);
		return false;
	}

	// .. and create the default tables
	try
	{
		m_database.execDML(_T("BEGIN TRANSACTION"));

		// .. and create the default tables
		m_database.execDML(
			_T("CREATE TABLE [Name] (")
				_T("[NameID] INTEGER PRIMARY KEY,")
				_T("[Name] TEXT,")
				_T("[SymbolID] INTEGER,")
				_T("[Type] INTEGER,")
				_T("[Reference] TEXT,")
				_T("[ppp] INTEGER,")
				_T("[Description] TEXT,")
				_T("[ShowName] INTEGER,")
				_T("[ShowRef] INTEGER,")
				_T("[DefRotate] INTEGER")
			_T(")"));

		m_database.execDML(
			_T("CREATE TABLE [Symbol] (")
				_T("[SymbolID] INTEGER PRIMARY KEY,")
				_T("[Data] TEXT")
			_T(")"));

		m_database.execDML(
			_T("CREATE TABLE [Attribute] (")
				_T("[AttributeID] INTEGER PRIMARY KEY,")
				_T("[NameID] INTEGER,")
				_T("[AttName] TEXT,")
				_T("[AttValue] TEXT,")
				_T("[ShowAtt] INTEGER")
			_T(")"));

		m_database.execDML( 
			_T("CREATE INDEX [idx_NameID] ON [Attribute] ( [NameID] )"));

		m_database.execDML(_T("COMMIT TRANSACTION"));

	} catch (CppSQLite3Exception& e)
	{
		m_database.execDML(_T("ROLLBACK TRANSACTION"));
		CString s;
		CString msg(e.errorMessage());
		s.Format(_T("Cannot create library %s.\r\n%s"), m_name, msg);
		AfxMessageBox(s);
		return false;
	}

	return true;
}

// Is an upgrade required before editing this library?
BOOL CLibrarySQLite::MustUpgrade()
{
	return FALSE;
}

// Upgrade to the latest version of the library system
BOOL CLibrarySQLite::Upgrade(CLibraryStore *)
{
	AfxMessageBox(IDS_NOUPGRADENEED);
	return FALSE;
}

