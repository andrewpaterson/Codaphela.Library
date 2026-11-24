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
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPUnit.h"
#include "CPPClasses.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's diagnostics, declarators, context and types, asked for once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcDeclarators = pcUnit->GetDeclarators();
	mpcContext = pcUnit->GetContext();
	mpcTypes = pcUnit->GetTypes();
	mapcClasses.Init();
	mbLayouts = false;
	maszLayouts.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::Kill(void)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		pcClass->Kill();
		SafeFree(pcClass);
	}
	mapcClasses.Kill();
	maszLayouts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every class the unit has made, in the order it made them.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPPClasses* CCPPClasses::GetClassList(void) { return &mapcClasses; }


//////////////////////////////////////////////////////////////////////////
//
// Each class laid out is written down as a line (AddLayout), for
// GetLayouts: what a target's layout rules made of the program's classes.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::SetLayouts(bool bLayouts)
{
	mbLayouts = bLayouts;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the lines of the classes laid out (SetLayouts) to paszLines, which
// is sorted and has each line once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::GetLayouts(CArrayChars* paszLines)
{
	size	i;

	for (i = 0; i < maszLayouts.NumElements(); i++)
	{
		paszLines->InsertIntoSorted(maszLayouts.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClasses::NewClass(CCPPSymbol* pcSymbol, int64 lliName)
{
	CCPPClass*	pcClass;

	pcClass = NewMalloc<CCPPClass>();
	pcClass->Init(pcSymbol, lliName);
	pcClass->mpsType = mpcTypes->Class(pcClass);
	mapcClasses.Add(pcClass);
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class (not a template instance) a symbol names.  A class used before it is
// defined, or only through pointers, is laid out when it is first needed whole.
// A member class of a class template is each instance's own, with the instance's
// template arguments: the instance's (pcEnclosing) that is given or that the
// bindings in use are of (EnclosingInstance).  [temp.mem.class]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClasses::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode, CCPPClass* pcEnclosing)
{
	CCPPClass*	pcClass;
	size		i;

	pcEnclosing = pcEnclosing ? pcEnclosing : mpcUnit->GetNames()->EnclosingInstance(pcSymbol);
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mpcSymbol == pcSymbol) && !pcClass->mpcTemplate && (pcClass->mpcEnclosing == pcEnclosing))
		{
			return pcClass;
		}
	}
	pcClass = NewClass(pcSymbol, pcSymbol->mlliName);
	if (pcEnclosing)
	{
		pcClass->mpcEnclosing = pcEnclosing;
		pcClass->mpcBindings = pcEnclosing->mpcBindings;
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A class without a name (typedef struct { ... } SName;) is known by its definition.
// One in a class template is a different class in each instance (its bindings).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClasses::GetUnnamedClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*	pcClass;
	CCPPTemplateArguments*	pcBindings;
	CCPPSyntax*		pcParent;
	size			i;

	for (pcBindings = mpcContext->GetBindings(); pcBindings; pcBindings = pcBindings->mpcOuter)
	{
		pcParent = pcSpecifier->mpcParent;
		while (pcParent && (!pcBindings->mpcClass || (pcParent != pcBindings->mpcClass->mpcSpecifier)))
		{
			pcParent = pcParent->mpcParent;
		}
		if (pcParent)
		{
			break;
		}
	}

	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (!pcClass->mpcSymbol && (pcClass->mpcSpecifier == pcSpecifier) && !pcClass->mpcTemplate && (pcClass->mpcBindings == pcBindings))
		{
			return pcClass;
		}
	}
	pcClass = NewClass(NULL, -1);
	pcClass->mpcSpecifier = pcSpecifier;
	pcClass->mpcBindings = pcBindings;
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// A class (not a template instance) by name, for a qualifier the parser
// couldn't resolve: one visible from pcLookup, else any.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClasses::FindClass(int64 lliName, CCPPScope* pcLookup)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcFound;
	size			i;

	pcFound = NULL;
	for (i = 0; i < mapcClasses.NumElements(); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if ((pcClass->mlliName == lliName) && !pcClass->mpcTemplate)
		{
			if (pcClass->mpcSymbol && pcClass->mpcSymbol->mpcScope->Encloses(pcLookup))
			{
				return pcClass;
			}
			pcFound = pcFound ? pcFound : pcClass;
		}
	}
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::CompleteClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (pcClass->IsComplete())
	{
		return true;
	}
	if (pcClass->IsBeingMade())
	{
		return mpcDiagnostics->Error(pcNode, "A class cannot contain itself");
	}
	if (!mpcUnit->GetInstantiator()->FindSpecifier(pcClass))
	{
		return mpcDiagnostics->Error(pcNode, "The class has not been defined");
	}
	return LayoutClass(pcClass, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// CompleteClass for the class of an object that is being made (a variable,
// a temporary, what new makes): it can't be abstract.  A base of an object
// can, so this is not asked for a base.  [class.abstract]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::CompleteObjectClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	if (!CompleteClass(pcClass, pcNode))
	{
		return false;
	}
	if (pcClass->IsAbstract())
	{
		return mpcDiagnostics->Error(pcNode, "An object of an abstract class can't be made: a pure virtual function of it has no body");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds a class's base classes without laying it out, so a pointer to it can be
// converted to a pointer to a base.  Offsets are worked out by LayoutClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::ResolveBases(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxBaseSpecifier*	pcBaseSpecifier;
	size						i;
	bool						bResult;

	if (pcClass->HasBases())
	{
		return true;
	}
	if (!mpcUnit->GetInstantiator()->FindSpecifier(pcClass))
	{
		return mpcDiagnostics->Error(pcNode, "The class has not been defined");
	}

	pcClass->meState = CPPCS_ResolvingBases;
	mpcContext->EnterBindings(pcClass->mpcBindings);
	bResult = true;
	for (i = 0; i < pcClass->mpcSpecifier->macpBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcClass->mpcSpecifier->macpBases.GetPtr(i);
		bResult &= pcBaseSpecifier->mbPack ? AddPackBases(pcClass, pcBaseSpecifier) : AddBase(pcClass, pcBaseSpecifier, pcBaseSpecifier->mpcName);
	}
	pcClass->mpcBase = (pcClass->macBases.NumElements() > 0) ? pcClass->macBases.Get(0)->pcClass : NULL;
	pcClass->meState = CPPCS_BasesKnown;
	mpcContext->LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a base specifier names (pcName: its name, or one of its pack
// expansion's) is a base of pcClass, its bases found.  Not one declared
// final.  [class] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::AddBase(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier, CCPPSyntaxName* pcName)
{
	CCPPClass*	pcBaseClass;
	SCPPBase*	psBase;
	bool		bResult;

	pcBaseClass = mpcUnit->GetNames()->GetNameClass(pcName, pcBaseSpecifier);
	if (!pcBaseClass)
	{
		//[class.derived] 2
		return mpcDiagnostics->Error(pcBaseSpecifier, (pcName->mpcSymbol && !pcName->mpcSymbol->IsClass()) ? "A base class is a class: the name is another kind of type" : "This base class is not supported by the compiler yet");
	}
	bResult = ResolveBases(pcBaseClass, pcBaseSpecifier);
	if (pcBaseClass->mpcSpecifier && pcBaseClass->mpcSpecifier->mbFinal)
	{
		bResult &= mpcDiagnostics->Error(pcBaseSpecifier, "The base class is declared final");
	}
	psBase = pcClass->macBases.Add();
	psBase->pcClass = pcBaseClass;
	psBase->iOffset = 0;
	psBase->bVirtual = pcBaseSpecifier->mbVirtual;
	psBase->eAccess = (pcBaseSpecifier->meAccess != CPPA_None) ? pcBaseSpecifier->meAccess : ((pcClass->mpcSpecifier->meKey == CT_class) ? CPPA_Private : CPPA_Public);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// : T... and : Leaf<I, T>... have a base for each element of the packs the
// name names, in order: the name expanded as any pattern is (ExpandPattern),
// of type packs and value packs alike.  [temp.variadic] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::AddPackBases(CCPPClass* pcClass, CCPPSyntaxBaseSpecifier* pcBaseSpecifier)
{
	CArraySyntax	apcNames;
	size			k;
	bool			bResult;

	apcNames.Init();
	bResult = mpcUnit->ExpandPattern(pcBaseSpecifier->mpcName, pcBaseSpecifier, &apcNames);
	for (k = 0; bResult && (k < apcNames.NumElements()); k++)
	{
		bResult = AddBase(pcClass, pcBaseSpecifier, (CCPPSyntaxName*)apcNames.GetPtr(k));
	}
	apcNames.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class: the bases in order (an empty base takes no space), the vtable
// pointer if no base has one, then the fields.  Members are read first: functions
// are declared (in class definitions are generated when called) and fields are
// collected.  A template instance's members and its members defined outside it are
// worked out with its template arguments.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::LayoutClass(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxBaseSpecifier*		pcBaseSpecifier;
	CCPPSyntaxNameComponent*		pcComponent;
	CCPPTemplate*					pcMember;
	CCPPFunction*					pcFunction;
	size							i;
	size							j;
	int								iClassAlign;
	int								iPack;
	bool							bResult;

	mpcUnit->GetStatistics()->Count(CPPCC_ClassLayouts);
	pcSpecifier = pcClass->mpcSpecifier;
	mpcContext->EnterBindings(pcClass->mpcBindings);
	bResult = true;

	pcClass->mbUnion = pcSpecifier->meKey == CT_union;
	pcComponent = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
	if (pcComponent && !pcClass->mpcTemplate)
	{
		pcClass->mlliName = mpcUnit->GetComponentName(pcComponent);
	}

	bResult &= ResolveBases(pcClass, pcSpecifier);
	pcClass->meState = CPPCS_LayingOut;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBaseSpecifier = (CCPPSyntaxBaseSpecifier*)pcSpecifier->macpBases.GetPtr(i);
		bResult &= CompleteClass(pcClass->macBases.Get(i)->pcClass, pcBaseSpecifier);
		if (pcClass->macBases.Get(i)->pcClass->mbUnion)
		{
			//[class.union] 2
			bResult &= mpcDiagnostics->Defer(pcBaseSpecifier->miFirstToken, "A union is no base class");
		}
		for (j = 0; j < i; j++)
		{
			if (pcClass->macBases.Get(j)->pcClass == pcClass->macBases.Get(i)->pcClass)
			{
				//[class.mi] 3
				bResult &= mpcDiagnostics->Defer(pcBaseSpecifier->miFirstToken, "A class is a direct base of another once");
				break;
			}
		}
	}

	//Its friends are of its namespace, whatever is being generated when it is completed.
	mpcContext->EnterNamespace(pcClass->mpcSymbol ? pcClass->mpcSymbol->mpcScope->GetNamespace() : mpcContext->GetNamespace());
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(pcClass, pcSpecifier->macpMembers.GetPtr(i));
	}
	mpcContext->LeaveNamespace();

	//A class template's members, and its member classes', defined outside it (so far: one defined later is given to
	//the instances already laid out then, DefineOutsideMembers).
	for (i = 0; (pcClass->mpcTemplate || pcClass->mpcEnclosing) && (i < mpcUnit->GetTemplates()->NumElements()); i++)
	{
		pcMember = mpcUnit->GetTemplates()->GetPtr(i);
		if (mpcUnit->GetInstantiator()->IsOutsideMemberOf(pcMember, pcClass))
		{
			mpcUnit->GetInstantiator()->DefineOutsideMember(pcClass, pcMember);
		}
	}

	//Its members a template<> definition specialises for it.
	if (pcClass->mpcTemplate)
	{
		mpcUnit->GetInstantiator()->SpecialiseMembers(pcClass);
	}

	CompleteMembers(pcClass);

	//The bases, the vtable pointer, then the fields.
	iClassAlign = AlignasAlignment(&pcSpecifier->macpAttributes, true);
	if (iClassAlign < 0)
	{
		bResult = false;
	}
	//(A member is aligned to at most the lesser of #pragma pack's bytes and the target's limit.)
	iPack = mpcUnit->GetTarget()->GetMemberAlignmentLimit();
	iPack = (iPack > 0) ? CCPPClassLayout::PackAlignment(iPack, pcSpecifier->miPack) : pcSpecifier->miPack;
	PlaceMembers(pcClass, iPack, iClassAlign);
	if (mbLayouts)
	{
		AddLayout(pcClass);
	}
	pcClass->meState = CPPCS_Complete;
	bResult &= CheckOverrides(pcClass);
	if (pcClass->mcVtable.mpcAmbiguous)
	{
		//[class.virtual] 2
		bResult &= mpcDiagnostics->Defer(pcSpecifier->miFirstToken, "A virtual function of a virtual base has two final overriders in the class, neither overriding the other");
	}

	if (pcClass->mpcInheritedBase)
	{
		InheritConstructors(pcClass);
	}

	//A class that needs one but declares none gets a constructor written for it when it is called.
	if (!pcClass->mbConstructors && pcClass->NeedsConstructor())
	{
		pcFunction = mpcUnit->GetFunctionDeclarer()->AddImplicit(pcClass, pcClass->mlliName, CPPFK_Constructor, &mpcTypes->msVoid, true);
	}
	mpcContext->LeaveBindings();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What the compiler adds to a class once its members are read, a
// closure's too: the destructor its bases or fields need to be destroyed,
// for a virtual destructor the deleting one, and its vtable.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::CompleteMembers(CCPPClass* pcClass)
{
	CCPPFunction*	pcFunction;

	//A destructor is needed to destroy the bases or the fields.
	if (!pcClass->mpcDestructor && pcClass->NeedsDestructor())
	{
		pcFunction = mpcUnit->GetFunctionDeclarer()->AddImplicit(pcClass, pcClass->mlliName, CPPFK_Destructor, &mpcTypes->msVoid, false);
		pcClass->mpcDestructor = pcFunction;
	}
	if (pcClass->HasVirtualDestructor())
	{
		//delete of a pointer to a base frees the object as its own class does, which only the object knows: by a
		//virtual function of its class, written for it, that destroys it and frees it.  [expr.delete] 3, 9, 10
		pcFunction = mpcUnit->GetFunctionDeclarer()->AddImplicit(pcClass, CPP_DELETING_NAME, CPPFK_Function, &mpcTypes->msVoid, false);
		pcFunction->mbDeleting = true;
		pcFunction->mbVirtual = true;
		pcFunction->mbDefined = true;
		pcClass->mpcDeleting = pcFunction;
	}
	pcClass->mcVtable.LayoutOwn(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a class's bases, vtable pointer and fields (CCPPClassLayout), a
// closure's too, and gives its type their size.  iPack is #pragma pack's
// bytes and iAlign alignas's, 0 for none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::PlaceMembers(CCPPClass* pcClass, int iPack, int iAlign)
{
	CCPPClassLayout	cLayout;

	cLayout.Init(pcClass, iPack, mpcUnit->GetTarget()->GetPointerBytes());
	cLayout.Layout(iAlign);
	mpcTypes->SetClassBytes(pcClass->mpsType, cLayout.GetBytes());
	cLayout.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Declares a member of a class being laid out.  Functions defined in the class are
// generated when they are called.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::ClassMember(CCPPClass* pcClass, CCPPSyntax* pcMember)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	CCPPSyntaxClassSpecifier*		pcClassSpecifier;
	SCPPField*					psField;
	CCPPVariable*					pcVariable;
	CCPPFunction*					pcFunction;
	SCPP65Type*						psType;
	SCPP65Type*						psElement;
	size							i;
	size							uiMembers;
	bool							bPure;
	bool							bResult;
	bool							bFriend;
	int64							lliZero;
	int								iAlign;

	switch (pcMember->GetKind())
	{
	case CPPS_AccessSpecifier:
	case CPPS_EmptyDeclaration:
	case CPPS_AliasDeclaration:
		return true;

	case CPPS_StaticAssert:
		//A class template's are worked out for each instance, whose arguments they may be about; a class's own were
		//when it was defined (ClassStaticAsserts).
		return !pcClass->mpcTemplate || mpcUnit->StaticAssert((CCPPSyntaxStaticAssert*)pcMember);

	case CPPS_UsingDeclaration:
		ClassUsing(pcClass, (CCPPSyntaxUsingDeclaration*)pcMember);
		return true;

	case CPPS_TemplateDeclaration:
		return mpcUnit->GetInstantiator()->MemberTemplate(pcClass, (CCPPSyntaxTemplateDeclaration*)pcMember);

	case CPPS_FunctionDefinition:
		pcDefinition = (CCPPSyntaxFunctionDefinition*)pcMember;
		if ((pcDefinition->meBody == CPPFB_Default) && pcDefinition->mpcDeclarator->GetName() && (mpcUnit->GetFunctionName(pcDefinition->mpcDeclarator->GetName()->GetLast()) == CPP_OPERATOR_NAME(CT_Assignment)))
		{
			//operator = (...) = default is the implicit copy assignment, which = does when the class declares none.
			return mpcUnit->GetFunctionDeclarer()->CheckDefaultedAssignment(pcClass, pcDefinition);
		}
		bFriend = pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_FRIEND);
		uiMembers = pcClass->mapcMembers.NumElements();
		pcFunction = mpcUnit->GetFunctionDeclarer()->DeclareQuietly(pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator, bFriend ? NULL : pcClass, CPPDA_Definition);
		if (!pcFunction && pcDefinition->mpcSpecifiers && (pcDefinition->mpcSpecifiers->muiFlags & CPPDS_VIRTUAL))
		{
			//Leaving out a virtual function would move the vtable's other entries.
			return mpcDiagnostics->Error(pcDefinition, "This virtual function's declaration is not supported by the compiler yet");
		}
		if (!pcFunction)
		{
			return true;
		}
		if (IsDeclaredAgainInClass(pcClass, pcFunction, uiMembers))
		{
			return mpcDiagnostics->Defer(pcDefinition->mpcDeclarator->miFirstToken, "The member function is declared twice in its class");
		}
		if (!mpcUnit->GetFunctionDeclarer()->CheckDefaultedOrDeleted(pcFunction, pcDefinition))
		{
			return false;
		}
		pcFunction->SetDefinition(pcDefinition);
		mpcUnit->GetConstants()->CheckDefinition(pcFunction);
		return true;

	case CPPS_SimpleDeclaration:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		pcSpecifiers = pcDeclaration->mpcSpecifiers;
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_FRIEND))
		{
			//A friend function is a function of the namespace, not a member.
			for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
			{
				pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
				if (pcInit->mpcDeclarator && pcInit->mpcDeclarator->IsFunction())
				{
					mpcUnit->GetFunctionDeclarer()->DeclareQuietly(pcSpecifiers, pcInit->mpcDeclarator, NULL, CPPDA_Declaration);
				}
			}
			return true;
		}
		if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
		{
			return true;
		}
		if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			//A nested class is laid out when it is used.
			pcClassSpecifier = (CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType;
			if (!mpcUnit->ClassStaticAsserts(pcClassSpecifier))
			{
				return false;
			}
			if (pcClassSpecifier->mpcSymbol)
			{
				GetClass(pcClassSpecifier->mpcSymbol, pcMember)->mpcSpecifier = pcClassSpecifier;
			}
			else if (pcDeclaration->macpDeclarators.NumElements() == 0)
			{
				//union { ... }; is an unnamed field whose members are the class's (FindField looks in it).
				psType = GetUnnamedClass(pcClassSpecifier)->mpsType;
				if (!CompleteClass(psType->pcClass, pcMember))
				{
					return false;
				}
				pcClass->macFields.Add()->Init(NULL, psType);
				return true;
			}
		}

		bResult = true;
		iAlign = pcSpecifiers ? AlignasAlignment(&pcSpecifiers->macpSpecifiers) : 0;
		if (iAlign < 0)
		{
			return false;
		}
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if (pcInit->mpcDeclarator->IsFunction() || (!pcInit->mpcBitField && pcInit->mpcDeclarator->IsFunctionByType(pcSpecifiers)))
			{
				//virtual int f() = 0;
				bPure = pcInit->mpcInitializer && mpcUnit->GetFolder()->Constant(pcInit->mpcInitializer, &lliZero) && (lliZero == 0);
				uiMembers = pcClass->mapcMembers.NumElements();
				pcFunction = mpcUnit->GetFunctionDeclarer()->DeclareQuietly(pcSpecifiers, pcInit->mpcDeclarator, pcClass, bPure ? CPPDA_Pure : CPPDA_Declaration);
				if (!pcFunction && (pcSpecifiers->muiFlags & CPPDS_VIRTUAL))
				{
					//Leaving out a virtual function would move the vtable's other entries.
					bResult &= mpcDiagnostics->Error(pcInit, "This virtual function's declaration is not supported by the compiler yet");
				}
				else if (!pcFunction)
				{
				}
				else if (IsDeclaredAgainInClass(pcClass, pcFunction, uiMembers))
				{
					bResult &= mpcDiagnostics->Defer(pcInit->mpcDeclarator->miFirstToken, "The member function is declared twice in its class");
				}
				else if ((pcInit->meInitializer == CPPI_Default) && (pcFunction->meKind != CPPFK_Function))
				{
					pcFunction->mbImplicit = true;
				}
				continue;
			}
			if (pcInit->mpcBitField)
			{
				bResult &= ClassBitField(pcClass, pcSpecifiers, pcInit);
				continue;
			}
			if (!pcInit->mpcSymbol)
			{
				continue;
			}

			psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				bResult = false;
				continue;
			}
			if (psType->IsFunction())
			{
				//A function's type that IsFunctionByType didn't take for a member function's (one made of a template's
				//parameter): it is no field.  [temp.arg.type] 3
				bResult &= mpcDiagnostics->Error(pcInit, "A member function declared by a type that depends on a template parameter is not supported by the compiler yet");
				continue;
			}
			if (!InjectedNameReached(pcClass, pcSpecifiers, psType, pcInit))
			{
				bResult = false;
				continue;
			}
			if (pcSpecifiers->muiFlags & CPPDS_STATIC)
			{
				if (pcInit->mpcInitializer && !(pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) && !(psType->bConst && psType->IsInteger()))
				{
					//[class.static.data] 3
					bResult &= mpcDiagnostics->Defer(pcInit->miFirstToken, "Only a static data member that is a const integer or enum, or constexpr, is initialised in its class");
				}
				pcVariable = mpcUnit->AddVariable(mpcUnit->GetGlobalVariables(), pcInit->mpcSymbol, psType, CPPVS_Global, 0);
				pcVariable->mpcClass = pcClass;
				pcVariable->mpcInClassInitialiser = pcInit->mpcInitializer;
				continue;
			}

			if (pcClass->mbUnion && psType->IsReference())
			{
				//[class.union] 2
				bResult &= mpcDiagnostics->Defer(pcInit->miFirstToken, "A union has no reference member");
				continue;
			}
			psElement = psType->ElementType();
			if (psElement->IsClass() && !CompleteClass(psElement->pcClass, pcInit))
			{
				bResult = false;
				continue;
			}
			if (psType->IsArray() && psElement->IsClass())
			{
				psType = mpcTypes->Array(psElement, psType->iCount);
			}
			psField = pcClass->macFields.Add();
			psField->Init(pcInit->mpcSymbol, psType);
			psField->iAlign = iAlign;
			psField->pcInitialiser = pcInit->mpcInitializer;
			psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
		}
		return bResult;

	default:
		return mpcDiagnostics->Error(pcMember, "This member is not supported by the compiler yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::f; in a class makes Base's functions named f overloads of the class's
// own.  using Base::Base; inherits Base's constructors (see InheritConstructors).
// Other using declarations (of types) are left out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::ClassUsing(CCPPClass* pcClass, CCPPSyntaxUsingDeclaration* pcUsing)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*	pcBase;
	SCPPUsing*	psUsing;
	int64			lliName;
	CCPPSymbol*		pcSymbol;
	CCPPClass*		pcDeclaring;
	bool			bReached;

	pcName = pcUsing->mpcName;
	if (pcUsing->mbTypename || !pcName || !pcName->IsQualifiedByClass())
	{
		return;
	}
	lliName = mpcUnit->GetFunctionName(pcName->GetLast());
	if (lliName == -1)
	{
		return;
	}
	mpcDiagnostics->BeginQuiet();
	pcBase = mpcUnit->GetNames()->GetComponentClass(pcName->GetQualifier(), NULL, pcUsing, pcClass);
	mpcDiagnostics->EndQuiet();
	if (!pcBase || (pcBase == pcClass))
	{
		return;
	}
	if (pcBase->mlliName == lliName)
	{
		pcClass->mpcInheritedBase = pcBase;
		return;
	}

	//What it names is a member the class may name: a base's private member isn't.  [namespace.udecl] 17
	pcSymbol = pcBase->FindMemberSymbol(lliName, CPPSK_Field, &pcDeclaring);
	if (!pcSymbol)
	{
		pcSymbol = pcBase->FindMemberSymbol(lliName, CPPSK_Function, &pcDeclaring);
	}
	if (pcSymbol)
	{
		mpcDiagnostics->BeginSilent();
		bReached = mpcUnit->GetBinder()->UsingReaches(pcSymbol, pcUsing);
		mpcDiagnostics->EndSilent();
		if (!bReached)
		{
			mpcDiagnostics->Defer(pcUsing->miFirstToken, "The using-declaration names a base's private member, which the class can't reach");
			return;
		}
	}
	psUsing = pcClass->macUsings.Add();
	psUsing->pcBase = pcBase;
	psUsing->lliName = lliName;
}


//////////////////////////////////////////////////////////////////////////
//
// using Base::Base;  The class gets a constructor for each of the base's (not its
// default, copy or move constructors, constructor templates, or ones with the same
// parameters as the class's own) that calls the base's with its arguments and makes
// the rest of the object as a default constructor does.  A class that declares no
// constructor of its own keeps its default constructor.  [class.inhctor.init]
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::InheritConstructors(CCPPClass* pcClass)
{
	CCPPClass*	pcBase;
	CCPPFunction*	pcInherited;
	CCPPFunction*	pcOwn;
	CCPPFunction*	pcFunction;
	SCPP65Type*		psParameter;
	size			i;
	size			j;
	size			k;
	bool			bSame;
	bool			bAdded;

	pcBase = pcClass->mpcInheritedBase;
	if (!CompleteClass(pcBase, NULL))
	{
		return;
	}
	bAdded = false;
	for (i = 0; i < pcBase->mapcMembers.NumElements(); i++)
	{
		pcInherited = pcBase->mapcMembers.GetPtr(i);
		if ((pcInherited->meKind != CPPFK_Constructor) || pcInherited->mpcTemplate || pcInherited->mbImplicit || (pcInherited->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = pcInherited->mapsParameters.GetPtr(0);
		if ((pcInherited->mapsParameters.NumElements() == 1) && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == pcBase))
		{
			continue;
		}

		bSame = false;
		for (j = 0; !bSame && (j < pcClass->mapcMembers.NumElements()); j++)
		{
			pcOwn = pcClass->mapcMembers.GetPtr(j);
			if ((pcOwn->meKind != CPPFK_Constructor) || (pcOwn->mapsParameters.NumElements() != pcInherited->mapsParameters.NumElements()))
			{
				continue;
			}
			//The same signature: const below the top makes another (D(const int*) doesn't stand for B(int*)).
			bSame = pcOwn->TakesParameters(&pcInherited->mapsParameters);
		}
		if (bSame)
		{
			continue;
		}

		pcFunction = mpcUnit->AddFunction(pcClass, pcClass->mlliName, CPPFK_Constructor, &mpcTypes->msVoid);
		for (k = 0; k < pcInherited->mapsParameters.NumElements(); k++)
		{
			pcFunction->mapsParameters.Add(pcInherited->mapsParameters.GetPtr(k));
			pcFunction->mapcDefaults.Add(pcInherited->mapcDefaults.GetPtr(k));
		}
		pcFunction->mbExplicit = pcInherited->mbExplicit;
		pcFunction->mbConstexpr = pcInherited->mbConstexpr;
		pcFunction->mbImplicit = true;
		pcFunction->mbDefined = true;
		pcFunction->mpcInherited = pcInherited;
		bAdded = true;
	}

	if (bAdded && !pcClass->mbConstructors)
	{
		pcFunction = mpcUnit->GetFunctionDeclarer()->AddImplicit(pcClass, pcClass->mlliName, CPPFK_Constructor, &mpcTypes->msVoid, true);
		pcClass->mbConstructors = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A member's type named by a base's name alone (A in class C : B { A* p; })
// is that base's injected class name, found in the class as a member of
// the base: it must be reachable from the class, which a private base of a
// base isn't.  False (a class error) if it isn't.  [class.access.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::InjectedNameReached(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, SCPP65Type* psType, CCPPSyntax* pcNode)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*		pcNamed;
	bool			bReached;

	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_NamedTypeSpecifier))
	{
		return true;
	}
	pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName;
	while (psType && (psType->IsPointer() || psType->IsReference() || psType->IsArray()))
	{
		psType = psType->psBase;
	}
	if (!pcName || !pcName->IsSimple() || !psType || !psType->IsClass())
	{
		return true;
	}
	pcNamed = psType->pcClass;
	if ((pcNamed == pcClass) || (mpcUnit->GetComponentName(pcName->GetLast()) != pcNamed->mlliName) || !IsDerived(pcClass, pcNamed, NULL))
	{
		return true;
	}
	mpcDiagnostics->BeginSilent();
	bReached = mpcUnit->GetBinder()->ReachesBase(pcClass, pcNamed, pcNode);
	mpcDiagnostics->EndSilent();
	return bReached || mpcDiagnostics->Defer(pcNode->miFirstToken, "The class's name here is its injected name, found through a base that can't be reached from here: write ::Name");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction, which a member of pcClass's definition declares, was
// declared by another of its members before: it was a member of the class
// (one of its first uiMembers) declared in the class, not outside it.  A
// member is declared once in its class (a function declared there may be
// defined outside it).  [class.mem] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsDeclaredAgainInClass(CCPPClass* pcClass, CCPPFunction* pcFunction, size uiMembers)
{
	CCPPSyntax*	pcNode;
	size		i;

	if (!pcFunction->mpcDeclarator || (pcFunction->mpcClass != pcClass) || pcFunction->mbImplicit)
	{
		return false;
	}
	for (i = 0; (i < uiMembers) && (pcClass->mapcMembers.GetPtr(i) != pcFunction); i++)
	{
	}
	if (i == uiMembers)
	{
		return false;
	}
	for (pcNode = pcFunction->mpcDeclarator->mpcParent; pcNode; pcNode = pcNode->mpcParent)
	{
		if (pcNode->GetKind() == CPPS_ClassSpecifier)
		{
			return pcNode == pcClass->mpcSpecifier;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A class as it was laid out, as a line of maszLayouts (SetLayouts): its
// name, where it is defined, its size, its alignment and the most alignas
// asks of it, then where each base, the vtable pointer and each field is
// ("name@offset", a bit-field's first bit after a '.').
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::AddLayout(CCPPClass* pcClass)
{
	CChars					sz;
	SPreprocessedToken*		psSource;
	SCPPField*				psField;
	CCPPTypeWriter			cWriter;
	size					i;

	cWriter.Init(mpcUnit, CPPTW_Shown);
	sz.Init();
	cWriter.AppendClass(pcClass, &sz);
	psSource = mpcUnit->GetTokeniser()->GetSource(mpcUnit->GetTokeniser()->Get(pcClass->mpcSpecifier->miFirstToken));
	if (psSource && psSource->pcFile)
	{
		sz.Append(' ')->Append(psSource->pcFile->ShortName())->Append('(')->Append(psSource->iLine + 1)->Append(')');
	}
	sz.Append(" size ")->Append(pcClass->mpsType->iBytes)->Append(" align ")->Append(pcClass->miAlign)->Append(" alignas ")->Append(pcClass->miAlignas);
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		sz.Append(pcClass->macBases.Get(i)->bVirtual ? " virtual " : " base ");
		cWriter.AppendClass(pcClass->macBases.Get(i)->pcClass, &sz);
		if (!pcClass->macBases.Get(i)->bVirtual)
		{
			sz.Append('@')->Append(pcClass->macBases.Get(i)->iOffset);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		sz.Append(" virtual-base ");
		cWriter.AppendClass(pcClass->macVirtualBases.Get(i)->pcClass, &sz);
		sz.Append('@')->Append(pcClass->macVirtualBases.Get(i)->iOffset);
	}
	if (pcClass->mcVtable.HasPointer())
	{
		sz.Append(" vptr@")->Append(pcClass->mcVtable.miPointerOffset);
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		sz.Append(' ');
		cWriter.AppendIdentifier(psField->lliName, &sz);
		sz.Append('@')->Append(psField->iOffset);
		if (psField->IsBitField())
		{
			sz.Append('.')->Append(psField->iBitOffset);
		}
	}
	maszLayouts.Add(&sz);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A bit-field (int iFlags : 3;), or an unnamed one (int : 3;, int : 0;),
// of an integer, bool or enum type, its width a constant no wider than its
// type.  LayoutClass places it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::ClassBitField(CCPPClass* pcClass, CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxInitDeclarator* pcInit)
{
	SCPP65Type*		psType;
	SCPPField*		psField;
	CChars			szMessage;
	int64			lliWidth;
	bool			bResult;

	//A class is laid out once, often first asked about quietly (TypeOf) or by the IR front end, so its own errors are
	//kept to report when the unit ends.
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_STATIC))
	{
		return mpcDiagnostics->Defer(pcInit->miFirstToken, "A bit-field can't be static");
	}
	psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
	if (!psType)
	{
		return false;
	}
	if (!psType->IsInteger())
	{
		return mpcDiagnostics->Defer(pcInit->miFirstToken, "A bit-field must be an integer, bool or enum");
	}
	bResult = true;
	if (!mpcUnit->GetFolder()->Constant(pcInit->mpcBitField, &lliWidth) || (lliWidth < 0))
	{
		szMessage.Init();
		mpcUnit->GetFolder()->ConstantMessage("A bit-field's width must be a constant of 0 or more", &szMessage);
		bResult = mpcDiagnostics->Defer(pcInit->mpcBitField->miFirstToken, szMessage.Text());
		szMessage.Kill();
		lliWidth = 1;
	}
	if (lliWidth > (int64)psType->iBytes * 8)
	{
		bResult = mpcDiagnostics->Defer(pcInit->mpcBitField->miFirstToken, "A bit-field can't be wider than its type");
		lliWidth = psType->iBytes * 8;
	}
	if ((lliWidth == 0) && pcInit->mpcSymbol)
	{
		bResult = mpcDiagnostics->Defer(pcInit->mpcBitField->miFirstToken, "A named bit-field can't have a width of 0");
		lliWidth = 1;
	}
	psField = pcClass->macFields.Add();
	psField->Init(pcInit->mpcSymbol, psType, (int)lliWidth);
	psField->pcInitialiser = pcInit->mpcInitializer;
	psField->bMutable = pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_MUTABLE);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's functions' override and final ([class.virtual] 4 and 5): one
// declared override must override a base class's virtual function, and none
// may override one declared final.  One that overrides may allow only the
// exceptions the function it overrides allows ([except.spec] 5).  One that overrides and is declared
// neither is warned about (mbWarnMissingOverride), but for a destructor, as
// G++'s -Wsuggest-override doesn't.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::CheckOverrides(CCPPClass* pcClass)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcOverridden;
	size			i;
	bool			bResult;

	bResult = true;
	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mbStatic || pcFunction->mbImplicit || !pcFunction->mpcDeclarator || (pcFunction->meKind == CPPFK_Constructor))
		{
			continue;
		}
		pcOverridden = pcClass->FindOverridden(pcFunction);
		if (pcFunction->mbOverride && !pcOverridden)
		{
			bResult &= mpcDiagnostics->Error(pcFunction->mpcDeclarator, "The function is declared override but overrides no virtual function of a base class");
		}
		else if (pcFunction->mbFinal && !pcFunction->mbVirtual && !pcOverridden)
		{
			//[class.mem] 8: override and final are a virtual member function's.
			bResult &= mpcDiagnostics->Error(pcFunction->mpcDeclarator, "The function is declared final but isn't virtual");
		}
		else if (pcOverridden && pcOverridden->mbFinal)
		{
			bResult &= mpcDiagnostics->Error(pcFunction->mpcDeclarator, "The function overrides one declared final");
		}
		else if (pcOverridden && !ReturnsCovariant(pcFunction, pcOverridden))
		{
			//[class.virtual] 7
			bResult &= mpcDiagnostics->Defer(pcFunction->mpcDeclarator->miFirstToken, "The function returns another type than the virtual function it overrides, and not a pointer or reference to a class derived from that one's");
		}
		else if (pcOverridden && !mpcUnit->GetExceptions()->AllowsOnlyWhat(mpcUnit->GetExceptions()->ExceptionsOf(pcFunction), mpcUnit->GetExceptions()->ExceptionsOf(pcOverridden)))
		{
			bResult &= mpcDiagnostics->Error(pcFunction->mpcDeclarator, "The function allows exceptions that the virtual function it overrides doesn't");
		}
		else if (pcOverridden && !pcFunction->mbOverride && !pcFunction->mbFinal && (pcFunction->meKind != CPPFK_Destructor) && mpcUnit->GetOptions()->mbWarnMissingOverride)
		{
			mpcDiagnostics->Warning(pcFunction->mpcDeclarator, "The function overrides a virtual function of a base class but isn't declared override");
		}
	}

	//The destructor the compiler declares allows what the class's parts' destructors do, which may be more.
	pcFunction = pcClass->mpcDestructor;
	pcOverridden = (pcFunction && pcFunction->mbImplicit) ? pcClass->FindOverridden(pcFunction) : NULL;
	if (pcOverridden && !mpcUnit->GetExceptions()->AllowsOnlyWhat(mpcUnit->GetExceptions()->ExceptionsOf(pcFunction), mpcUnit->GetExceptions()->ExceptionsOf(pcOverridden)))
	{
		bResult &= mpcDiagnostics->Error(pcClass->mpcSpecifier, "The class's destructor allows exceptions that the virtual destructor it overrides doesn't");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an overrider's result may be the overridden function's: the same
// type, or a pointer (or a reference of the same kind) to a class derived
// from the class the overridden's points to, no more const or volatile.
// [class.virtual] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::ReturnsCovariant(CCPPFunction* pcOverrider, CCPPFunction* pcOverridden)
{
	SCPP65Type*	psFrom;
	SCPP65Type*	psTo;

	psFrom = pcOverrider->mpsReturn;
	psTo = pcOverridden->mpsReturn;
	if (!psFrom || !psTo || psFrom->Same(psTo) || (psFrom == &mpcTypes->msUndeduced) || (psTo == &mpcTypes->msUndeduced) || pcOverrider->mpcTemplate || pcOverridden->mpcTemplate)
	{
		return true;
	}
	if (!((psFrom->IsPointer() && psTo->IsPointer()) || (psFrom->IsReference() && psTo->IsReference() && (psFrom->bRvalue == psTo->bRvalue))))
	{
		return false;
	}
	psFrom = psFrom->psBase;
	psTo = psTo->psBase;
	if (!psFrom->IsClass() || !psTo->IsClass() || (psFrom->bConst && !psTo->bConst) || (psFrom->bVolatile && !psTo->bVolatile))
	{
		return false;
	}
	return (psFrom->pcClass == psTo->pcClass) || IsDerived(psFrom->pcClass, psTo->pcClass, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function's declarator says override (or final, lliName).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::HasVirtSpecifier(CCPPSyntaxFunctionSuffix* pcSuffix, int64 lliName)
{
	CCPPToken*	pcToken;
	size		i;

	for (i = 0; pcSuffix && (i < pcSuffix->macpVirtSpecifiers.NumElements()); i++)
	{
		pcToken = mpcUnit->GetTokeniser()->Get(pcSuffix->macpVirtSpecifiers.GetPtr(i)->miFirstToken);
		if (pcToken && pcToken->IsIdentifier() && (((CCTIdentifier*)pcToken)->miIdentifier == lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A class written in a source file (IsInSourceFile) with a member function
// declared override or final, or with a base, is laid out where it is
// defined, so what final and override say is checked whether or not
// anything uses the class (CheckOverrides: one that overrides nothing, one
// that isn't virtual, one that overrides a final function; ResolveBases: a
// base that is final); any other class is laid out when it is first needed
// whole, and one nothing uses never is.
//
// A header's class is not: it is checked in the unit that first needs it
// whole.  Every unit parses each class its headers so much as name, and
// laying all of those out in every unit was half the time of the
// declarations (DeclarationsSpeedPlan.txt).  Nor is any class when the unit
// was compiled before, which found what there is to find
// (SetCompiledBefore).
// [class.virtual] 4, 5, [class] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::CheckVirtSpecifiers(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*						pcClass;
	CCPPSyntax*						pcMember;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	size							i;
	bool							bResult;

	if (mpcUnit->IsCompiledBefore() || !pcSpecifier->mpcSymbol || (pcSpecifier->mpcSymbol->meKind != CPPSK_Class) || !mpcUnit->GetCheck()->IsInSourceFile(pcSpecifier))
	{
		return true;
	}

	//The classes defined in it, in their turn.
	bResult = true;
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		pcMember = pcSpecifier->macpMembers.GetPtr(i);
		pcSpecifiers = (pcMember->GetKind() == CPPS_SimpleDeclaration) ? ((CCPPSyntaxSimpleDeclaration*)pcMember)->mpcSpecifiers : NULL;
		if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			bResult &= CheckVirtSpecifiers((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType);
		}
	}
	if (!bResult || !(pcSpecifier->HasVirtSpecifiedMember() || (pcSpecifier->macpBases.NumElements() > 0)))
	{
		return bResult;
	}
	pcClass = GetClass(pcSpecifier->mpcSymbol, pcSpecifier);
	if (!pcClass->IsComplete() && !pcClass->IsBeingMade())
	{
		mpcUnit->GetStatistics()->Count(CPPCC_ClassLayoutsWhereDefined);
	}
	return CompleteClass(pcClass, pcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// What an object of a type is aligned to, which is what alignof says: the
// type's alignment, unless the target aligns objects to less
// (CCPPTarget::GetObjectAlignmentLimit).  [basic.align] 2, [expr.alignof]
//
//////////////////////////////////////////////////////////////////////////
int CCPPClasses::ObjectAlignment(SCPP65Type* psType)
{
	return CCPPClassLayout::LimitedAlignment(psType, mpcUnit->GetTarget()->GetObjectAlignmentLimit());
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's alignment: its type's, or more where alignas asks it on the
// declaration or of the type's class.  *piAlignas is what alignas asks (0
// for nothing), which a place in a frame may not give.  [basic.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPPClasses::VariableAlignment(CCPPVariable* pcVariable, int* piAlignas)
{
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	int								iAlign;
	int								iAlignas;
	int								iDeclared;

	iAlign = ObjectAlignment(pcVariable->mpsType);
	iAlignas = CCPPClassLayout::TypeAlignas(pcVariable->mpsType);
	pcInit = mpcUnit->GetInstantiator()->VariableDeclarator(pcVariable);
	if (pcInit)
	{
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && pcDeclaration->mpcSpecifiers)
		{
			iDeclared = AlignasAlignment(&pcDeclaration->mpcSpecifiers->macpSpecifiers);
			iAlignas = (iDeclared > iAlignas) ? iDeclared : iAlignas;
		}
		iDeclared = AlignasAlignment(&pcInit->mpcDeclarator->macpAttributes);
		iAlignas = (iDeclared > iAlignas) ? iDeclared : iAlignas;
	}
	*piAlignas = iAlignas;
	return (iAlignas > iAlign) ? iAlignas : iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// The strictest alignment the alignas(n) and alignas(type) specifiers in
// papcSpecifiers ask for: 0 for none (alignas(0) asks for nothing), -1 after
// an error.  It is kept whatever #pragma pack says, as MSVC does.  A class's
// (bClass) errors are deferred, as it is laid out once.  [dcl.align]
//
//////////////////////////////////////////////////////////////////////////
int CCPPClasses::AlignasAlignment(CArraySyntax* papcSpecifiers, bool bClass)
{
	CCPPSyntax*		pcSpecifier;
	CCPPSyntax*		pcAlignment;
	SCPP65Type*		psType;
	int64			lliAlign;
	int				iAlign;
	size			i;

	iAlign = 0;
	for (i = 0; i < papcSpecifiers->NumElements(); i++)
	{
		pcSpecifier = papcSpecifiers->GetPtr(i);
		if (pcSpecifier->GetKind() != CPPS_AlignasSpecifier)
		{
			continue;
		}
		pcAlignment = ((CCPPSyntaxAlignasSpecifier*)pcSpecifier)->mpcAlignment;
		if (pcAlignment->GetKind() == CPPS_TypeId)
		{
			psType = mpcDeclarators->GetTypeIdType((CCPPSyntaxTypeId*)pcAlignment);
			if (!psType || (psType->IsClass() && !CompleteClass(psType->pcClass, pcAlignment)))
			{
				return -1;
			}
			lliAlign = ObjectAlignment(psType);
		}
		else if (!mpcUnit->GetFolder()->Constant(pcAlignment, &lliAlign))
		{
			mpcUnit->GetFolder()->ConstantError(pcAlignment, "The alignment must be a constant");
			return -1;
		}
		if ((lliAlign < 0) || (lliAlign > 0x10000) || ((lliAlign & (lliAlign - 1)) != 0))
		{
			if (bClass)
			{
				mpcDiagnostics->Defer(pcAlignment->miFirstToken, "The alignment must be a power of two");
			}
			else
			{
				//(A variable's alignment is asked for quietly too, and more than once.)
				mpcDiagnostics->ReportOnce(pcAlignment, "The alignment must be a power of two");
			}
			return -1;
		}
		if (lliAlign > iAlign)
		{
			iAlign = (int)lliAlign;
		}
	}
	return iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// A const integer static data member initialised in its class (static const int
// Max = 100;), or a constexpr float one: where its value is used it is a
// constant, and needs no definition.  Its address, or a reference to it, needs
// one.  [class.static.data] 3, 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsConstantMember(CCPPVariable* pcVariable)
{
	SCPP65Type*	psType;

	if (!pcVariable || !pcVariable->mpcClass || !pcVariable->mpcInClassInitialiser)
	{
		return false;
	}
	psType = pcVariable->mpsType;
	return psType && psType->bConst && (psType->IsInteger() || psType->IsFloat());
}


//////////////////////////////////////////////////////////////////////////
//
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPClasses::FindStatic(CCPPClass* pcClass, int64 lliName)
{
	CCPPVariable*	pcVariable;
	size			i;

	for (i = 0; i < mpcUnit->GetGlobalVariables()->NumElements(); i++)
	{
		pcVariable = mpcUnit->GetGlobalVariables()->GetPtr(i);
		if ((pcVariable->mpcClass == pcClass) && (pcVariable->mpcSymbol->mlliName == lliName))
		{
			return pcVariable;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcVariable = FindStatic(pcClass->macBases.Get(i)->pcClass, lliName);
		if (pcVariable)
		{
			return pcVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the pointer on the left of ->: the object's operator->'s result when
// it is an object of a class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPClasses::ArrowType(CCPPSyntax* pcObject, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	CCPPFunction*	pcFunction;
	int				i;

	psType = mpcUnit->GetTypeChecker()->TypeOf(pcObject);
	if (!psType || !psType->IsClass())
	{
		return psType;
	}
	if (!CompleteClass(psType->pcClass, pcNode))
	{
		return NULL;
	}
	pcFunction = mpcUnit->GetBinder()->FindArrowOperator(psType->pcClass, pcObject, pcNode);
	psType = pcFunction ? pcFunction->mpsReturn : NULL;
	psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	for (i = 0; psType && psType->IsClass() && (i < CPP_MAX_ARROWS); i++)
	{
		//An operator-> that returns an object, or a reference to one: its operator->'s.  [over.ref] 1
		pcFunction = CompleteClass(psType->pcClass, pcNode) ? mpcUnit->GetBinder()->FindArrowOperator(psType->pcClass, NULL, pcNode) : NULL;
		psType = pcFunction ? pcFunction->mpsReturn : NULL;
		psType = (psType && psType->IsReference() && psType->psBase->IsClass()) ? psType->psBase : psType;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a class is complete where pcNode is: its definition is before
// pcNode, or pcNode is in it, or in a class it is defined in (whose member
// function bodies see the class complete).  A class is laid out when it is
// first needed whole, wherever that is; this says whether the program may
// need it there.  What a template instance or a template's code uses is
// complete where the instance is made, so is taken to be.  [basic.types] 5,
// [class.mem] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsDefinedBefore(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPSyntaxClassSpecifier*	pcSpecifier;
	CCPPSyntax*					pcIn;

	pcSpecifier = pcClass->mpcSpecifier;
	if (!pcSpecifier || !pcNode || (pcNode->miFirstToken < 0) || (pcSpecifier->miFirstToken < 0) || pcClass->mpcTemplate || pcClass->mpcBindings || mpcContext->GetBindings())
	{
		return true;
	}
	if (pcSpecifier->miFirstToken < pcNode->miFirstToken)
	{
		return true;
	}
	for (pcIn = pcNode->mpcParent; pcIn; pcIn = pcIn->mpcParent)
	{
		if ((pcIn->GetKind() == CPPS_ClassSpecifier) && (pcIn->miFirstToken <= pcSpecifier->miFirstToken) && (pcIn->miLastToken >= pcSpecifier->miLastToken))
		{
			return true;
		}
	}
	return false;
}
