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

#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadView.h"
#include "TinyCadMultiDoc.h"

#include ".\bomgenerator.h"

#define PARTSLIST_PAD	25

CBOMGenerator::CBOMGenerator(void)
{
	m_prefix_import = false;
	m_all_sheets = false;
	m_all_attr = false;
	m_prefix_sheet = false;
	m_hierarchical = false;
	m_prefix_import = false;
	m_MissingRef = false;
}

CBOMGenerator::~CBOMGenerator(void)
{
	fileCollection::iterator fi = m_imports.begin();
	while (fi != m_imports.end())
	{
		delete *fi;
		++ fi;
	}
	m_imports.clear();
}



void CBOMGenerator::GenerateBomForDesign( bool all_sheets, bool all_attr, bool prefix_sheet, bool hierarchical, CMultiSheetDoc *pDesign )
{
	m_attrs.clear();
	m_methods.clear();
	m_MissingRef = false;

	m_all_sheets = all_sheets;
	m_all_attr = all_attr;
	m_prefix_sheet = prefix_sheet;
	m_hierarchical = hierarchical;

	m_attrs.push_back( "Description" );
	m_filename = pDesign->GetPathName();
	m_file_name_index = 0;

	GenerateBomForDesign( 0, pDesign );
}

void CBOMGenerator::GenerateBomForDesign( int level, CMultiSheetDoc *pDesign )
{
   	// Generate a component for every sheet in this design
	bool do_all_sheets = m_all_sheets || (level != 0);
    int sheet = do_all_sheets ? 0 : pDesign->GetActiveSheetIndex();	
	do
	{
		drawingIterator it = pDesign->GetSheet(sheet)->GetDrawingBegin();
		while (it != pDesign->GetSheet(sheet)->GetDrawingEnd()) 
		{
			CDrawingObject *ObjPtr = *it;

			if (ObjPtr->GetType() == xHierarchicalSymbol && m_hierarchical)
			{
				CDrawHierarchicalSymbol *pSymbol = static_cast<CDrawHierarchicalSymbol*>(ObjPtr);

				// Try and stop recursion by limiting the number of imports
				if (level > 100)
				{
					AfxMessageBox( IDS_RECURSION );
					continue;
				}

				// Push back this filename into the list of extra imports
				CImportFile *f = new CImportFile;
				++ m_file_name_index;
				f->m_file_name_index = m_file_name_index;
				if (f->Load( pSymbol->GetFilename() ) )
				{
					m_imports.push_back( f );
					GenerateBomForDesign( level+1, f->m_pDesign );
				}
				else
				{
					delete f;
				}
			}
			else if (ObjPtr->GetType() == xMethodEx3) 
			{
				CDrawMethod *thisMethod = (CDrawMethod*)ObjPtr;

				CString name = thisMethod->GetName();

				if (m_all_attr)
				{
					std::vector<CString>	fields;
					fields.resize( 1 );
					fields[ 0 ] = thisMethod->GetSymbolData()->description;
					for (int i = 2; i < thisMethod->GetFieldCount(); i++)
					{
						// Do we include this field?
						if (thisMethod->GetFieldType(i) == always_hidden)
						{
							continue;
						}

						// Get the field name - is it already in the headings?
						CString field_name = thisMethod->GetFieldName(i);

						for (int j=0;j<m_attrs.size();j++)
						{
							if (field_name.CompareNoCase( m_attrs[j] ) == 0)
							{
								break;
							}
						}
						if (j == m_attrs.size())
						{
							m_attrs.push_back( field_name );
						}

						// Now insert the field name into our vector
						if (fields.size() <= j)
						{
							fields.resize( j + 1 );
						}
						fields[j] = thisMethod->GetField(i);
					}

					// Now generate the names
					for (i = 0; i < fields.size();i++)
					{
						name += ",";
						name += fields[i];
					}

				}

				m_methods.insert( BomSort::BomObject( thisMethod, sheet+1, name ) );
			}

			++ it;
		}

		++sheet;
	}
	while (do_all_sheets && sheet <= pDesign->GetNumberOfSheets());
}


void CBOMGenerator::WriteToFile( FILE *fout, bool csv )
{
	if (csv)
	{
		_ftprintf(fout,_T("Reference,Quantity,Name"));
		for (int i = 0; i < m_attrs.size(); i++)
		{
			_ftprintf(fout,_T(",%s"),m_attrs[i]);
		}
		_ftprintf(fout,_T("\n"));
	}
	else
	{
		_ftprintf(fout,_T("====+  Parts List for %s  +====\n\n"), m_filename);
	}


  bomCollection::iterator itx = m_methods.begin();
  CString LastRef = "";
  CString LastName = "";
  int Xpos = 0, peices=0;
  while (itx != m_methods.end()) 
  {
	CDrawMethod *thisMethod = (*itx).m_pMethod;
	int sheet = (*itx).m_sheet;
	CString name = (*itx).m_name;

	if (name != LastName && LastName != "")
	{
		if (csv)
		{
			_ftprintf(fout,_T("\",%3d,%s\n"),peices,LastName);
		}
		else
		{
			// Pad with spaces
			while (Xpos < PARTSLIST_PAD) 
			{
				_ftprintf(fout,_T(" "));
				Xpos ++;
			}

			// Write the name
			_ftprintf(fout,_T("   %3d   %s\n"),peices,LastName);
			Xpos = 0;
		}

		peices = 0;
	}

	// Do not write out references more than once
	CString ref = thisMethod->GetRefSheet(m_prefix_sheet,m_prefix_import, 0, sheet);
	if (ref != LastRef) 
	{
		if (csv)
		{
			if (peices == 0)
			{
				_ftprintf(fout,_T("\""));
			}
			else
			{
				_ftprintf(fout,_T(","));
			}
		}
		else
		{
			if (Xpos!=PARTSLIST_PAD-1 && Xpos != 0) 
			{
				XInc(fout,Xpos,1);
				_ftprintf(fout,_T(","));
			}
			XInc(fout,Xpos,(ref).GetLength());
		}
		_ftprintf(fout,_T("%s"),ref);
		peices ++;
	}

	if (ref==(thisMethod->GetSymbolData())->reference)
		m_MissingRef = true;

	LastRef = ref;
	LastName = name;

	++ itx;
  }

  if (peices > 0)
  {
		if (csv)
		{
			_ftprintf(fout,_T("\",%3d,%s\n"),peices,LastName);
		}
		else
		{
			// Pad with spaces
			while (Xpos < PARTSLIST_PAD) 
			{
				_ftprintf(fout,_T(" "));
				Xpos ++;
			}

			// Write the name
			_ftprintf(fout,_T("   %3d   %s\n"),peices,LastName);
			Xpos = 0;
		}
  }
}


// Increment the xpos counter
void CBOMGenerator::XInc(FILE *theFile,int &Xpos,int amount)
{
	Xpos += amount;

	if (Xpos >=PARTSLIST_PAD) 
	{
		_ftprintf(theFile,_T("\n"));
		Xpos = amount;
	}
}

