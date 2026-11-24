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

	void			Init(CCPPSymbol* pcSymbol, SCPP65Type* psType, int iBitWidth = -1);
	bool			IsBitField(void) const;
	bool			IsUnnamedBitField(void) const;
	bool			NeedsValue(void) const;
};


typedef CArrayTemplate<SCPPField>		CArrayCPPFields;


//A base class and where it is in the derived class.
struct SCPPBase
{
	CCPPClass*	pcClass;
	int			iOffset;	//Where it is in the class's own part; not used for a virtual base (CCPPClass::macVirtualBases).
	ECPPAccess	eAccess;	//public, protected or private: what its members are, at most, as the derived class's.
	bool		bVirtual;	//A virtual base: one subobject however many paths reach it, where the complete object puts it.
};


typedef CArrayTemplate<SCPPBase>		CArrayCPPBases;


//A virtual base of a class, direct or not, and where it is in a complete object of the class.
struct SCPPVirtualBase
{
	CCPPClass*	pcClass;
	int			iOffset;
};


typedef CArrayTemplate<SCPPVirtualBase>	CArrayCPPVirtualBases;


//A way from a class to itself or to one of its bases, direct or not (CCPPClass::BasePaths): each class an object of
//the class has in it, once for each way to it, as its bases are written.
struct SCPPBasePath
{
	CCPPClass*	pcClass;
	int			iOffset;		//Where it is in the class; below pcVirtual where the path passes a virtual base.
	CCPPClass*	pcVirtual;		//The last virtual base the path passes, NULL if it passes none.
	bool		bVirtualEdge;	//Its last step is a virtual base specifier.
	bool		bPublic;		//Every step is public.
};


typedef CArrayTemplate<SCPPBasePath>	CArrayCPPBasePaths;


//What IsDerived gives as the offset of a base reached through a virtual base: there is none that is fixed.
#define CPP_VIRTUAL_PATH	(-0x7FFFFFFF)


//A pointer to a data member is the member's offset in its class and this much more: the null pointer to a member is
//then 0, as every null pointer is (so that zeroed memory holds null ones), and it is no member's, the first's too.
//[conv.mem] 1
#define CPP_MEMBER_POINTER_BIAS	1

//A pointer to a member function is the function's address and, above these bits, what the object's address is moved
//by for the function's this: 0 until it is converted to a pointer to a member of a class the function's class is
//further on in.  The null one is 0 too.  [conv.mem] 2
#define CPP_MEMBER_FUNCTION_MOVE_SHIFT	32


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
struct SCPPLambda;

class CCPPClass
{
public:
	CCPPSymbol*					mpcSymbol;
	int64						mlliName;
	SCPP65Type*					mpsType;
	CCPPClass*					mpcBase;
	CArrayCPPBases				macBases;
	CArrayCPPVirtualBases		macVirtualBases;	//Every virtual base, direct or not, once, in the order a complete object makes them.
	CArrayCPPBasePaths			masBasePaths;		//BasePaths' list, kept once the class is complete...
	bool						mbBasePathsKept;	//...which it then is.
	int							miOwnBytes;			//Its own part's size (without its virtual bases): what it takes as a non-virtual base.
	CArrayCPPUsings				macUsings;
	CArrayCPPFields				macFields;
	CArrayCPPFunctions			mapcMembers;		//Its functions, in the order they were added.
	CCPPVtable					mcVtable;			//Its virtual functions: its own vtable and the secondary ones.
	ECPPClassState				meState;
	bool						mbEmpty;			//No fields, no vtable pointer and empty bases.
	int							miAlign;
	int							miAlignas;			//The alignment alignas gives it, its bases or its members (0 for none): more than the frame may give.
	bool						mbUnion;
	bool						mbTypeInfo;			//The type info (for dynamic_cast) has been written.
	bool						mbConstructors;		//The class declares a constructor.
	CCPPClass*					mpcInheritedBase;	//The base whose constructors are inherited (using Base::Base), or NULL.
	CCPPFunction*				mpcDestructor;
	CCPPSyntaxClassSpecifier*	mpcSpecifier;		//The definition, NULL until it is seen.
	CCPPTemplate*				mpcTemplate;		//The class template this is an instance of.
	CCPPClass*					mpcEnclosing;		//A member class of a class template's instance (or of a member class of one): the class it is in.  Each instance has its own.  [temp.mem.class]
	CCPPTemplateArguments*		mpcBindings;
	CChars						mszArguments;		//The instance's template arguments, mangled for its label.
	bool						mbExternInstance;	//extern template class Name<Arguments>; names it: its members defined outside it
													//that aren't inline are another unit's (CCPPUnit::IsInstantiatedElsewhere).
	SCPPLambda*					mpsLambda;			//The lambda this is the closure of, or NULL (CCPPUnit::GetLambda).
	CCPPFunction*				mpcDeleting;		//The deleting destructor of a class whose destructor is virtual: a virtual
													//function the compiler writes that destroys the object and frees it as its
													//own class frees ([expr.delete] 9, 10; [class.dtor] 13), which delete calls.
	bool						mbPlaceholder;		//It stands for a template's type parameter, whose symbol is its: a class with
													//nothing in it, written as the parameter's name (CCPPUnit::PlaceholderClass).

	void			Init(CCPPSymbol* pcSymbol, int64 lliName);
	void			Kill(void);

	bool			IsUnnamed(void) const;	//Its labels are the unit's own.
	bool			IsComplete(void) const;
	bool			IsBeingMade(void) const;
	bool			IsLayingOut(void) const;
	bool			HasBases(void) const;
	bool			HasVirtualBases(void) const;
	SCPPVirtualBase*	GetVirtualBase(CCPPClass* pcBase);
	int				GetVirtualBaseIndex(CCPPClass* pcBase);
	bool			HasBase(CCPPClass* pcBase);
	CArrayCPPBasePaths*	BasePaths(void);

protected:
	void			AddBasePaths(CCPPClass* pcClass, int iOffset, CCPPClass* pcVirtual, bool bVirtualEdge, bool bPublic);

public:

	bool			IsInstanceNamed(CCPPSymbol* pcSymbol);
	SCPPField*		FindField(int64 lliName, int* piOffset, CCPPClass** ppcVirtual = NULL);
	SCPPField*		FindFieldSymbol(CCPPSymbol* pcSymbol, int* piOffset, CCPPClass** ppcVirtual = NULL);
	CCPPClass*		FindFieldClass(SCPPField* psField);
	CCPPSymbol*		FindMemberSymbol(int64 lliName, ECPPSymbolKind eKind, CCPPClass** ppcClass);
	size			NumAggregateFields(void);
	SCPPField*		AggregateField(size uiValue);
	bool			NeedsConstructor(void);
	bool			NeedsDestructor(void);
	bool			HasCopyConstructor(void);
	CCPPFunction*	GetCopier(bool bAssignment, bool bMove);
	CCPPFunction*	GetPartCopier(bool bAssignment, bool bMove);
	bool			HasVirtualDestructor(void);
	bool			IsCopyDeleted(void);
	bool			IsAssignmentDeleted(void);
	bool			IsTriviallyCopyable(void);
	bool			HasFieldNeedingValue(void);
	bool			IsTriviallyAssignable(void);
	bool			IsPolymorphic(void) const;
	bool			IsAbstract(void);
	bool			OverridesBase(CCPPFunction* pcFunction);
	CCPPFunction*	FindOverridden(CCPPFunction* pcFunction);
	int				GetVtablePointerOffset(void);
};


typedef CArrayTemplatePtr<CCPPClass>		CArrayCPPClasses;


#endif // __C_P_P_CLASS_H__
