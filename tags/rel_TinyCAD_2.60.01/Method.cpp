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



// This handles the actual drawing of composite objects (symbols)

#include "stdafx.h"
#include "TinyCad.h"
#include "TinyCadView.h"
#include "diag.h"
#include "colour.h"
#include "option.h"

// The Method Class //

// NB a = offset of the bottom left hand corner (used to define position)
//    b = offset of the top right hand corner (used to define size)

// The constructor
CDrawMethod::CDrawMethod(CTinyCadDoc *pDesign,hSYMBOL symbol)
: CDrawingObject( pDesign )
{
  rotate=0;

  m_Symbol = symbol;

  CSymbolRecord *newSymbol = GetSymbolData();
  SymName=newSymbol->name;
  
  a = CPoint(0,0);
  b = GetTr();

  Field[Name]=newSymbol->name;
  Field[Other]="";
  Field[Attrib]="";
  Field[Ref]=newSymbol->reference;
  rotate=3;
  part=0;
  Show[Name]=TRUE;
  Show[Other]=FALSE;
  Show[Ref]=TRUE;

  Pos[Name] = CPoint(0,0);
  Pos[Other] = CPoint(0,0);
  Pos[Ref] = CPoint(0,0);

  fields = newSymbol->fields;

  NewRotation();

  segment=1;

  Undo_Level=0;
  Delete_Level=-1; 
}


// Tag the resources being used by this symbol
void CDrawMethod::TagResources()
{
 	m_pDesign->GetOptions()->TagFont(fPIN);
	m_pDesign->GetOptions()->TagSymbol( m_Symbol );

/*
	CPoint tr;
	CDrawingObject *pointer=ExtractSymbol(tr);

  	while (pointer!=NULL) {
  		pointer->TagResources();
		pointer=pointer->next;
	}
*/
}



// Look for a seach string in the object
CString CDrawMethod::Find(const char *theSearchString)
{
  CString HoldString;

  for (int lp =0;lp!=3;lp++) {
  	HoldString = Field[lp];
  	HoldString.MakeLower();

  	// Now look for the search string in this string
	if (HoldString.Find(theSearchString) != -1)
  		return Field[lp];
  }

  return "";
}



// Rotate this object about a point
void CDrawMethod::Rotate(CPoint p,int ndir)
{
  extern int DoRotate(int,int);

  // Translate this point so the rotational point is the origin
  a = CPoint(a.x-p.x,a.y-p.y);
  b = CPoint(b.x-p.x,b.y-p.y);

  // Perfrom the rotation
  switch (ndir) {
	case 2: // Left
		a = CPoint(a.y,-a.x);
		b = CPoint(b.y,-b.x);
		break;		
	case 3: // Right
		a = CPoint(-a.y,a.x);
		b = CPoint(-b.y,b.x);
		break;
	case 4: // Mirror
		a = CPoint(-a.x,a.y);
		b = CPoint(-b.x,b.y);
		rotate ^= (rotate&4);
		break;
  }

  // Re-translate the points back to the original location
  a = CPoint(a.x+p.x,a.y+p.y);
  b = CPoint(b.x+p.x,b.y+p.y);

  // Set the a and b co-ords to their correct arrangements
  CPoint la = a;

  a = CPoint(max(la.x,b.x),max(la.y,b.y));
  b = CPoint(min(la.x,b.x),min(la.y,b.y));


  rotate = DoRotate(rotate&3,ndir) | (rotate&4);

  NewRotation();
}


// Another Constructor
CDrawMethod::CDrawMethod(CTinyCadDoc *pDesign)
: CDrawingObject( pDesign )
{
	m_Symbol = 0;
  rotate=0;

  SymName="";
  
  a=b=CPoint(0,0);

  Field[Name]="";
  Field[Other]="";
  Field[Ref]="";
  Field[Attrib]="";
  rotate=3;
  part=0;
  Show[Name]=TRUE;
  Show[Other]=FALSE;
  Show[Ref]=TRUE;

  Pos[Name] = CPoint(0,0);
  Pos[Other] = CPoint(0,0);
  Pos[Ref] = CPoint(0,0);


  segment=0;

  Undo_Level=0;
  Delete_Level=-1;
}


