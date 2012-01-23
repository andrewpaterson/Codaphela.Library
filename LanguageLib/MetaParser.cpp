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
#include "MetaParser.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/IntegerHelper.h"
#include "ElementTypes.h"
#include "BaseLib/Logger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaParser::Init(CMeta* pcMeta)
{
	mpcParser = NULL;
	mpcMeta = pcMeta;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::Init(CMeta* pcMeta, char* szLanguageDefinition)
{
	Init(pcMeta);
	return Parse(szLanguageDefinition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaParser::Kill(void)
{
	mpcMeta = NULL;
	mpcParser = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::Parse(char* szLanguageDefinition)
{
	CTextParser		cTextParser;
	CDefinition*	pcDefinition;
	int				iNumComplete;
	TRISTATE		tResult;
	int				iOldNumComplete;
	CChars			szErrors;

	mpcParser = &cTextParser;
	cTextParser.Init(szLanguageDefinition);
	CreateDefinitionHolders();

	iOldNumComplete = 0;
	for (;;)
	{
		iNumComplete = 0;
		for (int i = 0; i < mpcMeta->NumDefinitions(); i++)
		{
			pcDefinition = mpcMeta->GetDefinition(i);
			if (!pcDefinition->mbComplete)
			{
				mcCurrentDefinition.Init(pcDefinition->mszName.Text(), pcDefinition->miNumber);
				tResult = ParseDefinition(pcDefinition);
				if (tResult == TRITRUE)
				{
					pcDefinition->Copy(&mcCurrentDefinition);
					pcDefinition->mbComplete = TRUE;
				}
				mcCurrentDefinition.Kill();
			}
			if (pcDefinition->mbComplete)
			{
				iNumComplete++;
			}
		}
		if (iNumComplete == mpcMeta->NumDefinitions())
		{
			cTextParser.Kill();
			return TRITRUE;
		}

		if (iNumComplete == iOldNumComplete)
		{
			szErrors.Init();
			GetIncompleteDefinitions(&szErrors);
			gcLogger.Error(szErrors.Text());
			szErrors.Kill();
			cTextParser.Kill();
			return TRIERROR;
		}
		iOldNumComplete = iNumComplete;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::CreateDefinitionHolders(void)
{
	char			szIdentifier[1024];
	CDefinition*	pcDefinition;
	int				i;

    for (i = 0;; i++)
	{
		BreakOnFalse(mpcParser->FindExactCharacterSequence("$"));
		mpcParser->GetExactCharacter('$');

		//The $ must be followed by an identifier.
		ReturnErrorOnErrorAndFalse(mpcParser->GetIdentifier(szIdentifier));

		pcDefinition = mpcMeta->AddDefintion();
		pcDefinition->Init(szIdentifier, i);
		mpcParser->SaveState(&pcDefinition->msParserState);
	}

	//The beginning of the language definition file is the main body.
	pcDefinition = mpcMeta->AddDefintion();
	pcDefinition->Init("", i);
	mpcParser->Restart();
	mpcParser->SaveState(&pcDefinition->msParserState);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseDefinition(CDefinition* pcDefinition)
{
	TRISTATE			tResult;
	EElement			eNodeType;
	int					iNodeDepth;
	int					iDeepNodes;
	CNodeConstructor	cNode;

	mpcParser->LoadState(&pcDefinition->msParserState);

	iDeepNodes = 0;
	for (int i = 0;; i++)
	{
		iNodeDepth = 0;
		tResult = ParseNode(&iNodeDepth);
		if (iNodeDepth > 0)
		{
			iDeepNodes++;
		}
		ContinueOnTrue(tResult);
		if (tResult == TRIERROR)
		{
			i++;
		}
		if ((mpcParser->mbOutsideText) || (mpcParser->GetExactCharacter('$')))
		{
			if (i == 0)
			{
				gcLogger.Error("Can't have empty definition");
				return TRIERROR;
			}

			if (i == 1)
			{
				eNodeType = NT_Single;
			}
			else
			{
				eNodeType = NT_Ordered;
			}
			if ((i != 1) || ((i == 1) && (iDeepNodes != 1)))
			{
				cNode.Init();
				cNode.SetNumElementsInNode(i);
				cNode.SetType(eNodeType);
				cNode.SetName(pcDefinition->mszName.Text());
				mcCurrentDefinition.AddNode(0, &cNode);
				cNode.Kill();
			}
			else
			{
				//I have a bad feeling about this one...
				mcCurrentDefinition.SetFirstNodeName(pcDefinition->mszName.Text());
			}
			return TRITRUE;
		}
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseNode(int* piNodeDepth)
{
	TRISTATE tResult;
	EElement eInverse;

	eInverse = NM_Normal;
	tResult = mpcParser->GetExactCharacter('!');
	if (tResult == TRITRUE)
	{
		eInverse = NM_Inverse;
	}
	ReturnOnError(tResult);

	tResult = ParseSimpleNode(eInverse);
	ReturnOnErrorAndTrue(tResult);
	tResult = ParseCompoundNode(eInverse, piNodeDepth);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseCompoundNode(EElement eInverse, int* piNodeDepth)
{
	TRISTATE			tResult;
	int					iPos;
	CNodeConstructor	cNode;

	ReturnOnErrorAndFalse(mpcParser->GetExactCharacter('('));
	
	iPos = mcCurrentDefinition.NextElementPosition();
	cNode.Init();
	for (int i = 0;; i++)
	{
		tResult = mpcParser->GetExactCharacter(')');
		ReturnOnError(tResult);

		cNode.SetNumElementsInNode(i);

		if (tResult == TRITRUE)
		{
			tResult = ParseEndOfNode(&cNode);
			if (cNode.ShouldBeAdded())
			{
				mcCurrentDefinition.AddNode(iPos, &cNode);
				(*piNodeDepth)++;
			}
			cNode.Kill();
			return tResult;
		}

		tResult = ParseCompoundNodeOptions(&cNode);
		if (tResult == TRIERROR)
		{
			cNode.Kill();
			return tResult;
		}

		tResult = ParseNode(piNodeDepth);
		if (tResult == TRITRUE)
		{
			cNode.SetOrderedIfPossible();
			continue;
		}
		cNode.Kill();
		return tResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseCompoundNodeOptions(CNodeConstructor* pcNode)
{
	TRISTATE	tResult;
	char		c;

	tResult = mpcParser->GetEnumeratedCharacter(",|&", &c);
	ReturnOnErrorAndFalse(tResult);
	if (tResult == TRITRUE)
	{
		if (pcNode->msNode.iNumElementsInNode == 0)
		{
			gcLogger.Error("Can't begin node with one of \",|&\"");
			return TRIERROR;
		}
		else if (c == '|')
		{
			ReturnErrorOnFalse(pcNode->SetType(NT_Exclusive));
		}
		else if (c == '&')
		{
			ReturnErrorOnFalse(pcNode->SetType(NT_Inclusive));
		}
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseSimpleNode(EElement eInverse)
{
	TRISTATE			tResult;
	int					iPos;
	CNodeConstructor	cNode;

	cNode.Init();
	iPos = mcCurrentDefinition.NextElementPosition();
	for (;;)
	{
		tResult = ParseString();
		ReturnOnErrorBreakOnTrue(tResult);

		tResult = ParseCharacter();
		ReturnOnErrorBreakOnTrue(tResult);

		tResult = ParseKnownDefinition();
		ReturnOnErrorBreakOnTrue(tResult);

		tResult = ParseUnknownIdentifier();
		ReturnOnErrorBreakOnTrue(tResult);

		tResult = ParseKeyword();
		ReturnOnErrorBreakOnTrue(tResult);

		tResult = ParseEnd();
		ReturnOnErrorBreakOnTrue(tResult);

		return TRIFALSE;
	}
    
	tResult = ParseEndOfNode(&cNode);
	if (cNode.ShouldBeAdded())
	{
		cNode.SetNumElementsInNode(1);
		mcCurrentDefinition.AddNode(iPos, &cNode);
	}
	cNode.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseEnd(void)
{
	ReturnOnErrorAndFalse(mpcParser->GetExactIdentifier("END"));
	mcCurrentDefinition.AddEnd();
	return TRITRUE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseUnknownIdentifier(void)
{
	ReturnOnErrorAndFalse(mpcParser->GetExactIdentifier("identifier"));
	mcCurrentDefinition.AddUnknownIdentifier();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseKeyword(void)
{
	TRISTATE	tResult;

	tResult = mpcParser->GetExactIdentifier("integer");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mcCurrentDefinition.AddPrimitiveType(PT_IntegerType);
		return TRITRUE;
	}

	tResult = mpcParser->GetExactIdentifier("real");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mcCurrentDefinition.AddPrimitiveType(PT_RealType);
		return TRITRUE;
	}

	tResult = mpcParser->GetExactIdentifier("string");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mcCurrentDefinition.AddPrimitiveType(PT_StringType);
		return TRITRUE;
	}

	tResult = mpcParser->GetExactIdentifier("char");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mcCurrentDefinition.AddPrimitiveType(PT_CharType);
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseEndOfNode(CNodeConstructor* pcNode)
{
	TRISTATE tResult;

	tResult = ParseEndOfNodeNecessity(pcNode);
	tResult = ParseEndOfNodeRepetition(pcNode);
	tResult = ParseEndOfNodeFinallity(pcNode);
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseEndOfNodeNecessity(CNodeConstructor* pcNode)
{
	TRISTATE	tResult;
	char		szNodeName[2048];
	char		c;

	tResult = mpcParser->GetEnumeratedCharacter("+-", &c);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		if (c == '+')
		{
			pcNode->SetNecessity(NN_Necessary);
		}
		else if (c == '-')
		{
			pcNode->SetNecessity(NN_Empty);
		}

		tResult = mpcParser->GetExactCharacter(c);
		if (tResult != TRITRUE)
		{
			tResult = mpcParser->GetIdentifier(szNodeName);
			ReturnOnError(tResult);
			if (tResult == TRITRUE)
			{
				pcNode->SetName(szNodeName);
			}
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseEndOfNodeRepetition(CNodeConstructor* pcNode)
{
	int			iMin;
	int			iMax;
	TRISTATE	tResult;

	tResult = mpcParser->GetInteger(&iMin);

	pcNode->msNode.iMinimum = 1;
	pcNode->msNode.iMaximum = 1;
	if (tResult == TRIERROR)
	{
		//We've read off the end of the text, but that's allowed because the end of the node is not mandatory.
		return TRIERROR;
	}

	else if (tResult == TRIFALSE)
	{
		return TRITRUE;
	}

	else
	{
		ReturnErrorOnErrorAndFalse(mpcParser->GetExactCharacter('-'));
		pcNode->msNode.iMinimum = iMin;
		tResult = mpcParser->GetExactCharacter('X');
		ReturnOnError(tResult);
		if (tResult == TRITRUE)
		{
			pcNode->msNode.iMaximum = MAXINT;
		}
		else
		{
			tResult = mpcParser->GetInteger(&iMax);
			ReturnOnError(tResult);
			if (tResult == TRITRUE)
			{
				pcNode->msNode.iMaximum = iMax;
			}
		}
		ReturnErrorOnFalse(tResult);
		return TRITRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseEndOfNodeFinallity(CNodeConstructor* pcNode)
{
	TRISTATE	tResult;

	tResult = mpcParser->GetExactCharacter('.');
	if (tResult == TRITRUE)
	{
		pcNode->SetFinal(TRUE);
		return TRITRUE;
	}
	else if (tResult == TRIFALSE)
	{
		pcNode->SetFinal(FALSE);
		return TRITRUE;
	}
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseString(void)
{
	TRISTATE	tResult;
	char		szString[1024];


	tResult = mpcParser->GetString(szString);
	if (tResult == TRITRUE)
	{
		mcCurrentDefinition.AddText(szString);
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseCharacter(void)
{
	char	c;

	ReturnOnErrorAndFalse(mpcParser->GetExactCharacter('\''));
	ReturnOnError(mpcParser->GetCharacter(&c));
	ReturnErrorOnErrorAndFalse(mpcParser->GetExactCharacter('\''));

	mcCurrentDefinition.AddCharacter(c);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaParser::ParseKnownDefinition(void)
{
	CDefinition*	pcDefinition;
	TRISTATE		tResult;

	//NumElements-1 because the last element is always the nameless main body.
	for (int i = 0; i < mpcMeta->NumDefinitions()-1; i++)
	{
		pcDefinition = mpcMeta->GetDefinition(i);
		tResult = mpcParser->GetExactIdentifier(pcDefinition->mszName.Text());
		if (tResult == TRITRUE)
		{
			mcCurrentDefinition.AddDefinition(i);
		}
		ReturnOnErrorAndTrue(tResult);
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaParser::GetIncompleteDefinitions(CChars* sz)
{
	int				i;
	CDefinition*	pcDefinition;

	sz->Append("Incomplete definitions ");
	for (i = 0; i < mpcMeta->NumDefinitions(); i++)
	{
		pcDefinition = mpcMeta->GetDefinition(i);
		if (!pcDefinition->mbComplete)
		{
			sz->Append("[");
			sz->Append(pcDefinition->mszName);
			sz->Append("] ");
		}
	}
}


