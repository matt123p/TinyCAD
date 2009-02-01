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
#include "startup.h"
#include "revision.h"

// The constructor (Which loads etc., the bitmap)
CDlgStartUpWindow::CDlgStartUpWindow(CWnd *Parent)
{
  // Load a bitmap into a compatible DC
  theBitmap.LoadBitmap(_T("START_BITMAP"));
  BITMAP theBitmapSize;

  // Get it's size etc..
  theBitmap.GetObject(sizeof(theBitmapSize),&theBitmapSize);
  theSize=CSize(theBitmapSize.bmWidth,theBitmapSize.bmHeight);

  // Centre the window in the parent window 
  CRect pRect;
  GetDesktopWindow()->GetWindowRect(pRect);
  CPoint Centre = CPoint(pRect.left+pRect.Width()/2-theSize.cx/2,pRect.top+pRect.Height()/2-theSize.cy/2);
  CRect ClientRect = CRect(Centre.x,Centre.y,Centre.x+theSize.cx,Centre.y+theSize.cy);

  // Create our own Window's class for this window
  CString theClass = AfxRegisterWndClass( 0 ); 

  // Now create the window
  CreateEx( 0, theClass, _T("TinyCAD"),WS_POPUP | WS_VISIBLE, ClientRect.left,ClientRect.top,ClientRect.Width(),ClientRect.Height(), Parent->m_hWnd, NULL);

  // Set a timer to destroy this window in TIME_OUT miliseconds time
  timerID = SetTimer(1,TIME_OUT,NULL);
}


// The destructor (which gets rid of the Bitmap)
CDlgStartUpWindow::~CDlgStartUpWindow()
{

  theBitmap.DeleteObject();
}


BEGIN_MESSAGE_MAP( CDlgStartUpWindow, CWnd )
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// The On Paint Handler (which draws the bitmap into the window)
void CDlgStartUpWindow::OnPaint()
{
  CPaintDC dc(this);
  CBitmap *oldBitmap;

  // Load the bitmap into a DC
  CDC theBitmapDC;
  theBitmapDC.CreateCompatibleDC(&dc);
  oldBitmap = (CBitmap *)(theBitmapDC.SelectObject(&theBitmap));


  // The region to be re-drawn
  CRect rect;
  GetClientRect(rect);

  // Now copy the bitmap into our window
  dc.BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&theBitmapDC,rect.left,rect.top,SRCCOPY);
  
  theBitmapDC.SelectObject(oldBitmap);

  // Now draw the version number text over the top...
  CFont font;
  font.CreateFont(-25,0,0,0,400,FALSE,FALSE,FALSE,
		ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES,
		DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN,_T("Arial"));


  CFont *old_font = dc.SelectObject( &font );
  dc.SetTextAlign(TA_LEFT);
  dc.SetTextColor(RGB(24,24,165));
  dc.SetBkMode( TRANSPARENT );
  dc.TextOut( 265,120, CTinyCadApp::GetVersion() );

  dc.SelectObject( old_font );
}

// When this is called destroy the window
void CDlgStartUpWindow::OnTimer(UINT t)
{
  // Get rid of the timer
  KillTimer(timerID);

  // Get rid of the resources being used by this window
  // theBitmap.DeleteObject();

  // Now delete the window
  DestroyWindow();
}

// Called when the window is destroyed
void CDlgStartUpWindow::OnDestroy()
{
	delete this;
}