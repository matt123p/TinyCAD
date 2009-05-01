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


#ifndef __NET_H__
#define __NET_H__

#define NetComment	_T(";")		// The symbol used for comments

#include "resource.h"

class CTinyCadMultiDoc;
class CDrawMethod;

#define AttrSpiceProlog		_T("$$SPICE_PROLOG")
#define AttrSpiceEpilog		_T("$$SPICE_EPILOG")
#define AttrSpicePrologPri	_T("$$SPICE_PROLOG_PRIORITY")
#define AttrSpiceEpilogPri	_T("$$SPICE_EPILOG_PRIORITY")
#define AttrSpice			_T("$$SPICE")

class Counter
{
	int i;
public:
	Counter() : i(0) {}

	int next() { return i++; }
	int value() { return i; }
	void reset() { i=0; }
};

// This structure is for a balanced tree of nodes
class CNetListNode
{
private:

	int		m_file_name_index;			// The unqiue file name index this node came from
	CString m_label;					// The name of this node

public:
	int		m_NetList;					// The netlist this node is a member of
	CString m_reference;
	CString m_pin;


	CDPoint	m_a;						// The point where this node is on a design
	int		m_sheet;					// The sheet where this node is in a design
	CDrawingObject *m_parent;			// The object which generated this node
	CDrawMethod *m_pMethod;				// The method which the pin belongs to

	const CString getLabel()
	{
		if (m_label.IsEmpty())
		{
			return "";
		}

		if (m_file_name_index != 0)
		{
			CString s;
			/** This forms a unique net name for this node.  Only a hierarchical symbol will have a file name index that
			 *	is non-zero.  The unique net name presently consists of the net name prefix (hard-coded to "_HN_" for hierarchical net)
			 *	concatenated with the net node number concatenated with the pin name.  In the case of hierarchical symbols, the net
			 *	node number is the hierarchical design instance number (i.e., the first instance is 0, the second instance of the 
			 *	same design is 1, etc.).  This may be a bit problematic because this sequence may not guarantee uniqueness in all
			 *	situations. Only further testing will reveal if this is a problem or not.
			 */
			s.Format(_T("_HN_%d_%s"),
				m_file_name_index, m_label );
			return s;
		}
		else
		{
			return m_label;
		}
	}

	void setLabel( const TCHAR *name )
	{
		m_label = name;
	}

	// The constructor for this node
	CNetListNode(int file_name_index, int sheet, CDrawingObject *parent, CDPoint a )
	{
		m_a = a;
		m_parent = parent;
		m_NetList = -1;
		m_file_name_index = file_name_index;
		m_pMethod = NULL;
		m_sheet = sheet;
	}
	
	inline int getFileNameIndex() const { return m_file_name_index; }

	// The destructor for this node
	~CNetListNode()
	{
	}
};


inline bool operator<( const CPoint &a, const CPoint &b ) 
{
	if (a.x == b.x)
	{
		return a.y < b.y;
	}
	else
	{
		return a.x < b.x;
	}
}

struct CStringLessThanNoCase
{
  bool operator()(const CString& s1, const CString& s2) const
  {
    return s1.CompareNoCase(s2) < 0;
  }
};

typedef std::map<CString,int,CStringLessThanNoCase>	pinCollection;
typedef std::map<CString, CString, CStringLessThanNoCase> pinNameToNumberMap;

// This class is used for spice file generation
class CNetListSymbol
{
	// The file this method came from
	int				m_file_name_index;

public:
	
	// The pointer to the symbol and it's data
	CDrawMethod*	m_pMethod;		

	// The sheet that this method came from
	int				m_sheet;


	// The map of pins to their netlist (or power line)
	pinCollection	m_pins;

	// The map of pin names to their number - used by PSpice and Gandalf Speak netlist macros
	pinNameToNumberMap m_pin_name_map;

	CNetListSymbol( int file_name_index = 0, int sheet = 0, CDrawMethod *pMethod = NULL)
	{
		m_file_name_index = file_name_index;
		m_sheet = sheet;
		m_pMethod = pMethod;

	}
	inline int getFileNameIndex() const { return m_file_name_index; }
	inline void setFileNameIndex(int i) { m_file_name_index = i; }
};

