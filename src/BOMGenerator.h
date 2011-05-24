/*
	TinyCAD program for schematic capture
	Copyright 1994/1995/2002-2005 Matt Pyne.

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

#pragma once

#include "Net.h"

class CDrawMethod;

class CBOMGenerator
{
private:

	class BomSort
	{
	public:

		struct BomObject
		{
			CDrawMethod *m_pMethod;
			CString m_RefDes;
			int			m_sheet;
			CString		m_name;

			CString getRefDes() const { return m_RefDes; }

			BomObject( CDrawMethod *pMethod, const CImportFile& ImportContext,
				int sheet, CString name )
			{
				m_pMethod = pMethod;
				m_sheet = sheet;
				m_name = name;
				m_RefDes = CNetList::get_reference_path(pMethod, &ImportContext, CNetList::m_refDirectionForward);
			}
		};

		bool operator()( const BomObject& a, const BomObject &b ) const
		{
			if (a.m_name == b.m_name)
			{
				return a.getRefDes() < b.getRefDes();
					// a.m_pMethod->GetRef() < b.m_pMethod->GetRef();
			}

			return a.m_name < b.m_name;
		}
	};

	// The data we generate
	typedef std::set<BomSort::BomObject,BomSort> bomCollection;
	bomCollection m_methods;
    typedef std::vector<CString> attrHeadings;
    attrHeadings m_attrs;

	// The files we imported (for hierarchical designs)
	typedef std::vector<CImportFile*>	fileCollection;
	fileCollection	m_imports;
	Counter	    m_file_counter;

	// The root filename
	CString		m_filename;

	// Where there any references missing?
	bool		m_MissingRef;

	// Our options for generating this BOM
	bool		m_all_sheets;
	bool		m_all_attr;
	bool		m_prefix_sheet;
	bool		m_hierarchical;
	bool		m_prefix_import;

	void GenerateBomForDesign( int level, int parentPos, const CImportFile& pDesign, int type );
	void XInc(FILE *theFile,int &Xpos,int amount);

public:
	CBOMGenerator(void);
	~CBOMGenerator(void);

	void GenerateBomForDesign( bool all_sheets, bool all_attr, bool prefix_sheet, bool hierarchical, CMultiSheetDoc *pDesign, int type );
	void WriteToFile( FILE *fout, bool csv );
	void WriteToXls( FILE *fout );
	CString EscapeForCSV(const CString& str, char delimiter);

	bool GetMissingRef() { return m_MissingRef; }
};
