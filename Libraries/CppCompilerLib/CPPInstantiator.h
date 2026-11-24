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


typedef CArrayTemplatePtr<CArrayCPP65Types>	CArrayCPPPacks;
typedef CArrayTemplatePtr<CArrayInt64>		CArrayCPPValuePacks;


//A template parameter pack's element as a parameter of its own (CCPPInstantiator::PackElement), in the bindings it is bound
//in; or a function parameter pack's as a variable of its own (FunctionPackElement), in the function it is of.
struct SCPPPackElement
{
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcFunction;
	CCPPSymbol*				pcPack;
	size					uiIndex;
	CCPPSymbol*				pcElement;
};


typedef CArrayTemplate<SCPPPackElement>	CArrayCPPPackElements;


//An argument list with its pack expansions expanded, for the bindings and function it was expanded in
//(CCPPInstantiator::ExpandArguments).  papcExpanded is NULL if it couldn't be.
struct SCPPExpandedArguments
{
	CArraySyntax*			papcArguments;
	CCPPTemplateArguments*	pcBindings;
	CCPPFunction*			pcIn;
	CArraySyntax*			papcExpanded;
};


typedef CArrayTemplate<SCPPExpandedArguments>	CArrayCPPExpandedArgumentLists;


//What a definition written outside its class template defines (SCPPOutsideDefinition).
enum ECPPOutsideDefinition
{
	CPPOD_MemberSpecialisation,			//template<> R Class<Arguments>::f() { ... }, given to its instance (SpecialiseMembers).
	CPPOD_MemberTemplateSpecialisation,	//template<> template<> R Class<Arguments>::f<T>(...) { ... }, given to the instance's member template (AddMemberTemplateSpecialisations).
	CPPOD_MemberClass,					//template<class T> struct Class<T>::Inner { ... }; and template<class T> template<class U> struct Class<T>::Pair { ... }; (MemberClassDefinition).
	CPPOD_MemberVariableTemplate,		//template<class C> template<class T> T W<C>::value = ...;, each instance's member variable template's definition (DefineMemberVariableTemplate).
	CPPOD_MemberVariableSpecialisation,	//template<class C> template<class T> T W<C>::value<T*> = ...;, each instance's member variable template's partial specialisation.
	CPPOD_StaticMember,					//template<class T> int Class<T>::s = ...;, defined for each instance (DefineInstanceStatics).
};


//A definition written outside its class template, kept to be given to each instance: what it is, and the pieces of it
//its readers want, worked out once when it is seen (AddOutsideDefinition).
struct SCPPOutsideDefinition
{
	ECPPOutsideDefinition			eKind;
	CCPPSyntaxTemplateDeclaration*	pcDeclaration;	//The template declaration whose parameters name the class template's (the inner template<> of a member template's specialisation); NULL for a member's specialisation.
	CCPPSyntaxTemplateDeclaration*	pcMember;		//A member template's own template declaration, in pcDeclaration; else NULL.
	CCPPSyntaxFunctionDefinition*	pcFunction;		//The function defined, or NULL.
	CCPPSyntaxSimpleDeclaration*	pcSimple;		//The declaration of the class, variable or static member defined, or NULL.
	CCPPSyntaxClassSpecifier*		pcSpecifier;	//The member class defined, or NULL.
	CCPPSyntaxInitDeclarator*		pcInit;			//The static member's declarator, or NULL.
	CCPPSyntaxName*					pcName;			//The name defined (Class<T>::s, W<C>::value), or NULL for a class.
};


typedef CArrayTemplate<SCPPOutsideDefinition>	CArrayCPPOutsideDefinitions;


//The unit's templates (CCPPUnit::GetInstantiator): each template declaration read, a class template's, a function
//template's and a variable template's instances made for their arguments, their explicit and partial
//specialisations found, the definitions written outside a class template given to each instance, and explicit
//instantiations.  [temp]
class CCPPInstantiator
{
protected:
	CCPPUnit*				mpcUnit;
	CArrayCPPOutsideDefinitions		masOutsideDefinitions;	//The definitions written outside their class template, in the order seen, to give each instance.
	CArrayCPPPacks					mapapsPacks;			//The packs' types (NewPack).
	CArrayCPPValuePacks				mapalliPacks;			//The non-type packs' values (NewValuePack).
	CArrayCPPSymbols				mapcPackSymbols;		//The symbols made for function and template parameter packs' elements.
	CArrayCPPPackElements			masPackElements;		//Template parameter packs' elements in each bindings (PackElement), and function ones' in each function.
	CArrayCPPExpandedArgumentLists	masExpandedArguments;	//Argument lists with their packs expanded (ExpandArguments).
	size							muiRefusedArguments;	//Template arguments whose values weren't of their parameters' types (ValueArgumentFits), all told.

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);
	size					NumRefusedArguments(void) const;

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

	bool					FindSpecifier(CCPPClass* pcClass);
	void					AddOutsideDefinition(ECPPOutsideDefinition eKind, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntaxTemplateDeclaration* pcMember, CCPPSyntax* pcDefined);
	void					MemberSpecialisation(CCPPSyntaxFunctionDefinition* pcDefinition);
	void					SpecialiseMembers(CCPPClass* pcClass);
	void					SpecialiseMember(CCPPClass* pcClass, CCPPSyntaxFunctionDefinition* pcDefinition);
	bool					QualifierIsInstance(CCPPSyntaxNameComponent* pcQualifier, CCPPClass* pcClass, CCPPSyntax* pcNode);
	void					DefineOutsideMember(CCPPClass* pcClass, CCPPTemplate* pcMember);
	void					DefineOutsideMembers(CCPPTemplate* pcMember);
	bool					IsOutsideMemberOf(CCPPTemplate* pcMember, CCPPClass* pcClass);

	CArrayCPP65Types*		NewPack(void);
	CArrayInt64*			NewValuePack(void);
	void					FindPacks(CCPPSyntax* pcPattern, CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks);
	bool					PackExpansionLength(CArrayCPPBindingPtrs* papsPacks, CArrayCPPSymbols* papcFunctionPacks, CCPPSyntax* pcNode, size* puiElements);
	SCPPFunctionPack*		FindFunctionPack(CCPPSymbol* pcSymbol);
	bool					ExpandPattern(CCPPSyntax* pcPattern, CCPPSyntax* pcParent, CArraySyntax* papcExpanded);
	CArraySyntax*			ExpandArguments(CArraySyntax* papcArguments);
	CArraySyntax*			ListElements(CCPPSyntax* pcList);
	CCPPSymbol*				PackElement(SCPPBinding* psPack, size k);
	bool					ExpandTemplateArguments(CArraySyntax* papcArguments, CArrayCPPExpandedArguments* pasExpanded);
	SCPP65Type*				ExpandedArgumentType(SCPPExpandedArgument* psArgument);
	bool					ExpandedArgumentValue(SCPPExpandedArgument* psArgument, int64* plliValue);
	bool					PackSize(CCPPSymbol* pcPack, int64* plliValue);
	SCPPBinding*			PackBinding(CCPPSyntaxParameter* pcParameter);
	CCPPSymbol*				FunctionPackElement(CCPPFunction* pcFunction, CCPPSymbol* pcPack, size k);
	bool					HasPackExpansion(CArraySyntax* papcArguments);
	CCPPSyntax*				ExpansionPattern(CCPPSyntax* pcNode);
};


#endif // __C_P_P_INSTANTIATOR_H__

