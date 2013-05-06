/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __META_TOKENISER_H__
#define __META_TOKENISER_H__
#include "Meta.h"
#include "Definition.h"
#include "TextElement.h"
#include "IdentifierElement.h"
#include "CharElement.h"
#include "NodeElement.h"
#include "TypeElement.h"
#include "DefinitionElement.h"
#include "EndElement.h"
#include "BaseLib/Error.h"
#include "BaseLib/AdditionalTypes.h"
#include "BaseLib/LinkListTemplate.h"
#include "BaseLib/EnumeratorVoid.h"
#include "BaseLib/ArrayInt.h"
#include "TokenTree.h"


class CTokeniserError
{
public:
	char*			mpsPosition;
	int				miLine;
	int				miColumn;

	EErrorSeverity	eSeverity;
	CChars			szMessage;

	void Init(void);
	void Kill(void);
};


class CEnumeratorID
{
public:
	int					iID;
	CEnumeratorVoid		cIdentifierNames;

	void	Init(int iID); 
	void	Kill(void);
};


typedef CLinkListTemplate<CEnumeratorID>	CIdentifierIDs;
typedef CArrayTemplate<CTokeniserError>		CTokeniserErrors;

class CMetaTokeniser
{
public:
	CMeta*				mpcMeta;
	CTextParser*		mpcParser;
	CIdentifierIDs		mcIdentifierIDs;
	CArrayInt			mcEmptyNodes;
    CTokenTree*			mpcTokenTree;
	CTokeniserErrors	mcErrors;

	void Init(void);
	void Kill(void);

	TRISTATE		Tokenise(CTokenTree* pcTokenTree, CMeta* pcMeta, char* szText);
	TRISTATE		TokeniseDefinition(CDefinition* pcDefinition, int* iNumElements, int iNodeDepth);
	TRISTATE 		TokeniseElement(CDefinition* pcDefinition, int iNextElement, int* iNewElement, int* iNumElements, EElement* peInverseNode, int iNodeDepth);
	TRISTATE 		TokeniseNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* iNumElements, int iNodeDepth);
	TRISTATE 		TokeniseNodeInternal(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth);
	TRISTATE 		TokeniseOrderedNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth);
	TRISTATE 		TokeniseExclusiveNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth);
	TRISTATE 		TokeniseInclusiveNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth);

	TRISTATE 		ParseText(STextElement* psTextElement, int iNodeDepth);
	TRISTATE 		ParseChar(SCharElement* psCharElement, int iNodeDepth);
	TRISTATE 		ParseIdentifier(SIdentifierElement* psIdentifierElement, int iNodeDepth);
	TRISTATE 		ParsePrimitiveType(STypeElement* pvPrimitiveElement, int iNodeDepth);
	TRISTATE 		ParseInteger(int iNodeDepth);
	TRISTATE 		ParseReal(int iNodeDepth);
	TRISTATE 		ParseString(int iNodeDepth);
	TRISTATE		ParseChar(int iNodeDepth);
	TRISTATE		ParseEnd(int iNodeDepth);

	TRISTATE 		GetNextElementPosition(CDefinition* pcDefinition, int iNextElement, int* piNewElementPosition);
	int				GetElementSize(int* pvElement);
	int*			GetNextElement(int* pvElement);

	void			GenerateUnexpectedTokenErrorMessage(void);
	void			GenerateNodeExpectation(CChars* sz, SNodeElement* psNode);
	void			RecurseElement(CChars* sz, int* pvElement);
	void			RollForward(void);

	void			Output(char* szText1, char* szText2, char* szText3, char* szText4, char pad, int iNodeDepth);
	void			OutputNodeCount(SNodeElement* psNodeElement, int iNodes, TRISTATE tResult, int iNodeDepth);
	void			OutputNodeContents(char* szMessage, SParseState* psPosition, SNodeElement* psNodeElement, int iNodeDepth);
};


#endif //__META_TOKENISER_H__

