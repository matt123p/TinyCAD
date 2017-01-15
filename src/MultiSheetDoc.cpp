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

// MultiSheetDoc.cpp : implementation file
//

#include "stdafx.h"
#include "tinycad.h"
#include "MultiSheetDoc.h"
#include "TinyCadView.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc
IMPLEMENT_DYNCREATE(CMultiSheetDoc, CDocument)

CMultiSheetDoc::CMultiSheetDoc()
{
}

BOOL CMultiSheetDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument()) return FALSE;
	return TRUE;
}

CMultiSheetDoc::~CMultiSheetDoc()
{
}

BEGIN_MESSAGE_MAP(CMultiSheetDoc, CDocument)
//{{AFX_MSG_MAP(CMultiSheetDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc diagnostics

#ifdef _DEBUG
void CMultiSheetDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMultiSheetDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc serialization

void CMultiSheetDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMultiSheetDoc commands


// Force an autosave of the document
void CMultiSheetDoc::AutoSave()
{
}

// Get the file path name during loading or saving
CString CMultiSheetDoc::GetXMLPathName()
{
	return GetPathName();
}

// Is this document editing a library?
bool CMultiSheetDoc::IsLibInUse(CLibraryStore *lib)
{
	return false;
}

// get the number of documents in this multi-doc
int CMultiSheetDoc::GetNumberOfSheets()
{
	return 0;
}

void CMultiSheetDoc::SetActiveSheetIndex(int i)
{
}

int CMultiSheetDoc::GetActiveSheetIndex()
{
	return 0;
}

CString CMultiSheetDoc::GetSheetName(int i)
{
	return "error";
}

CTinyCadDoc* CMultiSheetDoc::GetSheet(int i)
{
	return NULL;
}

// Get the currently active sheet to work with
CTinyCadDoc* CMultiSheetDoc::GetCurrentSheet()
{
	return GetSheet(GetActiveSheetIndex());
}

void CMultiSheetDoc::OnFolderContextMenu()
{
}

void CMultiSheetDoc::SelectSheetView(int i)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS( CTinyCadView )))
		{
			static_cast<CTinyCadView*> (pView)->SelectSheet(i);
			pView->RedrawWindow();
		}
	}
}

void CMultiSheetDoc::SelectERCSheetView(int i, CDrawingObject *ercObject)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS( CTinyCadView )))
		{
			static_cast<CTinyCadView *> (pView)->SelectSheet(i);
			static_cast<CTinyCadView *> (pView)->SetScrollCentre(ercObject->m_point_a); //Around the ERC marker - user is still free to change the focus and switch to a different zoom factor
			pView->RedrawWindow();
		}
	}
}

BOOL CMultiSheetDoc::IsModified()
{
	if (CDocument::IsModified())
	{
		return TRUE;
	}

	for (int i = 0; i < GetNumberOfSheets(); i++)
	{
		if (GetSheet(i)->IsModified())
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMultiSheetDoc::DelayUpdateFrameTitle()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		CFrameWnd* pFrame = pView->GetParentFrame();
		if (pFrame != NULL)
		{
			pFrame->DelayUpdateFrameTitle();
		}
	}
}

//----------------------------------------------
//Override CDocument::DoSave() to avoid Microsoft MFC bug that is exposed in Windows 8.1.  This is an attempt to provide identical functionality
//while avoiding a resource bug that activates the "encountered an improper argument" bug that is missing an exception handler

BOOL CMultiSheetDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(":/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty())
			{
				ASSERT(strExt[0] == '.');
				int iStart = 0;
				newName += strExt.Tokenize(_T(";"), iStart);
			}
		}

//Replace the buggy DoPromptFileName() code with a different technique
		//if (!AfxGetApp()->DoPromptFileName(newName,
		// bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		// OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
		// return FALSE; // don't even attempt to save

		if (!MyDoPromptFileName(newName,bReplace))
			return false;
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == NULL)
		{
			// be sure to delete the file
			TRY
			{
				CFile::Remove(newName);
			}
			CATCH_ALL(e)
			{
//the normal place for the following Macro is in mfc\stdafx.h
//the example code states:  "I bring it here becuase it is used only here and I have no plan to use it anywhere else" (in this file)
#define DELETE_EXCEPTION(e) do { if(e) { e->Delete(); } } while (0)
//
				TRACE(traceAppMsg, 0, "Warning: failed to delete file after failed SaveAs.\n");
				DELETE_EXCEPTION(e);
			}
			END_CATCH_ALL
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	return TRUE;        // success
}

BOOL CMultiSheetDoc::MyDoPromptFileName(CString& fileName, BOOL bReplace)
{
	CFileDialog pDlg(FALSE, NULL, fileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("TinyCAD (*.dsn)|*.dsn|All files (*.*)|*.*||"), AfxGetMainWnd());
	CString title = bReplace ? _T("Save") : _T ("Save As");	//It is the resource identifiers that is the cause of the MFC bug that this function is working around!
	pDlg.m_ofn.lpstrTitle = title;
	if (pDlg.DoModal() != IDOK)
		return FALSE;

	fileName = pDlg.GetPathName();
	return TRUE;
}

//Original DoPromptFileName() from C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\atlmfc\src\mfc\docmgr.cpp
#if 0	//this code is here to serve as a type of developer's comment only
	BOOL CDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
	{
		CFileDialog dlgFile(bOpenFileDialog, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);

		CString title;
		ENSURE(title.LoadString(nIDSTitle));

		dlgFile.m_ofn.Flags |= lFlags;

		CString strFilter;
		CString strDefault;
		if (pTemplate != NULL)
		{
			ASSERT_VALID(pTemplate);
			_AfxAppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
		}
		else
		{
			// do for all doc template
			POSITION pos = m_templateList.GetHeadPosition();
			BOOL bFirst = TRUE;
			while (pos != NULL)
			{
				pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
				_AfxAppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
					bFirst ? &strDefault : NULL);
				bFirst = FALSE;
			}
		}

		// append the "*.*" all files filter
		CString allFilter;
		VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
		strFilter += allFilter;
		strFilter += (TCHAR)'\0';   // next string please
		strFilter += _T("*.*");
		strFilter += (TCHAR)'\0';   // last string
		dlgFile.m_ofn.nMaxCustFilter++;

		dlgFile.m_ofn.lpstrFilter = strFilter;
		dlgFile.m_ofn.lpstrTitle = title;
		dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

		INT_PTR nResult = dlgFile.DoModal();
		fileName.ReleaseBuffer();
		return nResult == IDOK;
	}
#endif	//End of developer's comment