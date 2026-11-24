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
#ifndef __C_P_P_CLASSES_H__
#define __C_P_P_CLASSES_H__
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPClass.h"
#include "CPPVariable.h"
#include "BaseLib/ArrayChars.h"


class CCPPUnit;
class CCPPDiagnostics;
class CCPPDeclarators;
class CCPPContext;
class CCPP65Types;
struct SCPP65Type;


//The unit's classes (CCPPUnit::GetClasses): each made once for a symbol (a member class of a class template's
//instance once for each instance, an unnamed one once for its definition), its bases resolved and laid out when it is
//first needed whole, its members declared, its bit-fields, alignments and using-declarations, and what final and
//override say checked.  [class], [class.mem], [class.derived], [class.virtual]
class CCPPClasses
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;
	CCPPDeclarators*	mpcDeclarators;
	CCPPContext*		mpcContext;
	CCPP65Types*		mpcTypes;
	CArrayCPPClasses	mapcClasses;
	bool				mbLayouts;		//Each class laid out is written down (SetLayouts)...
	CArrayChars			maszLayouts;	//...here, a line each (GetLayouts).

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	CArrayCPPClasses*		GetClassList(void);
	void					SetLayouts(bool bLayouts);
	void					GetLayouts(CArrayChars* paszLines);
	CCPPClass*				NewClass(CCPPSymbol* pcSymbol, int64 lliName);
	CCPPClass*				GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcEnclosing = NULL);
	CCPPClass*				GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPClass*				FindClass(int64 lliName, CCPPScope* pcLookup);
	bool					CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					CompleteObjectClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					CompleteMembers(CCPPClass* pcClass);
	void					PlaceMembers(CCPPClass* pcClass, int iPack, int iAlign);
	bool					CheckVirtSpecifiers(CCPPSyntaxClassSpecifier* pcSpecifier);
	bool					HasVirtSpecifier(CCPPSyntaxFunctionSuffix* pcSuffix, int64 lliName);
	int						AlignasAlignment(CArraySyntax* papcSpecifiers, bool bClass = false);
	int						ObjectAlignment(SCPP65Type* psType);
	int						VariableAlignment(CCPPVariable* pcVariable, int* piAlignas);
	CCPPVariable*			FindStatic(CCPPClass* pcClass, int64 lliName);
	bool					IsConstantMember(CCPPVariable* pcVariable);
	bool					IsDefinedBefore(CCPPClass* pcClass, CCPPSyntax* pcNode);
	SCPP65Type*				ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode);

protected:
	bool					AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier, CCPPSyntaxName* pcName);
	bool					AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier);
	bool					LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					AddLayout(CCPPClass* pcClass);
	bool					ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember);
	bool					ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit);
	void					ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing);
	void					InheritConstructors(CCPPClass* pcClass);
	bool					InjectedNameReached(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType, CCPPSyntax* pcNode);
	bool					IsDeclaredAgainInClass(CCPPClass* pcClass, CCPPFunction* pcFunction, size uiMembers);
	bool					CheckOverrides(CCPPClass* pcClass);
	bool					ReturnsCovariant(CCPPFunction* pcOverrider, CCPPFunction* pcOverridden);
};


#endif // __C_P_P_CLASSES_H__

