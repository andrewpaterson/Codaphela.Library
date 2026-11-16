/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_VTABLE_H__
#define __C_P_P_VTABLE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPFunction.h"


class CCPPClass;


//A secondary vtable: the one for the polymorphic base subobject at iOffset that does not share the class's
//vtable pointer.  It is laid out like pcOwner's vtable.  A function whose object is not that subobject is
//called through a thunk that adds maiAdjust's entry to this first.
struct SCPPSecondaryVtable
{
	int					iOffset;
	CCPPClass*			pcOwner;
	CArrayCPPFunctions	mapcFunctions;
	CArrayInt			maiAdjust;
	CArrayInt			maiVirtualOffsets;	//From the subobject to each of pcOwner's virtual bases, in pcOwner's order.
	CCPPClass*			pcVirtualBase;		//The virtual base the subobject is (or is in), where the complete object puts it, or NULL...
	int					iVirtualBaseOffset;	//...and where that is.
};


typedef CArrayTemplate<SCPPSecondaryVtable>	CArrayCPPSecondaryVtables;


//A vtable as it is written (CCPPVtable::GetEntries): before each part (the class's own, then each secondary vtable) a header of
//two words, the distance from the part's vtable pointer to the object's start and the class's type info, then the
//part's functions.  Before the header, the distances from the part's subobject to each virtual base of its class, the
//first nearest the header.  A vtable pointer points at its part's first function, and the label (_Class__2) at the
//first part's.  The type info (_Class__3) is an entry per subobject, its class's type info and its offset, and a zero.
#define CPP65_VTABLE_ENTRY_BYTES		4
#define CPP65_VTABLE_HEADER_ENTRIES		2
#define CPP65_VTABLE_TOP_OFFSET			(-CPP65_VTABLE_HEADER_ENTRIES * CPP65_VTABLE_ENTRY_BYTES)	//From a vtable pointer: the distance to the object's start.
#define CPP65_VTABLE_TYPE_INFO_OFFSET	(-CPP65_VTABLE_ENTRY_BYTES)			//From a vtable pointer: the class's type info.
#define CPP65_VTABLE_VIRTUAL_OFFSET(i)	(CPP65_VTABLE_TOP_OFFSET - (1 + (i)) * CPP65_VTABLE_ENTRY_BYTES)	//From a vtable pointer: the distance to the part's class's virtual base i.
#define CPP65_TYPE_INFO_ENTRY_BYTES		(2 * CPP65_VTABLE_ENTRY_BYTES)
#define CPP65_TYPE_INFO_OFFSET_OFFSET	CPP65_VTABLE_ENTRY_BYTES				//In a type info entry: the subobject's offset.
#define CPP65_VTABLE_LABEL				"2"
#define CPP65_TYPE_INFO_LABEL			"3"
#define CPP65_VTT_LABEL					"4"
#define CPP65_CONSTRUCTION_LABEL		"5"


enum ECPPVtableEntry
{
	CPPVE_Header,		//A part's header: iVtablePointer is where the part's vtable pointer is in the object (iInVirtual in pcVirtualBase).
	CPPVE_Function,		//pcFunction, through a thunk that adds iAdjust to this if it isn't 0.
	CPPVE_Offset,		//A distance to a virtual base, iAdjust, before a part's header.
};


struct SCPPVtableEntry
{
	ECPPVtableEntry		eKind;
	int					iVtablePointer;
	CCPPFunction*		pcFunction;
	int					iAdjust;
	CCPPClass*			pcVirtualBase;	//A header of a part in a virtual base: that base (SCPPSecondaryVtable::pcVirtualBase)...
	int					iInVirtual;		//...and where the part's vtable pointer is in it.
};


typedef CArrayTemplate<SCPPVtableEntry>	CArrayCPPVtableEntries;


//A class's virtual functions.  Its own vtable starts as its first base's: a function of the class that overrides
//one takes its place, and its other virtual functions go on the end.  The vtable pointer is the first base's
//where the class shares it, otherwise the class's own (CCPPClassLayout places it).  Each polymorphic base
//subobject that doesn't share that pointer has a secondary vtable.
//
//A construction vtable (LayoutConstruction) is a class's vtable for one of its objects that is a base subobject of
//another class's, which its constructors and destructor store while they run: its parts are the class's, but its
//virtual bases are where the complete object puts them.
class CCPPVtable
{
public:
	CArrayCPPFunctions			mapcFunctions;		//The class's own vtable.
	int							miPointerOffset;	//Where its pointer is in an object, -1 if the class has no vtable of its own.
	CArrayCPPSecondaryVtables	macSecondaries;		//The secondary vtables, written after the class's own.
	CArrayInt					maiVirtualOffsets;	//From the object to each of the class's virtual bases, before the class's own part.
	bool						mbWritten;			//The unit being generated has written it.
	CCPPClass*					mpcComplete;		//A construction vtable: the complete object's class...
	int							miCompleteOffset;	//...and where the subobject is in it.

	void	Init(void);
	void	Kill(void);

	bool	HasPointer(void) { return miPointerOffset != -1; }
	bool	IsPolymorphic(void) { return (miPointerOffset != -1) || (macSecondaries.NumElements() > 0); }
	int		GetPointerOffset(void);
	bool	HasPure(void);
	bool	IsOverriddenBy(CCPPFunction* pcFunction);
	CCPPFunction*	FindOverridden(CCPPFunction* pcFunction);

	void	LayoutOwn(CCPPClass* pcClass);
	void	LayoutSecondaries(CCPPClass* pcClass);
	void	LayoutConstruction(CCPPClass* pcClass, CCPPClass* pcComplete, int iOffset);
	void	GetEntries(CArrayCPPVtableEntries* pasEntries);
	void	AddEntries(CArrayCPPVtableEntries* pasEntries, CArrayInt* paiVirtualOffsets, int iVtablePointer, SCPPSecondaryVtable* psVirtual, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust);

protected:
	SCPPSecondaryVtable*	AddSecondary(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust);
	void	AddVirtualPart(CCPPClass* pcClass, CCPPClass* pcVirtualBase, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust);
	int		VirtualBaseOffset(CCPPClass* pcClass, CCPPClass* pcVirtualBase);
	void	SetVirtualOffsets(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayInt* paiOffsets);
	static void	AddOverriders(CCPPClass* pcClass, int iOffset, CArrayTemplatePtr<CCPPClass>* papcClasses, CArrayInt* paiOffsets);
};


#endif // __C_P_P_VTABLE_H__