CDrawingObject* CDrawMethod::ExtractSymbol( CPoint &tr )
{
  CDrawingObject *pMethod = GetSymbolData()->GetMethod( tr );

  // Calculate the bounding box
  if ((rotate&3)<2)
	b=CPoint(tr.x+a.x,tr.y+a.y);
  else
	b=CPoint(tr.y+a.x,tr.x+a.y);

  return pMethod;
}

BOOL CDrawMethod::IsNoSymbol()
{
	return m_Symbol == 0;
}


// Get the method to draw this symbol
CDesignFileSymbol *CDrawMethod::GetSymbolData()
{
	if (IsNoSymbol())
	{
		// Try looking up this symbol
		GetSymbolByName();
	}
	
	return m_pDesign->GetOptions()->GetSymbol( m_Symbol );
}


// Get the reference value
int CDrawMethod::GetRefVal()
{
  return atoi(GetRef().Mid(GetRef().FindOneOf("0123456789")));
}

// Set the reference value
void CDrawMethod::SetRefVal(int value)
{
  char Buffer[10];

  _itoa(value,Buffer,10);
  Field[Ref] = GetSymbolData()->reference;
  Field[Ref] = Field[Ref].Left(Field[Ref].ReverseFind('?')) + Buffer;
}



BOOL CDrawMethod::CanEdit()
{
  return TRUE;
}

CString CDrawMethod::GetName()
{
  return Field[Name];
}


void CDrawMethod::BeginEdit()
{
  g_EditToolBar.m_MethodEdit.Open(m_pDesign,this);
}

void CDrawMethod::EndEdit()
{
  End();
  g_EditToolBar.m_MethodEdit.Close();
}


CString CDrawMethod::GetField(int which)
{
  CString r = Field[which];

  if (which == Ref && GetSymbolData()->ppp!=1) {
	char Apart[2];
	if (GetSymbolData()->ppp>1) {
		Apart[0]=part + 'A';
		Apart[1]=0;
	} else
		Apart[0]=0;
	r += Apart;
  }

  return r;
}

void CDrawMethod::GetSymbolByName()
{
  CLibraryFileSymbol *theSymbol = static_cast<CTinyCadApp*>(AfxGetApp())->Extract(SymName);
  if (theSymbol != NULL)
  {
	m_Symbol = m_pDesign->GetOptions()->AddSymbol( theSymbol->GetDesignSymbol(m_pDesign) );
  }
  else
  {
	  // Use the no symbol...
	  m_Symbol = 0;
  }
}


void CDrawMethod::OldLoad(CStream &archive)
{
  BYTE Shows;

  archive >> SymName >> Field[Name] >> Field[Other] >> Field[Ref];
  Pos[Name]  = ReadPoint(archive);
  Pos[Other] = ReadPoint(archive);
  Pos[Ref]   = ReadPoint(archive);
  archive >> Shows >> rotate;
  a = ReadPoint(archive);
  archive >> part;

  Field[Attrib] = "";
  Show[Name]  = (Shows & 1)!=0;
  Show[Other] = (Shows & 2)!=0;
  Show[Ref]   = (Shows & 4)!=0;

  GetSymbolByName();  

  segment = 0;

  // Now get the calculation of the bounding box
  CPoint tr;
  ExtractSymbol( tr );
}


