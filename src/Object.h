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



#ifndef __OBJECT_H__
#define __OBJECT_H__

// This class is defined in library.h
class CDesignFileSymbol;
class CTinyCadDoc;

#include "DrawingObject.h"
#include "ArcPoint.h"
#include "DragUtils.h"


// Some sizes of parts...

#define CONNECT_SIZE 4



class CDrawRectOutline : public CDrawingObject
{
protected:

	// The constructor
	CDrawRectOutline( CTinyCadDoc *pDesign )
	: CDrawingObject( pDesign )
	{
	}


	// Redraw this object (including the handles)
	virtual void Display( BOOL erase = TRUE );
	virtual void PaintHandles( CContext &dc );

	// Move fields of this object about
	virtual int IsInsideField(CDPoint p);
	virtual void MoveField(int w, CDPoint r);
	virtual int SetCursorEdit( CDPoint p );
};

////// These are the schematic objects //////

class CDrawLine : public CDrawingObject 
{
protected:
	ObjType xtype;

	BOOL	m_use_default_style;
	WORD	m_style;

	BOOL has_placed;
	BOOL is_stuck;
	BOOL is_junction;
	BOOL m_re_edit;

	void ToAngle();
	CDPoint GetStickyPoint( CDPoint no_snap_q );

	CDragUtils	m_drag_utils_a;
	CDragUtils	m_drag_utils_b;

	// This is the network we were assigned on the
	// last network generation
	int		m_network;

public:

	CDrawLine& operator=( const CDrawLine &o )
	{
		m_point_a = o.m_point_a;
		m_point_b = o.m_point_b;
		xtype = o.xtype;
		m_use_default_style = o.m_use_default_style;
		m_style = o.m_style;
		has_placed = o.has_placed;
		is_stuck = o.is_stuck;
		is_junction = o.is_junction;
		m_re_edit = o.m_re_edit;
		m_network = o.m_network;
		return *this;
	}


	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag(ObjType t);

	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual ObjType GetType();
	virtual void Paint(CContext &dc,paint_options options);
	virtual void Display( BOOL erase = TRUE );
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void DblLButtonDown(CDPoint,CDPoint);
	virtual BOOL CanEdit() { return TRUE; }
	virtual int  GetContextMenu();
	virtual void ContextMenu( CDPoint p, UINT id );


	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual CDrawingObject* Store();
	virtual void BeginEdit(BOOL re_edit);
 	virtual void EndEdit();
	virtual void NewOptions();
	virtual CString GetName() const;
	virtual void TagResources();
	virtual int getMenuID();

	virtual void LButtonUp(CDPoint,CDPoint);		// The user has released the left hand button
	virtual BOOL CanControlAspect();				// If the user holds down ctrl, can should we fix the aspect ratio?

	// Move fields of this object about
	int IsInsideField(CDPoint p);
	virtual void MoveField(int w, CDPoint r);
	virtual int SetCursorEdit( CDPoint p );
	virtual void PaintHandles( CContext &dc );

	int getNetwork() { return m_network; }
	void setNetwork( int n ) { m_network = n; }

	// This is used for the construction of this object
	CDrawLine(CTinyCadDoc *pDesign, ObjType NewType);
};




class CDrawPolygon : public CDrawingObject 
{
protected:
	ObjType xtype;

	BOOL m_re_edit;

	WORD Style;
	WORD Fill;

	void ToAngle();
	BOOL IsInsidePolygon( CDPoint a );
	BOOL IsInsideLine( double,double,double,double );
	void FindNearestSegment( CDPoint a, int &line, int &handle );

	arcpointCollection	m_handles;

	pointCollection		m_points;

	void AddPolyBezier( pointCollection &cp, CArcPoint p1, CArcPoint p2 );
	void CalcBoundingRect();
	void FlatternPath();

public:
	virtual double DistanceFromPoint( CDPoint p );

	virtual void Load(CStream &);
	
	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();
	virtual BOOL IsEmpty();

	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual ObjType GetType();
	virtual void Paint(CContext &dc,paint_options options);
	virtual void Display( BOOL erase = TRUE );
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void DblLButtonDown(CDPoint,CDPoint);
	virtual void LButtonUp(CDPoint,CDPoint);
	virtual void Rotate(CDPoint p,int dir);

	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual CDrawingObject* Store();
	virtual void BeginEdit(BOOL re_edit);
 	virtual void EndEdit();
	virtual void NewOptions();
	virtual CString GetName() const;
	virtual void TagResources();
	virtual int getMenuID();
	virtual BOOL CanEdit() { return TRUE; }