// WARNING: this is nastily implementation dependent 
// These are used in the context of hierarchical designs
// The index refers to the entry in the m_imports
// vector. References/pointers cannot be used since this
// vector resizes during important stages of its construction.
// The alternative is to store a link to the m_imports
// vector itself, as long as that stays put...
// Using this template requires recursive class stuff.
//
// There is a BIG assumption here, namely that once an item
// is added to a vector, it does not move around elsewhere.
template <class T>
class CollectionMemberReference
{
	const std::vector<T>* collection;
	int index;

public:
	inline boolean operator !() const { return collection == NULL; }
	inline boolean isNull() const { return collection == NULL; }
	inline boolean isNotNull() const { return collection != NULL; }
	T* getObject() const { 
		if (collection == NULL)
		{
			return NULL;
		}
		else
			return (*collection)[index];
	}
	const T& getObjectWithDefault(const T& defaultValue) const {
		if (collection == NULL)
			return defaultValue;
		return (*collection)[index];
	}

	CollectionMemberReference() : collection(NULL) {}
	CollectionMemberReference(const std::vector<T>& coll, int i) 
		: collection(&coll), index(i)	{}

	// usual copy constructor and "=" operator semantics
	CollectionMemberReference(const CollectionMemberReference& other)
		: collection(other.collection), index(other.index) {}	

	CollectionMemberReference& operator= (const CollectionMemberReference& other)
	{
		collection = other.collection;
		index = other.index;
		return *this;
	}

};

// The collection of additional files that were imported by this netlist
class CImportFile
{
	int					m_file_name_index;
	CTinyCadMultiDoc*	m_pDesign;

	typedef CollectionMemberReference<CImportFile *> ParentFile_t;
	ParentFile_t m_parent;
	CString             m_RefContext; 
	// Reference context is a path to this instance of the hierarchical symbol.
	// e.g. "/H1/H3/H2" is hierarchical symbol H2 within hierarchical symbol H3
	// within hierarchical symbol H1 within the root.

private:
	void assignContext(const CollectionMemberReference<CImportFile *> parent, 
		CString myReference);

public:
	// use this constructor for root designs
	CImportFile(CTinyCadMultiDoc* pRootDesign) : m_pDesign(pRootDesign)
	{
	}
	// use this constructor for imported files with a parent
	CImportFile(const CollectionMemberReference<CImportFile *> parent, 
		CString myReference) : m_pDesign(NULL)
	{		
		assignContext(parent, myReference);
	}
	~CImportFile();

	BOOL Load( const TCHAR *filename );
	inline void setFileNameIndex(int i) { m_file_name_index = i; }
	inline int getFileNameIndex() const { return m_file_name_index; }
	inline CTinyCadMultiDoc* getDesign() const { return m_pDesign; }
	CString getReferenceContext() const { return m_RefContext; }
};


typedef std::map<CDPoint,int>		nodeCollection;
typedef std::vector<CNetListNode>	nodeVector;
typedef std::map<int,nodeVector>	netCollection;


// Here is the class that holds it all together...
class CNetList
{
public:
	typedef std::vector<CNetList>	linkCollection;

protected:

	CString			m_err_filename;
	FILE*			m_err_file;
	int				m_errors;

	int	m_CurrentNet;
	int GetNewNet();

	int Add(CNetListNode&);

	// The collection of nodes in this netlist
	nodeCollection		m_nodes;


	typedef std::vector<CImportFile*>	fileCollection;
	fileCollection	m_imports;

	// Write the to error file
	void writeError( const _TCHAR *str, ... );

	// Open the error file 
	void createErrorFile( const TCHAR *filename );
	
	// Open the error file in a text view
	void reopenErrorFile( bool force );

	// Create netlist and output as a PCB file (TinyCAD v1.xx)
	void WriteNetListFileTinyCAD( CTinyCadMultiDoc *pDesign, const TCHAR *filename );

