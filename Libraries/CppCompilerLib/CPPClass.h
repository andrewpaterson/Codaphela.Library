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
#ifndef __C_P_P_CLASS_H__
#define __C_P_P_CLASS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPFunction.h"


//A non-static data member.  Offsets include the base class's.
struct SCPPField
{
	CCPPSymbol*		pcSymbol;
	int64			lliName;
	SCPP65Type*		psType;
	int				iOffset;
	int				iAlign;				//Its alignas, 0 for none.
	CCPPSyntax*		pcInitialiser;		//The default member initialiser, or NULL.
	bool			bMutable;			//Declared mutable: not const in a const object.
	int				iBitWidth;			//A bit-field's width, -1 for a field that isn't one.  An unnamed bit-field (int : 3;) has no symbol.
	int				iBitOffset;			//A bit-field's first bit in the unit of its type at iOffset, 0 the lowest.

	bool			IsBitField(void) { return iBitWidth >= 0; }
	bool			IsUnnamedBitField(void) { return (iBitWidth >= 0) && (pcSymbol == NULL); }
};


typedef CArrayTemplate<SCPPField>		CArrayCPPFields;


//A base class and where it is in the derived class.
struct SCPPBase
{
	CCPPClass*	pcClass;
	int			iOffset;
};


typedef CArrayTemplate<SCPPBase>		CArrayCPPBases;


//using Base::f; in a class: Base's functions named f are overloads of the class's own.
struct SCPPUsing
{
	CCPPClass*	pcBase;
	int64		lliName;
};


typedef CArrayTemplate<SCPPUsing>		CArrayCPPUsings;


//A secondary vtable: the one for the polymorphic base subobject at iOffset that does not share the class's
//vtable pointer.  It is laid out like pcOwner's vtable.  A function whose object is not that subobject is
//called through a thunk that adds maiAdjust's entry to this first.
struct SCPPSecondaryVtable
{
	int					iOffset;
	CCPPClass*			pcOwner;
	CArrayCPPFunctions	mapcFunctions;
	CArrayInt			maiAdjust;
};


typedef CArrayTemplate<SCPPSecondaryVtable>	CArrayCPPSecondaryVtables;


//A vtable (CCPP65Generator::VtableEntries): before each part (the class's own, then each secondary vtable) a header of
//two words, the distance from the part's vtable pointer to the object's start and the class's type info, then the
//part's functions.  A vtable pointer points at its part's first function, and the label (_Class__2) at the first part's.
//The type info (_Class__3) is an entry per subobject, its class's type info and its offset, and a zero.
#define CPP65_VTABLE_ENTRY_BYTES		4
#define CPP65_VTABLE_HEADER_ENTRIES		2
#define CPP65_VTABLE_TOP_OFFSET			(-CPP65_VTABLE_HEADER_ENTRIES * CPP65_VTABLE_ENTRY_BYTES)	//From a vtable pointer: the distance to the object's start.
#define CPP65_VTABLE_TYPE_INFO_OFFSET	(-CPP65_VTABLE_ENTRY_BYTES)			//From a vtable pointer: the class's type info.
#define CPP65_TYPE_INFO_ENTRY_BYTES		(2 * CPP65_VTABLE_ENTRY_BYTES)
#define CPP65_TYPE_INFO_OFFSET_OFFSET	CPP65_VTABLE_ENTRY_BYTES				//In a type info entry: the subobject's offset.
#define CPP65_VTABLE_LABEL				"2"
#define CPP65_TYPE_INFO_LABEL			"3"


enum ECPPVtableEntry
{
	CPPVE_Header,		//A part's header: iVtablePointer is where the part's vtable pointer is in the object.
	CPPVE_Function,		//pcFunction, through a thunk that adds iAdjust to this if it isn't 0.
};


struct SCPPVtableEntry
{
	ECPPVtableEntry		eKind;
	int					iVtablePointer;
	CCPPFunction*		pcFunction;
	int					iAdjust;
};


typedef CArrayTemplate<SCPPVtableEntry>	CArrayCPPVtableEntries;


//A class and its layout: the bases in order (an empty base takes no space), then the vtable pointer if no base
//has one, then the fields.  A class is laid out when it is first needed whole.  mpcBase is the first base.
class CCPPClass
{
public:
	CCPPSymbol*					mpcSymbol;
	int64						mlliName;
	SCPP65Type*					mpsType;
	CCPPClass*					mpcBase;
	CArrayCPPBases				macBases;
	CArrayCPPUsings				macUsings;
	CArrayCPPFields				macFields;
	CArrayCPPFunctions			mapcMembers;		//Its functions, in the order they were added.
	CArrayCPPFunctions			mapcVirtuals;		//The vtable.
	int							miVtableOffset;		//-1 if the class has no vtable of its own.
	CArrayCPPSecondaryVtables	macVtables;			//The secondary vtables, written after the vtable.
	bool						mbComplete;
	bool						mbLayingOut;
	bool						mbBases;			//macBases is filled in (their offsets only once it is complete).
	bool						mbEmpty;			//No fields, no vtable pointer and empty bases.
	int							miAlign;
	bool						mbUnion;
	bool						mbVtable;			//The vtable has been written.
	bool						mbTypeInfo;			//The type info (for dynamic_cast) has been written.
	bool						mbConstructors;		//The class declares a constructor.
	CCPPClass*					mpcInheritedBase;	//The base whose constructors are inherited (using Base::Base), or NULL.
	CCPPFunction*				mpcDestructor;
	CCPPSyntaxClassSpecifier*	mpcSpecifier;		//The definition, NULL until it is seen.
	CCPPTemplate*				mpcTemplate;		//The class template this is an instance of.
	CCPPTemplateArguments*		mpcBindings;
	CChars						mszArguments;		//The instance's template arguments, mangled for its label.

	void			Init(CCPPSymbol* pcSymbol, int64 lliName);
	void			Kill(void);

	bool			IsInstanceNamed(CCPPSymbol* pcSymbol);
	SCPPField*		FindField(int64 lliName, int* piOffset);
	SCPPField*		FindFieldSymbol(CCPPSymbol* pcSymbol, int* piOffset);
	size			NumAggregateFields(void);
	bool			NeedsConstructor(void);
	bool			NeedsDestructor(void);
	bool			HasCopyConstructor(void);
	CCPPFunction*	GetCopyConstructor(void);
	bool			IsTriviallyCopyable(void);
	bool			IsPolymorphic(void);
	bool			IsAbstract(void);
	bool			OverridesBase(CCPPFunction* pcFunction);
	int				GetVtablePointerOffset(void);
};


typedef CArrayTemplatePtr<CCPPClass>		CArrayCPPClasses;


#endif // __C_P_P_CLASS_H__