	// Move fields of this object about
	virtual int IsInsideField(CDPoint p);
	virtual void MoveField(int w, CDPoint r);
	virtual int SetCursorEdit( CDPoint p );
	virtual void PaintHandles( CContext &dc );

	// For the context menu
	virtual void FinishDrawing( CDPoint p );
	virtual void ContextMenu( CDPoint p, UINT id );
	virtual int  GetContextMenu();

	// This is used for the construction of this object
	CDrawPolygon(CTinyCadDoc *pDesign, ObjType NewType = xLineEx2);
};


class CDrawJunction : public CDrawingObject 
{
protected:
	BOOL is_stuck;
public:
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();
	
	virtual void Load(CStream &);

	virtual double DistanceFromPoint( CDPoint p );

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();


	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual ObjType GetType();		// Get this object's type
	virtual CString GetName() const;
	virtual int getMenuID() { return IDM_TOOLJUNC; }

	// This is used for the construction of this object
	CDrawJunction(CTinyCadDoc *pDesign);
};

class CDrawBusSlash : public CDrawingObject {
	BYTE theDir;
public:
	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();
	virtual void Rotate(CDPoint,int);

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual ObjType GetType();		// Get this object's type
	virtual CString GetName() const;
	virtual int getMenuID();
	virtual void Display( BOOL erase = TRUE );

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst( CActiveNode &a );
	virtual bool GetActive( CActiveNode &a );

	// These are used for the construction of this object
	CDrawBusSlash(CTinyCadDoc *pDesign,int NewDir = 0);
};

class CDrawNoConnect : public CDrawingObject 
{
protected:
	BOOL is_stuck;
public:
	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();


	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual ObjType GetType();		// Get this object's type
	virtual CString GetName() const;
	virtual int getMenuID() { return IDM_TOOLCONNECT; }

	// This is used for the construction of this object
	CDrawNoConnect(CTinyCadDoc *pDesign);
	virtual ~CDrawNoConnect() { }
};


class CEditDlgTextEdit;

class CDrawText : public CDrawRectOutline
{

	friend CEditDlgTextEdit;

	// The font style
	hFONT	FontStyle;

	// The font colour
	COLORREF	FontColour;

	CString str;
	BYTE dir;
	ObjType xtype;

	BOOL is_stuck;
	
	double	original_width;
	double original_box_width;
	double target_box_width;

	void SetScalingWidths();
	void CalcLayout();

public:
	virtual void TagResources();
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag(ObjType t);
	virtual BOOL IsEmpty();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual ObjType GetType();			// Get this object's type
	virtual void Rotate(CDPoint,int);
	virtual BOOL CanEdit();
	virtual void Display( BOOL erase = TRUE );


	CString Find(const TCHAR *);	// Does this string match this text?
	virtual CString GetName() const;
	CString GetValue() { return str; }
	void NewFont(LOGFONT *);	// Change the font
	virtual void NewOptions();

	virtual int getMenuID();
	virtual void MoveField(int w, CDPoint r);

	// These are used for the construction of this object
	CDrawText(CTinyCadDoc *pDesign,ObjType NewType);
	virtual ~CDrawText() { }
};

class CEditDlgLabelEdit;
class CDrawLabel : public CDrawRectOutline
{

public:
	enum label_style
	{
		label_normal,
		label_in,
		label_out,
		label_io
	};


private:
	friend CEditDlgLabelEdit;
	friend CNetList;	//Used to assist debugging the netlists

	// The font style
	hFONT		FontStyle;

	// The font colour
	COLORREF	FontColour;

	// The label style
	label_style	m_Style;

	// The location of the active point
	CDPoint	m_active_point;

	// The additional width of the label style
	double		m_additional_width;

	// The adjustment so that the active point is on-grid..
	CDPoint	m_adjust;

	CString str;
	BYTE	dir;

	BOOL is_stuck;
	BOOL is_junction;
	
	double	original_width;
	double original_box_width;
	double target_box_width;

	void SetScalingWidths();
	void CalcLayout();
	int DoRotate(int olddir,int newdir);

public:
	virtual void TagResources();
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void Load(CStream &,ObjType xtype);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();
	virtual BOOL IsEmpty();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual ObjType GetType();			// Get this object's type
	virtual void Rotate(CDPoint,int);
	virtual BOOL CanEdit();
	virtual void Display( BOOL erase = TRUE );