	// Create netlist and output as a PCB file (PADS-PCB)
	void WriteNetListFilePADS( CTinyCadMultiDoc *pDesign, const TCHAR *filename );

	// Create netlist and output as a Eagle PCB script
	void WriteNetListFileEagle( CTinyCadMultiDoc *pDesign, const TCHAR *filename );

	// Create netlist and output as a Protel PCB script
	void WriteNetListFileProtel( CTinyCadMultiDoc *pDesign, const TCHAR *filename );

	// Perform the work of making a netlist from a single sheet in this design...
	void MakeNetForSheet( fileCollection &imports, int import_index, int sheet, Counter& file_counter);

	// Expand a spice line
	typedef std::map<int,CString> labelCollection;
	CString expand_spice( int file_name_index, int sheet, CNetListSymbol &symbol, labelCollection &labels, CString spice );
	bool eval_spice_macro( int file_name_index, int sheet, CNetListSymbol &symbol, labelCollection &labels, CString &spice_line, CString macro );

	// Get a netlist name from a pin number
	bool get_pin_by_number_or_name( CNetListSymbol &symbol, labelCollection &labels, CString pin, int &nodes, CString &r, int &net );
	bool get_pin_by_number( CNetListSymbol &symbol, labelCollection &labels, CString pin, int &nodes, CString &r, int &net );

	// Get a attribute value from an attribute name
	bool get_attr( int file_name_index, int sheet, CNetListSymbol &symbol, CString attr, CString &r );

	// Get a hierarchical reference path from a symbol.
	CString get_reference_path( const CDrawMethod* psymbol, const CImportFile *pcontext, bool forward, TCHAR separator=_T('_'));

	// Clear out the imports etc..
	void clear();

public:

	netCollection		m_nets;
	BOOL				m_prefix_references;
	BOOL				m_follow_imports;
	BOOL				m_prefix_import;


	CNetList();
	virtual ~CNetList();


	// Perform the work of making a netlist from this design...
	void MakeNet( CTinyCadMultiDoc *pDesign );

	// Link together several netlists
	void Link( linkCollection& nets );

	// Tell all of the wires what network they are associated with
	void WriteWires();
	
	// Create netlist and output as a PCB file
	void WriteNetListFile( int type, CTinyCadMultiDoc *pDesign, const TCHAR *filename );

	// Create netlist and output as a SPICE file
	void WriteSpiceFile( CTinyCadMultiDoc *pDesign, const TCHAR *filename );
};




// The structure which defines which errors/warnings are in use
struct ErrorTest {
	int DupRef	: 1;		// Duplicated references 0
	int UnConnect	: 1;		// Unconnected items 1
	int NoConnect	: 1;		// Mode than one item on a no-connect net 2
	int Power	: 1;		// Power connected to power 3
	int OutputPwr	: 1;		// Power connected to an output 4
	int Output	: 1;		// Output connected to an output 5
	int NoOutput	: 1;		// No outputs driving inputs 6
	int UnConnected : 1;		// Unconnected nets 7
};

// The type of nets that exist
enum NetType 
{	
		nUnknown, 			// The net has not been evaluated yet						0
		nPassive,			// The net only contains passive connections				1
		nInput,				// The net only contains inputs								2
		nOutput,			// The net contains an output and some inputs/passives		3
		nBiDir,				// The net contains a bi-dir/inputs/passives/tri-state		4
		nPower,				// The net contains a power item and some inputs/passives	5
		nNoConnect,			// The net contains a no-connect and one other item			6
};



// The dialog for the electrical rules check box
class CDlgERCBox : public CDialog {
	ErrorTest 	theErrorTest;

public:	
 	CDlgERCBox(CWnd *pParentWnd = NULL) : CDialog( IDD_ERCBOX, pParentWnd ) {}
	BOOL OnInitDialog();
	virtual void OnOK();

	void SetErrorTest(ErrorTest NewErrorTest) { theErrorTest = NewErrorTest; }
	ErrorTest GetErrorTest() { return theErrorTest; }
};


#endif
