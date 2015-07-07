/*
 * Project:		TinyCAD program for schematic capture
 *				http://tinycad.sourceforge.net
 * Copyright:	© 1994-2005 Matt Pyne
 * License:		Lesser GNU Public License 2.1 (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */
#ifndef __WINXPTOOLBAR_H__
#define __WINXPTOOLBAR_H__

#pragma once

// CWinXPToolbar

class CWinXPToolbar: public CToolBar
{
	DECLARE_DYNAMIC( CWinXPToolbar)

public:
	CWinXPToolbar();
	virtual ~CWinXPToolbar();

protected:
	CImageList m_ToolbarImages;
	CImageList m_ToolbarImagesDisabled;
	CImageList m_ToolbarImagesHot;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNcPaint();
	void EraseNonClient(void);
	virtual void DoPaint(CDC* pDC);
	void DrawGripper(CDC * pDC, const CRect& rect);
	void AttachToolbarImages(UINT imageWidth, UINT inNormalImageID, UINT inDisabledImageID, UINT inHotImageID);

protected:
	void CWinXPToolbar::MakeToolbarImageList(UINT imageWidth, UINT inBitmapID, CImageList& outImageList);
};

#endif 