	virtual void Shift( CDPoint r );					// Move the object by the relative displacement r
	virtual int	 IsInsideField(CDPoint p);

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst( CActiveNode &a );
	virtual bool GetActive( CActiveNode &a );

	virtual CDPoint GetLabelPoint() { return m_active_point; }

	CString Find(const TCHAR *);	// Does this string match this text?
	virtual CString GetName() const;
	CString GetValue() { return str; }
	void NewFont(LOGFONT *);	// Change the font
	virtual void NewOptions();

	virtual int getMenuID();
	virtual void MoveField(int w, CDPoint r);

	// These are used for the construction of this object
	CDrawLabel(CTinyCadDoc *pDesign);
	virtual ~CDrawLabel() { }
};




class CEditDlgPowerEdit;

class CDrawPower : public CDrawingObject {

	friend CEditDlgPowerEdit;
	friend CNetList;

	CDPoint	TextPos; // The position of the text item
	CDPoint	TextEnd;

	BOOL is_stuck;
	BOOL is_junction;

	CString str;
	BYTE dir;	// Direction this item is facing (0=up,1=down,2=left,3=right)
	BYTE which;	// Which type of power item it is

	void CalcLayout();
	int DoRotate(int olddir,int newdir);

public:
	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void TagResources();
	virtual void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual void Rotate(CDPoint,int);
	virtual BOOL CanEdit();
	virtual ObjType GetType();		// Get this object's type
	virtual CString GetName() const;
	CString GetValue() { return str; }
	CString Find(const TCHAR *);

	virtual int getMenuID() { return IDM_TOOLPOWER; }

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst( CActiveNode &a );
	virtual bool GetActive( CActiveNode &a );


	// These are used for the construction of this object
	CDrawPower( CTinyCadDoc *pDesign );
	virtual ~CDrawPower() { }
};


class CEditDlgPinEdit;
class CDrawMethod;

class CDrawPin : public CDrawingObject {

	friend CEditDlgPinEdit;		// The friendly dialog box to edit this pin

	CString m_str;				// The pin name
	CString m_number;			// The pin number
	BYTE m_show;				// Show name or number?
	BYTE m_dir;					// Direction this item is facing (0=up,1=down,2=left,3=right)
	BYTE m_which;				// Which type of power item it is
	BYTE m_elec;				// Which electrical properties this pin has
	BYTE m_part;				// Which subpart the item is in
	BYTE m_converted_power;		// This pin is a power pin converted to normal
	WORD m_length;				// The overall length of the pin
	int	 m_number_pos;			// The relative position of the pin's number
	BOOL m_centre_name;			// Place the name on the pin's shaft rather than the end

	void DetermineLayout( CDPoint &pa,CDPoint &pb,CDPoint &pc,CDPoint &pd,CDPoint &pta,CDPoint &ptb, int &dr, int rotmir );
	void DetermineSize();
	int DoRotate(int olddir,int newdir);

public:
	BOOL IsInvisible();// Is this pin currently visible?
			   		
	BOOL IsPower() { return m_which == 4; }				// Is this pin a power pin?  Note that this will return false for power pins converted to normal pins.
	BOOL IsConvertedPower() { return m_converted_power != 0; }	// Is this pin a power pin converted to normal?
	void ConvertPowerToNormal();

	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void TagResources();
	virtual void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void OldLoad(CStream &);
	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual BOOL CanEdit();
	virtual void Rotate(CDPoint,int);
	virtual ObjType GetType();
	virtual CString GetName() const;
	CString GetNumber()  { return m_number; }
	CString GetPinName() { return m_str; }
	CString Find(const TCHAR *);
	int     GetPart()    { return m_part; }
	int		GetElec()    { return m_elec; }
	static const TCHAR *GetElectricalTypeName(int etype);

	virtual int getMenuID() { return IDM_LIBPIN; }

	// Get the actual position of this pin given the parent method
	CDPoint GetActivePoint(CDrawMethod *parent);

	void SetPart(int NewPart) { m_part= (BYTE) NewPart; }

	BOOL IsHierarchicalPin();

	CDrawPin(CTinyCadDoc *pDesign);				// The constructor
	virtual ~CDrawPin() { }
};

////// An imported advanced metafile //////

class CDrawMetaFile : public CDrawRectOutline
{
	hMETAFILE m_metafile;

public:
	virtual ObjType GetType();
	virtual void TagResources();
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual CString GetName() const;
	void setMetaFile( HENHMETAFILE data );
	bool setImageFile( const TCHAR *filename );
	bool setBitmap( CBitmap &bitmap );
	void determineSize( CDC &dc );

