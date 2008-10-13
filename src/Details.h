/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

#pragma once

#include "context.h"
#include "stream.h"
#include "xmlwriter.h"

//=========================================================================
class CDetails
{
	//--
	private:	static const int	M_NBOXWIDTH;

	//--
	private:	static const int	M_NLINEHEIGHT;

	//--
	private:	static const int	M_NRULERHEIGHT;

	//-- How many pixels in each milimetre
	private:	static const int	M_NPIXELSPERMM;

	//-- The default size of the page
	public:		static const CSize	M_SZMAX;
	
	// The size of the page
	public:	CSize			m_szPage;

	//-- Do we display the details box?
	public:	bool			m_bIsVisible;

	//-- Do we show the rulers at the side of the page?
	public:	bool			m_bHasRulers;

	//-- How many divisions in the horizontal ruler?
	public:	int				m_iHorizRulerSize;

	//-- How many divisions in the vertical ruler?
	public:	int				m_iVertRulerSize;


	//-- The date when this design was last edited/saved
	public:	CString			m_szLastChange;

	//-- The title of this design
	public:	CString			m_sTitle;

	//-- The author of this design
	public:	CString			m_sAuthor;

	//-- The revision code of this design
	public:	CString			m_sRevision;

	//-- The document number of this design
	public:	CString			m_sDocNo;

	//-- The organisation which designed this design
	public:	CString			m_sOrg;

	//-- The number of sheets in this design
	public:	CString			m_sSheets;

	//---------------------------------------------------------------------
	public:					CDetails();
	public:					~CDetails();

	//---------------------------------------------------------------------
	private:	void		Init();
	//---------------------------------------------------------------------
	public:		void		Reset();
	//---------------------------------------------------------------------
	public:		void		Read( CStream& oArchive );
	//---------------------------------------------------------------------
	public:		void		ReadEx( CStream& oArchive );
	//---------------------------------------------------------------------
	public:		void		ReadXML( CXMLReader& xml, TransformSnap& oSnap );
	//---------------------------------------------------------------------
	public:		void		WriteXML( CXMLWriter& xml ) const;
	//---------------------------------------------------------------------
	//-- Draw the details box
	private:	void		DisplayBox( CContext & dc, COption& oOption, CString sPathName ) const;
	//---------------------------------------------------------------------
	private:	void		DisplayRulers( CContext & dc, COption& oOption ) const;
	//---------------------------------------------------------------------
	public:		void		Display( CContext & dc, COption& oOption, CString sPathName ) const;
	//---------------------------------------------------------------------
	public:		bool		IsVisible() const;
	//---------------------------------------------------------------------
	public:		bool		HasRulers() const;
	//---------------------------------------------------------------------
	public:		bool		IsPortrait() const;
	//---------------------------------------------------------------------
	public:		CString		GetLastChange() const;
	//---------------------------------------------------------------------
	public:		CString		GetTitle() const;
	//---------------------------------------------------------------------
	public:		CString		GetAuthor() const;
	//---------------------------------------------------------------------
	public:		CString		GetRevision() const;
	//---------------------------------------------------------------------
	public:		CString		GetDocumentNumber() const;
	//---------------------------------------------------------------------
	public:		CString		GetOrganisation() const;
	//---------------------------------------------------------------------
	public:		CString		GetSheets() const;
	//---------------------------------------------------------------------
	public:		CPoint		GetPageBoundsAsPoint() const;
	//---------------------------------------------------------------------
	public:		CDPoint		GetOverlap() const;
	//---------------------------------------------------------------------
	public:		CRect		GetPageBoundsAsRect() const;
	//---------------------------------------------------------------------
 	public:		void		SetVisible( bool bIsVisible );
	//---------------------------------------------------------------------
	public:		void		SetRulers( bool bHasRulers, int v, int h );
	//---------------------------------------------------------------------
	public:		void		SetLastChange( const TCHAR* szLastChange );
	//---------------------------------------------------------------------
	public:		void		SetTitle( CString sTitle );
	//---------------------------------------------------------------------
	public:		void		SetAuthor( CString sAuthor );
	//---------------------------------------------------------------------
	public:		void		SetRevision( CString sRevision );
	//---------------------------------------------------------------------
	public:		void		SetDocumentNumber( CString sDocNo );
	//---------------------------------------------------------------------
	public:		void		SetOrganisation( CString sOrganisation );
	//---------------------------------------------------------------------
	public:		void		SetSheets( CString sSheets );
	//---------------------------------------------------------------------
	//-- Set the page boundries from a CPoint
	public:		void		SetPageBounds( CPoint ptBounds );
	//---------------------------------------------------------------------
	//-- Update the page boundries etc, using a printer device context
	public:		void		SetPageBounds( PRINTDLG& pd );
	//---------------------------------------------------------------------
};
//=========================================================================
