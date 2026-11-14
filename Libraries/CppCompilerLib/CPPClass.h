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
#include "CPPVtable.h"


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


//How far a class has come.  Each state has all that the ones before it have.
enum ECPPClassState
{
	CPPCS_Declared,			//Named, and perhaps defined: nothing is known of it yet.
	CPPCS_ResolvingBases,	//macBases is being filled in.
	CPPCS_BasesKnown,		//macBases is filled in, without the bases' offsets.
	CPPCS_LayingOut,		//Its bases and fields are being placed.
	CPPCS_Complete,			//Laid out: its size, its fields' and its bases' offsets and its vtable are known.
};


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
	CCPPVtable					mcVtable;			//Its virtual functions: its own vtable and the secondary ones.
	ECPPClassState				meState;
	bool						mbEmpty;			//No fields, no vtable pointer and empty bases.
	int							miAlign;
	bool						mbUnion;
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

	bool			IsUnnamed(void) { return mlliName < 0; }	//Its labels are the unit's own.
	bool			IsComplete(void) { return meState == CPPCS_Complete; }
	bool			IsBeingMade(void) { return (meState == CPPCS_ResolvingBases) || (meState == CPPCS_LayingOut); }
	bool			HasBases(void) { return meState >= CPPCS_ResolvingBases; }

	bool			IsInstanceNamed(CCPPSymbol* pcSymbol);
	SCPPField*		FindField(int64 lliName, int* piOffset);
	SCPPField*		FindFieldSymbol(CCPPSymbol* pcSymbol, int* piOffset);
	CCPPSymbol*		FindMemberSymbol(int64 lliName, ECPPSymbolKind eKind, CCPPClass** ppcClass);
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