	// This is used for the construction of this object
	CDrawMetaFile(CTinyCadDoc *pDesign);
	virtual ~CDrawMetaFile();

	virtual int getMenuID() { return IDM_TOOLSQUARE; }
};



////// These are the annotation objects //////

class CDrawSquare : public CDrawRectOutline
{
	WORD Style;
	WORD Fill;
	BOOL m_re_edit;
	ObjType m_type;

	double EllipseDistanceFromPoint( CDPoint p, BOOL &IsInside  );

public:
	virtual double DistanceFromPoint( CDPoint p );
	BOOL PointInEllipse( CDPoint p );
	virtual ObjType GetType();
	virtual void TagResources();
	virtual void Paint(CContext &,paint_options);
	virtual CDrawingObject* Store();

	void OldLoad(CStream &,int);
	void OldLoad2(CStream &);
	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag( BOOL isSquare );

	virtual void NewOptions();
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual CString GetName() const;
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual BOOL CanEdit() { return TRUE; }
	virtual void LButtonUp(CDPoint,CDPoint);		// The user has released the left hand button
	virtual void LButtonDown(CDPoint,CDPoint);
	bool IsSquare() { return m_type == xSquareEx3; }


	// This is used for the construction of this object
	CDrawSquare(CTinyCadDoc *pDesign, ObjType type);

	virtual int getMenuID() { return IsSquare() ? IDM_TOOLSQUARE : IDM_TOOLCIRCLE; }
};


class CDrawError : public CDrawingObject {
	int		ErrorNumber;
public:
	virtual ObjType GetType();
	virtual void Paint(CContext &,paint_options);
	virtual CString GetName() const;
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual BOOL CanEdit();
	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual void Display( BOOL erase );
	int GetErrorNumber() { return ErrorNumber; }
	virtual CDrawingObject* Store();

	// This is used for the construction of this object
	CDrawError(CTinyCadDoc *pDesign,CDPoint NewA, int q );

};



// The Tag class

class CDrawTag : public CDrawingObject 
{
	CString 	TagName;

public:
	ObjType 	GetType() 		{ return xTag; }
	virtual BOOL IsInside(double,double,double,double)
	{ 
		return FALSE; 
	}

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();


	CString		GetTagName() 	{ return TagName; }
	virtual void		SetTagName(const TCHAR *NewTagName) { TagName = NewTagName; }

	// This is used for the construction of this object
	CDrawTag(CTinyCadDoc *pDesign, CDPoint NewA,const TCHAR *NewTagName)
		: CDrawingObject(pDesign) 
	{ 
		m_point_a = NewA; 
		TagName = NewTagName; 
	};

	CDrawTag(CTinyCadDoc *pDesign)
		: CDrawingObject(pDesign)
	{ 
		m_point_a=CDPoint(0,0); 
		TagName = ""; 
	}
	virtual ~CDrawTag() { }
};





class CDrawRuler : public CDrawingObject 
{
	BOOL	horiz;
	CDRect	text_rect;

public:
	virtual double DistanceFromPoint( CDPoint p );

	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual BOOL IsInside(double left,double right,double top,double bottom);
	virtual ObjType GetType();
	virtual void Paint(CContext &dc,paint_options options);
	virtual void Display( BOOL erase = TRUE );
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual CDrawingObject* Store();
	virtual CString GetName() const;
	virtual int getMenuID();
	virtual int SetCursorEdit( CDPoint p );

	virtual BOOL IsConstruction();

	// This is used for the construction of this object
	CDrawRuler( CTinyCadDoc *pDesign, BOOL new_horiz );
	virtual ~CDrawRuler();
};





////// These are the block objects //////

class CDrawBlockMove : public CDrawingObject {
	int placed;
	CDPoint OldPos;
public:
	virtual void Paint(CContext &,paint_options);
	CDrawBlockMove(CTinyCadDoc *);
	virtual void EndEdit();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);

	virtual int SetCursor( CDPoint p );
	virtual int getMenuID() { return IDM_EDITMOVE; }
};



class CDrawBlockRotate : public CDrawingObject {
	int placed;

public:
	virtual void Paint(CContext &,paint_options);
	CDrawBlockRotate(CTinyCadDoc *);
	virtual void EndEdit();
	virtual void BeginEdit(BOOL re_edit);
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void ChangeDir(int);