void CDrawMethod::OldLoad2(CStream &archive)
{
  BYTE Shows;

  archive >> SymName >> Field[Name] >> Field[Other] >> Field[Ref] >> Field[Attrib];
  Pos[Name]  = ReadPoint(archive);
  Pos[Other] = ReadPoint(archive);
  Pos[Ref]   = ReadPoint(archive);
  archive >> Shows >> rotate;
  a = ReadPoint(archive);
  archive >> part;

  Show[Name]  = (Shows & 1)!=0;
  Show[Other] = (Shows & 2)!=0;
  Show[Ref]   = (Shows & 4)!=0;

  GetSymbolByName();

  segment = 0;

  // Now get the calculation of the bounding box
  CPoint tr;
  ExtractSymbol( tr );
}

void CDrawMethod::Load(CStream &archive)
{
  BYTE Shows;

  archive >> m_Symbol >> Field[Name] >> Field[Other] >> Field[Ref] >> Field[Attrib];
  Pos[Name]  = ReadPoint(archive);
  Pos[Other] = ReadPoint(archive);
  Pos[Ref]   = ReadPoint(archive);
  archive >> Shows >> rotate;
  a = ReadPoint(archive);
  archive >> part;

  // Read in the special other text
  WORD special_text_count;
  archive >> special_text_count;
  for (int i = 0; i < special_text_count; i++)
  {
	  CSymbolField sf;
	  sf.Load( archive );
	  fields.push_back( sf );
  }


  Show[Name]  = (Shows & 1)!=0;
  Show[Other] = (Shows & 2)!=0;
  Show[Ref]   = (Shows & 4)!=0;

  m_Symbol = m_pDesign->GetOptions()->GetNewSymbolNumber( m_Symbol );

  segment = 0;

  // Now get the calculation of the bounding box
  CPoint tr;
  ExtractSymbol( tr );
}


void CDrawMethod::Save(CStream &archive)
{
  BYTE Shows;

  Shows  = Show[Name] ? 1 : 0;
  Shows |= Show[Other] ? 2 : 0;
  Shows |= Show[Ref] ? 4 : 0; 

  archive << m_Symbol << Field[Name] << Field[Other] << Field[Ref] << Field[Attrib];
  WritePoint(archive,Pos[Name]);
  WritePoint(archive,Pos[Other]);
  WritePoint(archive,Pos[Ref]);
  archive << Shows << rotate;
  WritePoint(archive,a);
  archive << part;

  // For future enhancements  
  WORD special_text_count = fields.size();
  archive << special_text_count;
  for (int i = 0; i < special_text_count; i++)
  {
	  fields[i].Save( archive );
  }
}

inline void swap(int &a,int &b) { int sp; sp=a; a=b; b=sp; }

int CDrawMethod::IsInside(int left,int right,int top,int bottom)
{
	CPoint tr;
   CDrawingObject *pointer=ExtractSymbol(tr);


  // Use fast cut-off to see if the bounding box is inside
  // the intersection box
  if ( !((a.x<left && b.x<=left) || (a.x>right && b.x>right)
    || (a.y<top && b.y<=top) || (a.y>bottom && b.y>bottom))) {
	int nl,nr,nt,nb;

	// Translate the intersection box
	nl = left-a.x;
	nr = right-a.x;
	nt = top-a.y;
	nb = bottom-a.y;

	// Rotate the intersection box
	switch (rotate&3) {
		case 1:		// Down
			swap(nt,nb);
			nt=-(nt-tr.y);
			nb=-(nb-tr.y);
			break;
		case 2:		// Left
			swap(nt,nl);
			swap(nb,nr);
			break;
		case 3:		// Right
			swap(nt,nl);
			swap(nb,nr);
			swap(nt,nb);
			nt=-(nt-tr.y);
			nb=-(nb-tr.y);
			break;
	}

	// Mirror the intersection box
	if ((rotate&4)!=0) {
		nl=-nl+tr.x;
		nr=-nr+tr.x;
		swap(nl,nr);
	}

	// Select the correct part for this device
	int hold = m_pDesign->GetPart();
	m_pDesign->SetPart( part );

	// Search each element until one is inside
	while (pointer!=NULL && !pointer->IsInside(nl,nr,nt,nb))
		pointer=pointer->next;


	m_pDesign->SetPart( hold );

	if (pointer!=NULL)
		return TRUE;
  }

  int lp;
  for (lp=0;lp!=3;lp++) {
	if (!Show[lp])
		continue;
	CPoint p1,p2;

	CSize size=m_pDesign->GetTextExtent(GetField(lp),fPIN);

	p1 = CPoint(Pos[lp].x+a.x,Pos[lp].y+a.y);
	p2 = CPoint(Pos[lp].x+a.x+size.cx,Pos[lp].y+a.y-size.cy);
	if ( !( (p1.x<left && p2.x<=left) || (p1.x>right && p2.x>right)
	     || (p1.y<top && p2.y<=top) || (p1.y>bottom && p2.y>bottom)) )
		return TRUE;

  }

  return FALSE;
}


