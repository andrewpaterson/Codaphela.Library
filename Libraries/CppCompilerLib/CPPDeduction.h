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
#ifndef __C_P_P_DEDUCTION_H__
#define __C_P_P_DEDUCTION_H__
#include "BaseLib/ArrayInt64.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPClass.h"
#include "CPPTemplate.h"


class CCPPUnit;
class CCPPDiagnostics;


//One deduction of a function template's arguments from a call's.  [temp.deduct.call]  It is made for the
//template, Deduce is given the call's arguments, and what it found is kept here: for each of the template's
//parameters the type deduced, or NULL if none was, for a parameter that isn't a type its value, and for a
//template template parameter a class that is an instance of the template deduced.  A parameter
//pack's types follow the last parameter's.  The unit binds them (BindArguments), with the defaults for those
//not deduced.
class CCPPDeduction
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.
	CCPPTemplate*		mpcTemplate;
	CArrayCPP65Types	mapsDeduced;
	CArrayInt64			malliDeduced;
	CArrayCPPDeducedPacks	masPacks;
	bool				mbConflict;	//A parameter was deduced as two types (DeduceType).

public:
	void				Init(CCPPUnit* pcUnit, CCPPTemplate* pcTemplate);
	void				Kill(void);

	void				Deduce(CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments);
	void				DeduceConversion(CCPPSyntaxTypeId* pcType, SCPP65Type* psTo);
	CArrayCPP65Types*	GetTypes(void);
	CArrayInt64*		GetValues(void);
	CArrayCPPDeducedPacks*	GetPacks(void);
	bool				HasConflict(void) const;

protected:
	bool				KnownPackSize(CCPPSyntaxParameter* pcParameter, size* puiSize);
	void				DeduceParameters(CCPPSyntaxFunctionSuffix* pcSuffix, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments);
	bool				DeduceParameter(CCPPSyntaxParameter* pcParameter, CArrayCPP65Types* papsArguments, CArraySyntax* papcArguments, size i);
	SCPP65Type*			WithoutQualifiersOf(CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType);
	bool				HasArray(CCPPSyntaxDeclarator* pcDeclarator);
	SCPP65Type*			StripDeclarator(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psType);
	void				DeduceBound(CCPPSyntax* pcSize, int iCount);
	bool				IsForwardingReference(CCPPSyntaxParameter* pcParameter);
	SCPP65Type*			ForwardedType(SCPP65Type* psArgument, CCPPSyntax* pcArgument);
	void				DeduceMemberPointer(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type* psArgument);
	void				DeduceFromInstance(CCPPSyntaxNameComponent* pcComponent, SCPP65Type* psArgument);
	CCPPTemplateArguments*	ArgumentsFor(CCPPSymbol* pcTemplate, SCPP65Type* psType);
	CCPPTemplateArguments*	AliasArguments(CCPPSymbol* pcTemplate, SCPP65Type* psType);
	void				DeduceType(CCPPSymbol* pcSymbol, SCPP65Type* psType);
	void				DeduceValue(CCPPSymbol* pcSymbol, int64 lliValue);
	CCPPClass*			FindInstanceBase(CCPPClass* pcClass, CCPPSymbol* pcTemplate);
	CCPPSymbol*			GetArgumentSymbol(CCPPSyntax* pcArgument);
	bool				DeducePack(CCPPSyntax* pcArgument, CCPPTemplateArguments* pcArguments, size uiFirst);
	void				DeduceFromList(CCPPSyntaxName* pcName, CCPPSyntax* pcList);
	bool				IsPlainType(SCPP65Type* psType);
	bool				NeedsPattern(CCPPSyntaxDeclarator* pcDeclarator);
	void				DeduceByPattern(CCPPSyntaxParameter* pcParameter, SCPP65Type* psArgument);
};


//A pattern whose type can only be worked out once the specialisation's parameters are known, and the argument it
//must then be.
struct SCPPDeferredPattern
{
	CCPPSyntax*	pcPattern;
	SCPP65Type*	psArgument;
};


typedef CArrayTemplate<SCPPDeferredPattern>	CArrayCPPDeferredPatterns;


//One try at matching what is written with a template declaration's parameters against what it must be: a
//specialisation's arguments (X<T*, int>) against those an instance is wanted for, or an alias template's type
//against a type.  If it matches, GetDeduced is what the parameters are, and GetScore how special the pattern is
//(the specialisation with the highest score that matches is the one used).  [temp.class.spec.match],
//[temp.deduct.type]
class CCPPPatternMatch
{
protected:
	CCPPUnit*					mpcUnit;
	CCPPDiagnostics*			mpcDiagnostics;	//Where its errors go: the unit's.
	CCPPTemplateArguments*		mpcDeduced;		//The parameters, deduced as the pattern is matched.  The unit's.
	CArrayCPPDeferredPatterns	masDeferred;
	int							miScore;

public:
	void					Init(CCPPUnit* pcUnit, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	void					Kill(void);

	bool					Match(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments);
	bool					MatchTypeId(CCPPSyntaxTypeId* pcPattern, SCPP65Type* psType);
	bool					MatchParameter(CCPPSyntaxParameter* pcParameter, SCPP65Type* psType);
	CCPPTemplateArguments*	GetDeduced(void) const;
	int						GetScore(void) const;

protected:
	bool					MatchPattern(CArraySyntax* papcPattern, CCPPTemplateArguments* pcArguments);
	bool					MatchArguments(CArraySyntax* papcPattern, CArrayCPPBindings* pasArguments);
	bool					MatchType(CCPPSyntax* pcPattern, SCPP65Type* psArgument);
	bool					MatchDeclarator(CCPPSyntaxDeclarator* pcDeclarator, SCPP65Type** ppsType);
	bool					MatchPointer(CCPPSyntaxPointerOperator* pcOperator, SCPP65Type* psType);
	bool					MatchClass(CCPPSyntaxName* pcName, CCPPClass* pcClass);
	bool					MatchFunction(CCPPSyntaxFunctionSuffix* pcSuffix, SCPP65Type* psType);
	bool					MatchBound(CCPPSyntax* pcSize, int iCount);
	bool					IsDeduced(void);
	bool					MatchDeferred(void);
	bool					Deduce(SCPPBinding* psBinding, SCPP65Type* psType);
	void					FlattenArguments(CCPPTemplateArguments* pcArguments, CArrayCPPBindings* pasArguments);
	bool					PatternParts(CCPPSyntax* pcPattern, CCPPSyntaxDeclSpecifiers** ppcSpecifiers, CCPPSyntaxDeclarator** ppcDeclarator);
	bool					IsPackExpansion(CCPPSyntax* pcPattern);
	SCPPBinding*			ExpandedParameter(CCPPSyntax* pcPattern);
	CCPPSyntaxNamedTypeSpecifier*	NamedType(CCPPSyntaxDeclSpecifiers* pcSpecifiers);
	bool					MatchPack(SCPPBinding* psPack, CArrayCPPBindings* pasArguments, size uiFirst);
};


#endif // __C_P_P_DEDUCTION_H__