	virtual int SetCursor( CDPoint p );
	virtual int getMenuID() { return IDM_EDITROTATE; }
	virtual CString GetName() const;
};


class CDrawBlockDup : public CDrawingObject {
	int placed;

public:
	virtual void Paint(CContext &,paint_options);
	CDrawBlockDup(CTinyCadDoc *);
	virtual void EndEdit();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);

	virtual int SetCursor( CDPoint p );
	virtual int getMenuID() { return IDM_EDITDUP; }
};



class CDrawBlockDrag : public CDrawingObject 
{
	int placed;
	CDPoint			m_OldPos;
	CDragUtils		m_drag_utils;

public:
	virtual void Paint(CContext &,paint_options);
	CDrawBlockDrag(CTinyCadDoc *pDesign);
	virtual void EndEdit();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual int SetCursor( CDPoint p );
	virtual int getMenuID() { return IDM_EDITDRAG; }
};



class CDrawBlockImport : public CDrawingObject {

public:
	CDrawBlockImport(CTinyCadDoc *pDesign);
	virtual void EndEdit();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void Import();

	virtual int getMenuID() { return IDM_FILEIMPORT; }
};



////// These are the single object editing objects //////


class CDrawEditItem : public CDrawingObject {
	BOOL	InMove;
	BOOL	InSelectByDrag;
	int		EditMethodText;
	CDPoint  LastPos;
	CDPoint  OffsetMove;
	CDPoint  OffsetDrag;

	CDragUtils	m_drag_utils;

	CDrawingObject* GetClosestObject( CDPoint p );

	void ClickSelection( CDPoint p, CDPoint s );

public:
	CDrawEditItem( CTinyCadDoc *pDesign );
	virtual void NewOptions();
	virtual void EndEdit();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);
	virtual void LButtonUp(CDPoint,CDPoint);
	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void Paint(CContext &,paint_options);
	virtual void ReleaseSelection();
	virtual void EndSelection();
	virtual void RButtonUp(CDPoint,CDPoint);
	virtual ObjType GetType() { return xEditItem; }
	virtual int SetCursor( CDPoint p );
	virtual void BeginEdit( BOOL re_edit );
	virtual void Display( BOOL erase = TRUE );
	virtual void ContextMenu( CDPoint p, UINT id );
	virtual void ChangeDir(int NewDir);

	virtual int getMenuID() { return IDM_EDITEDIT; }
};



////// This is the method CDrawingObject for use with library symbols //////

class CEditDlgMethodEdit;




class CDrawMethod : public CDrawingObject {

	friend CEditDlgMethodEdit;		// Both these objects aid the edit of this object

	// Where this symbol data comes from
	hSYMBOL			m_Symbol;

protected:
	BYTE part;				// Which part in the package this is
	BYTE rotate;			// rotation and mirror
	BOOL can_scale;
	BOOL show_power;		// Do we show the power pins?

	struct CField
	{
		SymbolFieldType	m_type;
		CString			m_description;
		CString			m_value;
		BOOL			m_show;
		CDPoint			m_position;
	};

	typedef std::vector<CField>	fieldCollection;
	fieldCollection	m_fields;

	double scaling_x;			// The scaling of this symbol
	double scaling_y;

	typedef std::vector<CDPoint>	activePointsCollection;
	activePointsCollection	m_activePoints; 	//Cached Active Points list


	void NewRotation();		// Calculate new co-ords after a rotation

	void GetSymbolByName(const TCHAR *SymName );

	int DoRotate(int olddir,int newdir);

public:
	void TagResources();

	enum FieldPos { Ref=0, Name, Other };

	CDPoint GetTr();
	CDesignFileSymbol *GetSymbolData();
	virtual BOOL ExtractSymbol( CDPoint &tr, drawingCollection &method );
	virtual BOOL IsInside(double left,double right,double top,double bottom);

	virtual double DistanceFromPoint( CDPoint p );
	virtual int IsInsideField(CDPoint);		// Is a point pointing at one of our fields?
	virtual void MoveField(int,CDPoint);		// Move a field
	virtual int SetCursorEdit( CDPoint p );

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst( CActiveNode &a );
	virtual bool GetActive( CActiveNode &a );
	virtual CDPoint GetFirstStaticPoint();

