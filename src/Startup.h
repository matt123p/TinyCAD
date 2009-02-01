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

#define	TIME_OUT	2500

class CDlgStartUpWindow : public CWnd
{

private:
	CBitmap	theBitmap;
	CSize	theSize;
	int	timerID;

public:
	CDlgStartUpWindow(CWnd *);		// The constructor
	virtual ~CDlgStartUpWindow();	// The destructor
	
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};