ObjType CDrawMethod::GetType()
{
  return xMethodEx2;
}


void CDrawMethod::Move(CPoint p, CPoint no_snap_p)
{
  // r is the relative displacement to move
  CPoint r;

  r.x=p.x-a.x;
  r.y=p.y-a.y;

  if (r.x!=0 || r.y!=0) {
	Display();
	a=p;
	b=CPoint(b.x+r.x,b.y+r.y);
	segment=0;
	Display();
  }
}


void CDrawMethod::Place(CPoint p, CPoint)
{
  Display();
  Move(p);
  Store();
  Display();	// Write to screen
}



void CDrawMethod::MoveField(int w, CPoint r)
{
  Display();
  Pos[w].x += r.x;
  Pos[w].y += r.y;
  Display();
}


int CDrawMethod::IsInsideField(CPoint p)
{
  int lp;
  CSize size;

  // Convert co-ords to symbol space
  p.x = p.x -a.x;
  p.y = p.y -a.y;

  for (lp=0;lp!=3;lp++) {
	if (!Show[lp])
		continue;

	size=m_pDesign->GetTextExtent(GetField(lp),fPIN);

	if (p.x>=Pos[lp].x && p.x<=Pos[lp].x+size.cx
	 && p.y<=Pos[lp].y && p.y>=Pos[lp].y-size.cy)
		break;
  }

  if (lp<=2)
	return lp;
  else
	return -1;
}


// The space between the method and text
#define SPACING 5


void CDrawMethod::NewRotation()
{
	  CPoint tr;
  ExtractSymbol( tr );

  // Calculate the bounding box
  if ((rotate&3)<2)
	b=CPoint(tr.x+a.x,tr.y+a.y);
  else
	b=CPoint(tr.y+a.x,tr.x+a.y);


  // Calculate the text positions
  int TextHeight=sPIN;
  if (TextHeight <0)
  	TextHeight = -TextHeight;
  int NextY=TextHeight;

  if (Show[Name]) {
	if ((rotate&3)<2) {
		// Display to right hand side if display up or down
		Pos[Name]=CPoint(SPACING,b.y+NextY-a.y);
		NextY+=TextHeight+SPACING/2;
	} else
		// Display above if left or right
		Pos[Name]=CPoint(b.x-a.x,b.y-a.y);
  }
  if (Show[Other]) {
	if ((rotate&3)<2)
		// Display to right hand side if display up or down
		Pos[Other]=CPoint(SPACING,b.y+NextY-a.y);
	else
		// Display below if left or right
		Pos[Other]=CPoint(b.x-a.x,NextY);
	NextY+=TextHeight+SPACING/2;
  }
  if (Show[Ref]) {
	if ((rotate&3)<2)
		// Display to right hand side if display up or down
		Pos[Ref]=CPoint(SPACING,b.y+NextY-a.y);
	else
		// Display below if left or right
		Pos[Ref]=CPoint(b.x-a.x,NextY);
  }
}

