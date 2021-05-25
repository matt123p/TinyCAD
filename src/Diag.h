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

#ifndef __DIAG_H__
#define __DIAG_H__


//This file, diag.h, is somewhat misnamed and contains mostly minor dialog classes.
//It should more aptly be named dialog.h or DlgMisc.h

#define SIZESTRING 254
#define SIZENOTETEXT 8192

#include "TinyCadDoc.h"

class CDrawingObject;
class CDrawMethod;
class CDrawPin;
class CDrawText;
class CDrawPower;
class CTinyCadView;

// The dialog for getting the name of a new find string
class CDlgFindBox: public CDialog
{

	CString String;

public:
	CDlgFindBox(CWnd *pParentWnd) :
		CDialog(IDD_FIND, pParentWnd)
	{
	}

	CString GetString()
	{
		return String;
	}

	virtual void OnOK();
};

// The dialog for editing the text object
class CDlgPageSizeBox: public CDialog
{

	CPoint Size;
	BOOL PageSizeChanged;
	BOOL PageSetupChanged;

	BOOL SetPrinter(BOOL display_dialogue);

public:
	CDlgPageSizeBox(CWnd *pParentWnd, CPoint CurrentSize) :
		CDialog(IDD_PAGESIZE, pParentWnd)
	{
		Size = CurrentSize;
	}
	CPoint GetSize()
	{
		return Size;
	}

	void OnMakePort();
	void OnMakeLand();
	void OnPrinterSet();
	void OnChange();
	void OnChangeText();

	virtual void OnOK();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};

// The dialog for the error edit
class CDlgERCListBox: public CDialog
{
	int open;
	CListBox *theListBox;
	BOOL stop;
	CMultiSheetDoc *m_pDesign;
	CTinyCadView *m_pView;
public:
	CDlgERCListBox();
	void Open(CMultiSheetDoc *pDesign, CTinyCadView *pView);
	void Close();
	void ReCheck();
	void AddString(CString);
	void SetSelect(int);
	void OnClick();

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonAnnotate();
	afx_msg void OnBnClickedButtonPcbNetlist();
	afx_msg void OnBnClickedButtonSpiceNetlist();
};

////////// The rebar edit dialogs //////////


class CEditDlg: public CDialog
{
public:
	CEditDlg();

	// Dialog Data
	//{{AFX_DATA(CEditDlg)
	enum
	{
		IDD = IDD_ABOUTBOX
	};
	//}}AFX_DATA


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	//}}AFX_VIRTUAL
	void OnCancel() override;

private:
	CDrawingObject *m_pObject;

protected:
	CTinyCadDoc *m_pDesign;

	int opens;

public:
	void Show(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	CDrawingObject *getObject();

	void changeSelected(CDrawingObject *previous, CDrawingObject *pObject);
	void Close();

protected:
	//{{AFX_MSG(CEditDlg)
	// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CEditDlgNoTool: public CEditDlg
{
public:
	void Create();

};

// The dialog for editing the power object
class CEditDlgPowerEdit: public CEditDlg
{
	BOOL stop;
public:
	CEditDlgPowerEdit()
	{
		opens = 0;
	}
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	void Create();
	void ReFocus();

	// Stop Enter closing this dialog
	void OnOK()
	{
	}

	afx_msg void OnChange();

	DECLARE_MESSAGE_MAP()
};

// The dialog for the rotate block
class CEditDlgRotateBox: public CEditDlg
{

public:
	CEditDlgRotateBox()
	{
		opens = 0;
	}
	void Open(CTinyCadDoc *pDesign, CDrawingObject *pObject);
	void Create();

	afx_msg void OnLeft();
	afx_msg void OnRight();
	afx_msg void OnMirror();

	DECLARE_MESSAGE_MAP()
};

#endif
