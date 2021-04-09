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
	xNoDraw = 1,
	xAnnotation = 2,
	xDesignInformation = 3,
	xIndexInformation = 4,
	xSymbol = 5,
	xError = 6,
	xFont = 7,
	xSymbolEx = 8,
	xLayerOld = 9,
	xLineStyle = 10,
	xFillStyle = 11,
	xLayer = 12,
	xLayerTable = 13,
	xSymbolEx2 = 14,

	xBusName = 15,
	xNoConnect = 16, xJunction = 17, xLabel = 18, xWire = 19, xBus = 20, xMethod = 21,
	xPower = 22, xPin = 23, xBusSlash = 24, xLabelEx = 25, xMethodEx = 26, xMethodEx2 = 27, xPinEx = 28,
	xOrigin,

	xLine = 32, xDash = 33, xText = 34, xCircle = 35, xSquare = 36, xArc = 37, xLineEx = 38, xTextEx = 39,
	xCircleEx = 40, xSquareEx = 41, xArcEx = 42,

	xLineEx2 = 43, xCircleEx2 = 44, xSquareEx2 = 45, xArcEx2 = 46, xMetaFile = 47, 

	xTag  = 48, xRuler = 49, xMethodEx3 = 50, xPolygon = 51,
	xNoteText = 64,

	xEditItem = 128, 
	xDesignInformation2 = 129,
	xMetaFiles = 130,
	xOptions = 131,
	xSymbols = 132, xSymbolMethod = 133, xSymbolDelete = 134,
	xCircleEx3 = 135,
	xSquareEx3 = 136,
	xTextEx2 = 137,
	xLabelEx2 = 138,
	xBusNameEx = 139,
	xImage = 140,
	xHierarchicalSymbol = 141,
	xRefPoint = 142,

	// the value for undefined
	xNULL = 255
};

class CActiveNode;

//////// The is the main object that all objects inherit from //////


class CDrawingObject
{

public:

	CTinyCadDoc *m_pDesign; // Our current design that this object belongs to

	CDPoint m_point_a; // position of this object (in internal units)
	CDPoint m_point_b; // the size of the object, these two points define
	// the bounding box of the object

	char m_segment; // Mode of current edit

	CDrawingObject(CTinyCadDoc *pDesign);
	virtual ~CDrawingObject()
	{
	} // The destructor


	CDPoint ReadPoint(CStream &);

	double AspectRatio() const  {
		return abs((m_point_b.y - m_point_a.y) / (m_point_b.x - m_point_a.x));
	}

	virtual void Load(CStream &); // Load data into this object

	// Load and save to an XML file
	virtual void SaveXML(CXMLWriter &xml);
	virtual void LoadXML(CXMLReader &xml);

	// Extract the netlist/active points from this object
	virtual void GetActiveListFirst(CActiveNode &a);
	virtual bool GetActive(CActiveNode &a);

	virtual double DistanceFromPoint(CDPoint p);
	virtual BOOL IsInside(double left, double right, double top, double bottom); // Is this object inside this rectangle?
	virtual BOOL IsCompletelyInside(double left, double right, double top, double bottom); // Is this object completely inside this rectangle?

	virtual BOOL CanEdit(); // Is this object editable after placed?
	virtual CString GetName() const; // Get the string name of this object
	virtual ObjType GetType(); // Get this object's type

	virtual void TagResources(); // Tag the resources this object is currently using
	virtual CString Find(const TCHAR *); // Find a string in this object

	virtual void BeginEdit(BOOL re_edit); // Called before starting editing this object
	virtual void EndEdit(); // Called before deleting this object from current edit

	virtual void NotifyEdit(int action); // Called during add to undo and undo/redo actions

	virtual void Display(BOOL erase = TRUE);
	virtual void Paint(CContext &, paint_options); // Display this object on the screen
	virtual void PaintHandles(CContext &); // Display this object's handles (for editing)

	virtual void Shift(CDPoint r); // Move the object by the relative displacement r

	virtual void Move(CDPoint, CDPoint no_snap_p); // Follow a mouse movement
	virtual void LButtonDown(CDPoint, CDPoint); // The user has clicked with the left hand button
	virtual void DblLButtonDown(CDPoint, CDPoint); // The user has double clicked with the left hand button
	virtual BOOL RButtonDown(CDPoint, CDPoint); // The user has clicked with the right hand button
	virtual void LButtonUp(CDPoint, CDPoint); // The user has released the left hand button
	virtual void RButtonUp(CDPoint, CDPoint); // The user has released the right hand button

	virtual BOOL CanControlAspect(); // If the user holds down ctrl, can should we fix the aspect ratio?

	// For the context menu
	virtual void ContextMenu(CDPoint p, UINT id);
	virtual void FinishDrawing(CDPoint p);
	virtual int GetContextMenu();

	// Move fields of this object about
	virtual int IsInsideField(CDPoint p);
	virtual void MoveField(int w, CDPoint r);
	virtual int SetCursor(CDPoint p);
	virtual int SetCursorEdit(CDPoint p);

	virtual void NewOptions(); // Indicate to an object being edited that the options have changed


	virtual CDrawingObject* Store(); // Store current object
	virtual void Rotate(CDPoint, int); // Rotate this item about a point

	virtual BOOL IsConstruction();
	virtual BOOL IsEmpty();

	virtual UINT getMenuID()
	{
		return static_cast<UINT>(-1);
	}

	virtual bool operator==(const CDrawingObject &obj) const; // Compare two objects for equality
	virtual bool operator!=(const CDrawingObject &obj) const; // Compare two objects for unequality
};

typedef std::list<CDrawingObject*> drawingCollection;
typedef drawingCollection::iterator drawingIterator;

typedef std::vector<CDPoint> activePointsCollection;
typedef activePointsCollection::iterator activePointsIterator;

// The active node for an object
class CActiveNode
{
public:
	// For public use..
	CDPoint m_a;

	// Only for use by the iterator functions
	activePointsIterator m_iterator;
	activePointsIterator m_iteratorEnd;

	int m_sent;

	CActiveNode()
	{
		m_sent = 0;
	}
};

#endif