	void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &,paint_options);
	virtual void PaintHandles( CContext &dc );
	virtual CDrawingObject* Store();

	void OldLoad(CStream &);
	void OldLoad2(CStream &);
	void OldLoad3(CStream &);
	virtual void Load(CStream &);

	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );
	static const TCHAR* GetXMLTag();

	virtual void Move(CDPoint, CDPoint no_snap_p);
	virtual void Shift( CDPoint r );
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual void BeginEdit(BOOL re_edit);
	virtual void EndEdit();
	virtual BOOL CanEdit();
	virtual void Rotate(CDPoint,int);
	virtual BOOL IsNoSymbol();

	void ReplaceSymbol( hSYMBOL old_symbol, hSYMBOL new_symbol, bool keep_old_fields );

	virtual ObjType GetType();		// Get this object's type
	virtual CString GetName() const;
	
	CString GetRef() const { return m_fields[Ref].m_value; }
	CString GetRefSheet( BOOL use_sheet, BOOL use_file_index, int file_name_index, int sheet ) const 
	{ 
		CString s;
		if (file_name_index != 0 && use_file_index)
		{
			s.Format(_T("%d_%s"), file_name_index, m_fields[Ref].m_value );
		}
		else
		{
			s = m_fields[Ref].m_value;
		}

		if (use_sheet)
		{
			CString r;
			r.Format(_T("%d%s"),sheet, s );
			return r;
		}
		else
		{
			return s; 
		}
	}
	int GetFieldIndexByName(CString name, bool caseSensitive=false);
	CString GetField(int);		// Get the string value of a field
	CString GetFieldByName(CString name, bool caseSensitive=false);
		// Get the string value of a field
	CString GetDecoratedField(int);	// Get the string value of a field possibly prepended with the field name and a colon, space
	BOOL IsFieldVisible(SymbolFieldType field_type, CString field_value);		// true if field is currently visible, false if not
	int		GetFieldCount() { return static_cast<int>(m_fields.size()); }
	SymbolFieldType GetFieldType(int i) { return m_fields[i].m_type; }
	CString	GetFieldName(int);
	CString Find(const TCHAR *);
	CDPoint  GetFieldPos(int q) { return CDPoint(m_point_a.x+m_fields[q].m_position.x,m_point_a.y+m_fields[q].m_position.y); }
	BOOL	HasRef() { return _tcschr(GetRef(),'?') == NULL; }

	BOOL GetShowPower() { return show_power; }
	int  GetRefVal();
	int  GetSubPart() { return part; }
	int  GetRotate() { return rotate; }
	virtual void SetRefVal(int value);
	void SetRef(const TCHAR *NewRef) { m_fields[Ref].m_value = NewRef; }
	void SetPart(int NewPart) { part = (BYTE) NewPart; }
	hSYMBOL GetSymbolID() { return m_Symbol; }

	// Add/remove the next references
	void AddReference( int min_ref, bool all_sheets );
	void RemoveReference();

	// The context menu operations
	virtual int GetContextMenu();
	virtual void ContextMenu( CDPoint p, UINT id );

	CDrawMethod(CTinyCadDoc *pDesign,hSYMBOL,int new_rotation);		// The constructors
	CDrawMethod(CTinyCadDoc *pDesign);

	void ScalePoint( CDPoint &r );

	virtual ~CDrawMethod() 
	{
	}

	virtual int getMenuID() { return IDM_TOOLGET; }
};


////// These are non-displayable objects! //////

// The zoom in/out object
class CDrawCentre : public CDrawingObject {
public:
	CDrawCentre( CTinyCadDoc *pDesign )
		: CDrawingObject( pDesign )
	{
	}

	virtual void Display( BOOL erase );
	virtual ObjType GetType();
	virtual void LButtonDown(CDPoint,CDPoint);		// Zoom In
	virtual BOOL RButtonDown(CDPoint,CDPoint);				// Zoom Out

	virtual int SetCursor( CDPoint p ) { return 9; }
	virtual int getMenuID() { return IDM_VIEWCENTRE; }
};

// The zoom in/out object
class CDrawRefPainter : public CDrawingObject 
{
private:
	int m_starting_point;

public:
	CDrawRefPainter( CTinyCadDoc *pDesign, int starting_point )
		: CDrawingObject( pDesign )
	{
		m_starting_point = starting_point;
	}

	virtual void Display( BOOL erase );
	virtual ObjType GetType();
	virtual void LButtonDown(CDPoint,CDPoint);
	virtual BOOL RButtonDown(CDPoint,CDPoint);				// Zoom Out
	virtual void RButtonUp(CDPoint,CDPoint);// The user has released the right hand button
	virtual int SetCursor( CDPoint p ) { return 10; }
	virtual int getMenuID() { return -1; }
};


#endif
