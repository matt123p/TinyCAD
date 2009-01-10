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



#ifndef __DRAWING_OBJECT_H__
#define __DRAWING_OBJECT_H__

#include "XMLWriter.h"
#include "XMLReader.h"

#include "DRect.h"

class CContext;
class CTinyCadDoc;

enum paint_options
{
	draw_normal,
	draw_selected,
	draw_selectable,
};

#include "Stream.h"


// The different types of object
enum ObjType {
	xEndFile = 0,
	xNoDraw,
	xAnotation,
	xDesignInformation,
	xIndexInformation,
	xSymbol,
	xError,
	xFont,
	xSymbolEx,
	xLayerOld,
	xLineStyle,
	xFillStyle,
	xLayer,
	xLayerTable,
	xSymbolEx2,

	xBusName = 15,
	xNoConnect = 16, xJunction, xLabel, xWire, xBus, xMethod,
	xPower, xPin, xBusSlash, xLabelEx, xMethodEx, xMethodEx2, xPinEx,

	xLine = 32, xDash, xText, xCircle, xSquare, xArc, xLineEx, xTextEx,
	xCircleEx, xSquareEx, xArcEx,

	xLineEx2, xCircleEx2, xSquareEx2, xArcEx2, xMetaFile, 

	xTag  = 48, xRuler, xMethodEx3, xPolygon,

	xEditItem = 128, 
	xDesignInformation2,
	xMetaFiles,
	xOptions,
	xSymbols, xSymbolMethod, xSymbolDelete,
	xCircleEx3,
	xSquareEx3,
	xTextEx2,
	xLabelEx2,
	xBusNameEx,
	xImage,
	xHierarchicalSymbol,
	xRefPoint,

	// the value for undefined
	xNULL = 255
};


class CActiveNode;


//////// The is the main object that all objects inherit from //////


class CDrawingObject 
{

public:

	CTinyCadDoc	*m_pDesign;					// Our current design that this object belongs to

	CDPoint m_point_a;						// position of this object (in internal units)
	CDPoint m_point_b;						// the size of the object, these two points define
											// the bounding box of the object

	char m_segment;							// Mode of current edit

	CDrawingObject( CTinyCadDoc *pDesign );
	virtual ~CDrawingObject() { }			// The destructor


	CDPoint 	ReadPoint(CStream &);

	virtual void Load(CStream &);			// Load data into this object

	// Load and save to an XML file
	virtual void SaveXML( CXMLWriter &xml );
	virtual void LoadXML( CXMLReader &xml );

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst( CActiveNode &a );
	virtual bool GetActive( CActiveNode &a );


	virtual double DistanceFromPoint( CDPoint p );
	virtual BOOL IsInside(double left,double right,double top,double bottom);	// Is this object inside this rectangle?
	virtual BOOL CanEdit();					// Is this object editable after placed?
	virtual CString GetName() const;		// Get the string name of this object
	virtual ObjType GetType();				// Get this object's type

	virtual void TagResources();			// Tag the resources this object is currently using
	virtual CString Find(const TCHAR *);		// Find a string in this object

	virtual void BeginEdit(BOOL re_edit);	// Called before starting editing this object
	virtual void EndEdit();					// Called before deleting this object from current edit

	virtual void Display( BOOL erase = TRUE );
	virtual void Paint(CContext &, paint_options  );	// Display this object on the screen
	virtual void PaintHandles(CContext & );				// Display this object's handles (for editing)
	
	virtual void Shift( CDPoint r );					// Move the object by the relative displacement r

	virtual void Move(CDPoint, CDPoint no_snap_p);	// Follow a mouse movement
	virtual void LButtonDown(CDPoint,CDPoint);		// The user has clicked with the left hand button
	virtual void DblLButtonDown(CDPoint,CDPoint);		// The user has double clicked with the left hand button
	virtual BOOL RButtonDown(CDPoint,CDPoint);		// The user has clicked with the right hand button
	virtual void LButtonUp(CDPoint);					// The user has released the left hand button
	virtual void RButtonUp(CDPoint,CDPoint);			// The user has released the right hand button

	virtual BOOL CanControlAspect();				// If the user holds down ctrl, can should we fix the aspect ratio?

	// For the context menu
	virtual void ContextMenu( CDPoint p, UINT id );
	virtual void FinishDrawing( CDPoint p );
	virtual int  GetContextMenu();

	
	// Move fields of this object about
	virtual int IsInsideField(CDPoint p);
	virtual void MoveField(int w, CDPoint r);
	virtual int SetCursor( CDPoint p );
	virtual int SetCursorEdit( CDPoint p );

	virtual	void NewOptions();				// Indicate to an object being edited that the options have changed


	virtual CDrawingObject* Store();		// Store current object
	virtual void Rotate(CDPoint,int);		// Rotate this item about a point

	virtual BOOL IsConstruction();

	virtual int getMenuID() { return -1; }

	virtual bool operator==( const CDrawingObject &obj ) const; // Compare two objects for equality
	virtual bool operator!=( const CDrawingObject &obj ) const; // Compare two objects for unequality
};


typedef std::list<CDrawingObject*> drawingCollection;
typedef drawingCollection::iterator drawingIterator;


// The active node for an object
class CActiveNode
{
public:
	// For public use..
	CString	m_label;
	CDPoint	m_a;


	// Only for use by the iterator functions
	drawingIterator		m_iterator;
	drawingCollection	m_method;
	int					m_sent;

	CActiveNode()
	{
		m_sent = 0;
	}
};



#endif
