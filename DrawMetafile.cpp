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




// This handles the actual drawing of objects

#include "stdafx.h"
#include <math.h>
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"
#include "revision.h"

#include "ImageMetafile.h"
#include "ImagePNG.h"
#include "ImageJpeg.h"


// The Constructor
CDrawMetaFile::CDrawMetaFile( CTinyCadDoc *pDesign )
: CDrawRectOutline( pDesign )
{
	m_segment=1;
	m_metafile = -1;
	m_point_a = CDPoint(30,30);
	m_point_b = CDPoint(130,130);
}


CDrawMetaFile::~CDrawMetaFile()
{
}

void CDrawMetaFile::setMetaFile( HENHMETAFILE data )
{
	HENHMETAFILE copy = CopyEnhMetaFile( data, NULL );

	ENHMETAHEADER hdr;
	GetEnhMetaFileHeader( data, sizeof(hdr), &hdr );

	double width = fabs(static_cast<double>(hdr.rclFrame.left - hdr.rclFrame.right) ) ;
	double height = fabs(static_cast<double>(hdr.rclFrame.bottom - hdr.rclFrame.top) ) ;

	m_point_b.x = fabs(static_cast<double>(m_point_b.y - m_point_a.y)) * height / width;

	m_metafile = m_pDesign->GetOptions()->AddMetaFile( new CImageMetafile( copy ) );
}

bool CDrawMetaFile::setBitmap( CBitmap &bitmap )
{
	CImagePNG *i = new CImagePNG();
	i->ImportBitmap( bitmap );
	m_metafile = m_pDesign->GetOptions()->AddMetaFile( i );

	return true;
}

bool CDrawMetaFile::setImageFile( const TCHAR *filename )
{
	CImage *i = NULL;

	// Determine the image type
	TCHAR* ext = _tcsrchr(filename,'.');
	if (!ext)
	{
		AfxMessageBox( _T("Unknown image file type") );
		return false;
	}

	// Special case...
	if (_tcsicmp(ext,_T(".bmp")) == 0)
	{
		// Create the png
		CImagePNG *p = new CImagePNG;
		if (!p->ImportBMP( filename ))
		{
			return false;
		}

		m_metafile = m_pDesign->GetOptions()->AddMetaFile( p );
		return true;
	}


	// First read in the file
	FILE *fin = _tfopen(filename,_T("rb"));
	if (!fin)
	{
		AfxMessageBox(_T("Cannot open file"));
		return false;
	}

	// Get the file length
	fseek( fin,0,SEEK_END);
	int bytes = ftell( fin );
	fseek( fin,0, SEEK_SET );

	// Read in the data
	unsigned char *buffer = new unsigned char[ bytes ];
	bytes = fread( buffer,1, bytes, fin );

	// Close the file
	fclose( fin );


	if (_tcsicmp(ext,_T(".png")) == 0)
	{
		i = new CImagePNG;
	}
	else if (_tcsicmp(ext,_T(".jpeg")) == 0
			||	 _tcsicmp(ext,_T(".jpg")) == 0
			||	 _tcsicmp(ext,_T(".jpe")) == 0)
	{
		i = new CImageJpeg;
	}
	else if (_tcsicmp(ext,_T(".emf")) == 0)
	{
		// Create a metafile
		i = new CImageMetafile;
	}
	else
	{
		AfxMessageBox( _T("Unknown image file type"));
		return false;
	}


	// Read in the buffer
	i->SetCompressedData( buffer, bytes );

	
	if (i != NULL)
	{
		m_metafile = m_pDesign->GetOptions()->AddMetaFile( i );
	}

	return i != NULL;
}

void CDrawMetaFile::determineSize( CDC &dc )
{
	CImage *pImage = m_pDesign->GetOptions()->GetImage(m_metafile);
	if (pImage)
	{
		const int max_dim = 500;
		CSize sz = pImage->GetSize( dc );
		if (sz.cx > max_dim)
		{
			sz.cy = (max_dim * sz.cy) / sz.cx;
			sz.cx = max_dim;
		}
		if (sz.cy > max_dim)
		{
			sz.cx = (max_dim * sz.cx) / sz.cy;
			sz.cy = max_dim;
		}

		m_point_b.x = m_point_a.x + sz.cx;
		m_point_b.y = m_point_a.y + sz.cy;
	}
}


ObjType CDrawMetaFile::GetType()
{
  return xMetaFile;
}

CString CDrawMetaFile::GetName() const
{
  return "MetaFile";
}


void CDrawMetaFile::TagResources()
{
	m_pDesign->GetOptions()->TagMetaFile( m_metafile );
}


BOOL CDrawMetaFile::RButtonDown(CDPoint p, CDPoint s)
{
  BOOL r = !m_segment;

  Display();
  m_segment=1;
  
  return r;
}

// Load the rectange from a file
void CDrawMetaFile::Load(CStream& archive )
{
 // Read in the location
  m_point_a = ReadPoint(archive);
  m_point_b = ReadPoint(archive);
  archive >> m_metafile;
  m_metafile = m_pDesign->GetOptions()->GetNewMetaFileNumber(m_metafile);
}

const TCHAR* CDrawMetaFile::GetXMLTag()
{
	return _T("METAFILE");
}

// Load and save to an XML file
void CDrawMetaFile::SaveXML( CXMLWriter &xml )
{
	xml.addTag(GetXMLTag());

	xml.addAttribute( _T("a"), CDPoint(m_point_a) );
	xml.addAttribute( _T("b"), CDPoint(m_point_b) );
	xml.addAttribute( _T("id"), m_metafile );

	xml.closeTag();

}

void CDrawMetaFile::LoadXML( CXMLReader &xml )
{
	xml.getAttribute( _T("a"), m_point_a );
	xml.getAttribute( _T("b"), m_point_b );
	xml.getAttribute( _T("id"), m_metafile );
	m_metafile = m_pDesign->GetOptions()->GetNewMetaFileNumber(m_metafile);
}



void CDrawMetaFile::Paint(CContext &dc,paint_options options)
{

  CDPoint sma = m_point_a;
  CDPoint smb = m_point_b;

  CDRect drect(sma.x,sma.y,smb.x,smb.y);
  CRect rect = dc.GetTransform().Scale( drect );
  int rotmir = dc.GetTransform().GetRotMir();

  CImage *pImage = m_pDesign->GetOptions()->GetImage(m_metafile);
  if (pImage)
  {
	  pImage->Paint( *dc.GetDC(), rect, rotmir );
  }
  else
  {
	  CImage::PaintInvalid( *dc.GetDC(), rect );
  }
}


// Store the rectangle in the drawing
CDrawingObject *CDrawMetaFile::Store()
{
  CDrawMetaFile *NewObject;

  NewObject = new CDrawMetaFile(m_pDesign);
  *NewObject = *this;

  m_pDesign->Add(NewObject);
  
  m_segment=1;

  return NewObject;
}