CPoint CDrawMethod::GetTr()
{ 
  CPoint tr;
  ExtractSymbol( tr );
	return tr; 
}

void CDrawMethod::Display( BOOL erase )
{
	// Invalidate the symbol
	CRect r( a.x,a.y,b.x,b.y);
	m_pDesign->InvalidateRect( r, erase, 2 );

  // Now invalidate our text
  for (int lp=0;lp!=3;lp++)
  {
	if (Show[lp])
	{
		CSize sz = m_pDesign->GetTextExtent( GetField(lp), fPIN );
		r.left = Pos[lp].x+a.x;
		r.top = Pos[lp].y+a.y;
		r.right = r.left + sz.cx;
		r.bottom = r.top - sz.cy;
		m_pDesign->InvalidateRect( r, erase, 5 );
	}
  }

}


void CDrawMethod::Paint(Context &dc,paint_options options)
{
	  CPoint tr;
  CDrawingObject *pointer=ExtractSymbol( tr );

  // Move the object about by using the grid offset!
  // (that'll have to do until I think of a better method)
  CPoint oldpos= (dc.theTransform).SetTRM(a,tr,rotate);
  // Select the correct part for this device
  int OldPart = m_pDesign->GetPart();
  m_pDesign->SetPart(part);

  paint_options method_options = options == draw_handles ? draw_selected : options;

  while (pointer!=NULL) {
	pointer->Paint(dc,method_options);
	pointer=pointer->next;
  }
  
  (dc.theTransform).EndTRM(oldpos);
  m_pDesign->SetPart(OldPart);



  // Now display the text (if necessary)
  dc.SelectFont(*m_pDesign->GetOptions()->GetFont(fPIN),2);
  switch (options)
  {
  case draw_selected:
  case draw_handles:
	  dc.SetTextColor(cPIN);
	  break;
  case draw_selectable:
  	  dc.SetTextColor(cPIN_CLK);
	  break;
  default:
	  dc.SetTextColor(cPIN);
  }


  

  for (int lp=0;lp!=3;lp++)
  {
	if (Show[lp])
		dc.TextOut(GetField(lp),CPoint(Pos[lp].x+a.x,Pos[lp].y+a.y),options);
  }

}



// Store this method in the drawing
void CDrawMethod::Store()
{
  CDrawMethod *NewObject;

  NewObject = new CDrawMethod(m_pDesign);

  *NewObject=*this;
  
  m_pDesign->Add(NewObject);
}



// Add/remove the next references
void CDrawMethod::AddReference( int min_ref )
{
	// Search the document for a gap in our references
	// 
	CDrawingObject *pointer = m_pDesign->GetHead();
	CString our_reference = GetSymbolData()->reference;
	int last_ref_number = min_ref - 1;
	int last_ppp_number = 0;
	while (pointer!=NULL) 
	{
		if (pointer->GetType() == xMethodEx2) 
		{
			CDrawMethod *pMethod = static_cast<CDrawMethod*>( pointer );

			if (pMethod->GetSymbolData()->reference == our_reference)
			{
				// This is the same reference, so we keep track
				// of the maximum value
				int ref_number = pMethod->GetRefVal();
				int ppp_number = pMethod->GetSubPart();
				if (    last_ref_number < ref_number
					|| (last_ref_number == ref_number && last_ppp_number < ppp_number) )
				{
					last_ref_number = ref_number;
					last_ppp_number = ppp_number;
				}
			}
		}

		pointer = pointer->next;
	}

	// Now we know what the last value was, so set
	// us to the next value
	last_ppp_number ++;
	if (last_ppp_number >= GetSymbolData()->ppp)
	{
		last_ppp_number = 0;
		last_ref_number ++;
	}

	SetRefVal(last_ref_number);
	SetPart(last_ppp_number);
}

void CDrawMethod::RemoveReference()
{
	SetRef(GetSymbolData()->reference);
	SetPart(0);
}

