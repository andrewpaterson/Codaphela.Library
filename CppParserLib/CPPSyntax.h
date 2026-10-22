/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_SYNTAX_H__
#define __C_P_P_SYNTAX_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CppTokens.h"
#include "CPPSyntaxKind.h"
#include "CPPSyntaxEnums.h"


class CCPPSyntax;
class CCPPSyntaxTree;
class CCPPScope;
class CCPPSymbol;


typedef CArrayTemplatePtr<CCPPSyntax>	CArraySyntax;


//A node of the abstract syntax tree.  Nodes know the tokens they were parsed from, so they can be found in the source.
class CCPPSyntax
{
public:
	CCPPSyntaxTree*		mpcTree;
	CCPPSyntax*			mpcParent;
	int32				miFirstToken;
	int32				miLastToken;		//Inclusive.

	virtual void		Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent);
	virtual void		Kill(void);

	virtual char*		GetType(void) =0;
	virtual ECPPSyntax	GetKind(void) =0;
	virtual void		PrettyPrint(CChars* pszDest, int iDepth) =0;
	virtual void		TypePrint(CChars* pszDest, int iDepth);
	virtual size		NumChildren(void) =0;
	virtual CCPPSyntax*	GetChild(size iIndex) =0;

	virtual bool		IsDeclaration(void);
	virtual bool		IsStatement(void);
	virtual bool		IsExpression(void);
	virtual bool		IsTypeSpecifier(void);

			void		SetTokens(int iFirstToken, int iLastToken);
			void		Dump(void);

protected:
			void		Indent(CChars* pszDest, int iDepth);
			void		AppendToken(CChars* pszDest, int iToken);
			void		AppendTokens(CChars* pszDest, int iFirstToken, int iLastToken);
			void		PrintKeyword(CChars* pszDest, ECppTokens eToken);
			void		PrintNode(CChars* pszDest, CCPPSyntax* pcNode, int iDepth);
			void		PrintList(CChars* pszDest, CArraySyntax* papcNodes, char* szSeparator, int iDepth);
			void		PrintDeclarations(CChars* pszDest, CArraySyntax* papcNodes, int iDepth);
			void		PrintStatements(CChars* pszDest, CArraySyntax* papcNodes, int iDepth);
			void		PrintCondition(CChars* pszDest, CCPPSyntax* pcCondition, int iDepth);
			void		PrintSubStatement(CChars* pszDest, CCPPSyntax* pcStatement, int iDepth);
};


#endif // __C_P_P_SYNTAX_H__

