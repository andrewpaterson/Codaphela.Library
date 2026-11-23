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
#ifndef __C_P_P_INSTANTIATOR_H__
#define __C_P_P_INSTANTIATOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"
#include "CPPTemplate.h"


class CCPPUnit;
class CCPPClass;
class CCPPFunction;
class CCPPSymbol;
class CCPPVariable;
struct SCPP65Type;
struct SCPPBinding;


//The unit's templates (CCPPUnit::GetInstantiator): each template declaration read, a class template's, a function
//template's and a variable template's instances made for their arguments, their explicit and partial
//specialisations found, the definitions written outside a class template given to each instance, and explicit
//instantiations.  [temp]
class CCPPInstantiator
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					IsStaticMemberDefinition(CCPPSyntaxSimpleDeclaration* pcSimple);
	bool					DefineInstanceData(void);
	bool					DefineInstanceStatics(bool* pbResult);
	CCPPSyntax*				InstanceStaticInitialiser(CCPPClass* pcClass, CCPPSymbol* pcSymbol, CCPPTemplateArguments** ppcBindings);
	bool					DefineVariableInstances(bool* pbResult);
	CCPPSyntaxName*			VariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple);
	CCPPSyntaxName*			MemberVariableTemplateName(CCPPSyntaxSimpleDeclaration* pcSimple);
	CCPPTemplate*			VariableTemplate(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcClass);
	CCPPTemplate*			ClassVariableTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass);
	void					DefineMemberVariableTemplate(CCPPTemplate* pcTemplate);
	void					SpecialiseMemberVariableTemplate(CCPPTemplate* pcPrimary, CCPPSyntaxTemplateDeclaration* pcOutside);
	CCPPSyntaxInitDeclarator*	VariableDeclarator(CCPPVariable* pcVariable);
	CCPPVariable*			VariableInstance(CCPPSyntaxName* pcName, CCPPSyntax* pcNode);
	CCPPVariable*			ClassVariableInstance(CCPPSymbol* pcSymbol, CCPPSyntaxNameComponent* pcComponent, CCPPClass* pcClass, CCPPSyntax* pcNode);
	bool					IsInstanceStaticDefined(CCPPClass* pcClass, CCPPSyntaxName* pcName);
	CCPPVariable*			OwnStatic(CCPPClass* pcClass, CCPPSymbol* pcSymbol);
	bool					TemplateDeclaration(CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	void					DeclareFunctionTemplate(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxSimpleDeclaration* pcSimple);
	void					DefineFunctionTemplate(CCPPTemplate* pcTemplate, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxFunctionDefinition* pcDefinition);
	void					NoDefaultArguments(CCPPSyntaxDeclarator* pcDeclarator, const char* szError);
	void					DeclaredDefaultArguments(CCPPSyntaxTemplateDeclaration* pcDeclared, CCPPSyntaxDeclarator* pcDeclaredDeclarator, CCPPSyntaxTemplateDeclaration* pcDefined, CCPPSyntaxDeclarator* pcDefinedDeclarator);
	CCPPTemplate*			FindFunctionTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator);
	void					FunctionTemplateSignature(CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxDeclarator* pcDeclarator, CChars* pszDest);
	void					DeclareClassTemplate(CCPPSyntaxElaboratedTypeSpecifier* pcSpecifier, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	void					DefineClassTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxClassSpecifier* pcSpecifier);
	CCPPTemplate*			Specialisation(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, CCPPSyntaxNameComponent* pcPattern);
	CCPPTemplate*			OutsideMemberOwner(CCPPTemplate* pcPrimary, CCPPSyntaxNameComponent* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplate*			NewTemplate(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember);
	bool					ExplicitInstantiation(CCPPSyntax* pcDeclaration, bool bExtern);
	bool					ExplicitlyInstantiatedOnce(void* pvInstance, CCPPSyntax* pcNode);
	bool					ExplicitMemberInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern);
	bool					SameParameters(SCPP65Type* psFirst, SCPP65Type* psSecond);
	bool					ExplicitFunctionInstantiation(CCPPSyntaxInitDeclarator* pcInit, bool bExtern);
	bool					IsInstantiatedElsewhere(CCPPFunction* pcFunction);
	CCPPTemplate*			MemberClassTemplate(CCPPSymbol* pcSymbol, CCPPClass* pcClass);
	CCPPTemplate*			GetTemplate(CCPPSymbol* pcSymbol);
	CCPPTemplate*			GetTemplateByName(int64 lliName, ECPPSyntax eDefinition, CCPPScope* pcLookup);
	CCPPTemplate*			GetComponentTemplate(CCPPSyntaxNameComponent* pcComponent);
	CCPPTemplateArguments*			BindArguments(CCPPSyntaxTemplateDeclaration* pcTemplate, CArraySyntax* papcArguments, CArrayCPP65Types* papsDeduced, CChars* pszArguments, CCPPSyntax* pcNode, CArrayInt64* palliDeduced = NULL, CArrayCPPDeducedPacks* pasPacks = NULL);
	bool					ValueArgumentFits(CCPPSyntaxParameter* pcParameter, SCPPExpandedArgument* psArgument, int64 lliValue, CCPPTemplateArguments* pcBindings);
	bool					ConstantFits(SCPP65Type* psType, int64 lliValue);
	SCPP65Type*				ValueParameterType(CCPPSymbol* pcParameter, SCPP65Type* psOtherwise);
	bool					IsEntityParameter(CCPPSyntaxParameter* pcParameter);
	bool					BindEntity(SCPPBinding* psBinding, SCPPExpandedArgument* psArgument, CChars* pszArguments);
	bool					BindTemplate(SCPPBinding* psBinding, CCPPSyntax* pcArgument, CCPPTemplate* pcDeduced, CCPPSyntax* pcNode, CChars* pszArguments);
	bool					BindTypePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CArrayCPP65Types* papsDeduced, CChars* pszArguments);
	bool							SameReturnWritten(CCPPSyntaxDeclSpecifiers* pcFirst, CCPPSyntaxDeclSpecifiers* pcSecond);
	bool							TemplateParametersMatch(CCPPSyntaxTemplateDeclaration* pcParameter, CCPPSyntaxTemplateDeclaration* pcArgument);
	bool							IsPackParameter(CCPPSyntaxTemplateDeclaration* pcTemplate, size uiIndex);
	bool							BindDeducedPack(SCPPBinding* psBinding, CArrayCPPDeducedPacks* pasPacks, size uiParameter, CChars* pszArguments);
	bool					BindValuePack(SCPPBinding* psBinding, CArrayCPPExpandedArguments* pasArguments, size uiFirst, CChars* pszArguments);
	void					AppendMangledValue(CChars* pszArguments, int64 lliValue);
	SCPPBinding*			FindBinding(CCPPSymbol* pcParameter);
	CCPPClass*			InstantiateClass(CCPPTemplate* pcTemplate, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPClass*			InstanceOfClass(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode);
	CCPPClass*			PlaceholderClass(CCPPSymbol* pcParameter);
	CCPPTemplateArguments*	BindAsItself(CCPPSyntaxTemplateDeclaration* pcDeclaration, CChars* pszArguments, CCPPSyntax* pcNode);
	CCPPClass*			InstantiateClassAsItself(CCPPTemplate* pcTemplate);
	CCPPFunction*		InstantiateFunctionAsItself(CCPPTemplate* pcTemplate);
	CCPPClass*			UndefinedInstance(CCPPSymbol* pcSymbol, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPTemplate*			FindSpecialisation(CCPPTemplate* pcPrimary, CCPPTemplateArguments* pcArguments, CCPPTemplateArguments** ppcBindings, CCPPSyntax* pcNode);
	bool					SpecialisationAtLeastAs(CCPPTemplate* pcFirst, CCPPTemplate* pcSecond, CCPPTemplate* pcPrimary, CCPPSyntax* pcNode, bool* pbKnown);
	CCPPTemplateArguments*			NewBindings(void);
	CCPPFunction*			InstantiateFunction(CCPPTemplate* pcTemplate, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	CCPPFunction*			InstantiateFunctionFor(CCPPTemplate* pcTemplate, SCPP65Type* psFunction, CCPPSyntax* pcNode);
	CCPPFunction*		InstanceOfFunction(CCPPTemplate* pcTemplate, CCPPTemplateArguments* pcBindings, CChars* pszArguments, CCPPSyntax* pcNode);
	void					AddMemberTemplateSpecialisations(CCPPTemplate* pcPrimary);
	CCPPTemplate*			FindFunctionSpecialisation(CCPPTemplate* pcPrimary, CChars* pszArguments, CCPPSyntax* pcNode);
	CCPPTemplateArguments*			RenameBindings(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	CCPPTemplateArguments*	RenamedBindings(CCPPTemplateArguments* pcFrom, CCPPSyntaxTemplateDeclaration* pcDeclaration);
	bool					IsClassTemplateDefinition(CCPPSyntax* pcDeclaration);
	CCPPSyntaxClassSpecifier*	MemberClassDefinition(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration** ppcDeclaration);
	CCPPTemplateArguments*	MemberTemplateClassBindings(CCPPTemplate* pcTemplate);
	bool					MemberTemplate(CCPPClass* pcClass, CCPPSyntaxTemplateDeclaration* pcTemplateDeclaration);
	ECPPRefQualifier		RefQualifier(CCPPSyntaxFunctionSuffix* pcSuffix);
	bool					DefineMemberTemplate(CCPPTemplate* pcTemplate, CCPPSyntax* pcNode);
};


#endif // __C_P_P_INSTANTIATOR_H__

