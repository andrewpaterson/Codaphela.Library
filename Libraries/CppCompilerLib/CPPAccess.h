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
#ifndef __C_P_P_ACCESS_H__
#define __C_P_P_ACCESS_H__
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPFunction.h"
#include "CPPClass.h"


class CCPPUnit;
class CCPPDiagnostics;


//Says whether a class's member may be named where it is named: private, protected and public, friends, and the
//access of the bases it is reached through.  [class.access]
//
//Nothing is kept to say it with.  What a member's access is, and which class it is a member of, is where its
//declaration is in the class's definition: after which of public:, protected: and private:, or the class's own
//(private for class, public for struct and union).  A class's friends are the friend declarations among its
//members.  Where a name is used is the syntax it is in (a class's definition, for what is written in the class)
//and the function being written (for a member function defined outside its class, and a friend function).
//
//A member is named as a member of a class (pcNaming): the class of the object it is taken from, the class that
//qualifies its name, or the class of the member function it is used in.  It may be named if, as a member of that
//class, it is public; or private, and the use is in a member or friend of that class; or protected, and the use
//is in a member or friend of that class or of a class derived from it; or if it may be named as a member of a base
//of that class which the use may itself reach.  [class.access.base] 5
//
//The instances of a class template are one class here (they have one definition), so an instance may name the
//private members of another: more than the language allows.  Nor is a protected member refused for being taken
//from an object of a base class ([class.protected]).
//
//That a member may not be named is an error where it is, whatever is being tried around it (it doesn't change
//what the name means), and is reported once for each place.
class CCPPAccess
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.

public:
	void	Init(CCPPUnit* pcUnit);

	bool	CheckFunction(CCPPFunction* pcFunction, CCPPClass* pcNaming, CCPPSyntax* pcNode);
	bool	CheckSymbol(CCPPSymbol* pcSymbol, CCPPClass* pcNaming, CCPPSyntax* pcNode);
	bool	Check(CCPPSyntax* pcDeclaration, CCPPClass* pcNaming, CCPPSyntax* pcNode);
	bool	IsThroughDerived(CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier* pcDeclaring, CCPPClass* pcNaming, CCPPSyntax* pcNode);
	bool	CheckBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode);
	bool	CheckUsing(CCPPSymbol* pcSymbol, CCPPSyntax* pcUsing);
	bool	CheckProtectedObject(CCPPSymbol* pcSymbol, CCPPClass* pcObjectClass, CCPPSyntax* pcNode);
	bool	HasOnlyPublicFields(CCPPClass* pcClass);

protected:
	int		DeclaredAccess(CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier** ppcClass);
	CCPPSyntax*	FindUsingDeclaration(CCPPClass* pcClass, CCPPSyntax* pcDeclaration, CCPPSyntaxClassSpecifier* pcDeclaring);
	int		AccessAs(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcDeclaring, int iDeclared);
	bool	IsAccessible(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcDeclaring, int iDeclared, CCPPSyntax* pcNode);
	bool	IsAllowed(int iAccess, CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode);
	bool	IsInClass(CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode);
	bool	IsInMemberOf(CCPPSymbol* pcClass, CCPPSyntax* pcNode);
	bool	IsInFriendOf(CCPPSyntaxClassSpecifier* pcClass, CCPPSyntax* pcNode);
	bool	IsInDerivedFrom(CCPPSyntaxClassSpecifier* pcClass);
	bool	IsFriendFunction(CCPPSyntaxDeclarator* pcDeclarator);
	bool	HasBase(CCPPClass* pcClass, CCPPSyntaxClassSpecifier* pcBase);
};


#endif // __C_P_P_ACCESS_H__

