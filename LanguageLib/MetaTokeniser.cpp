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
#include "BaseLib/DebugOutput.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/Logger.h"
#include "MetaTokeniser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniserError::Init(void)
{
	szMessage.Init();
	mpsPosition = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniserError::Kill(void)
{
	mpsPosition = NULL;
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::Init(void)
{
	mpcMeta = NULL;
	mcEmptyNodes.Init(16);
	mpcTokenTree = NULL;
	mpcParser = NULL;
	mcErrors.Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::Kill(void)
{
	int					i;
	CTokeniserError*	pcError;

	for (i = 0; i < mcErrors.NumElements(); i++)
	{
		pcError = mcErrors.Get(i);
		pcError->Kill();
	}

	mcErrors.Kill();
	mpcMeta = NULL;
	mpcTokenTree = NULL;
	SafeKill(mpcParser);
	mcEmptyNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEnumeratorID::Init(int iID)
{
	this->iID = iID;
	cIdentifierNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEnumeratorID::Kill(void)
{
	iID = -1;
	cIdentifierNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::Tokenise(CTokenTree* pcTokenTree, CMeta* pcMeta, char* szText)
{
	CDefinition*	pcDefinition;
	TRISTATE		tResult;

	if ((pcMeta == NULL) || (pcMeta->NumDefinitions() == 0))
	{
		gcLogger.Error("Can't parse without meta information");
		return TRIERROR;
	}

	mpcMeta = pcMeta;
	mpcTokenTree = pcTokenTree;

	mpcParser = (CTextParser*)malloc(sizeof(CTextParser));
	mpcParser->Init(szText);
    
	pcDefinition = (CDefinition*)pcMeta->GetEntryDefinition();

	mpcTokenTree->BeginTree();
	tResult = TokeniseDefinition(pcDefinition, NULL, 0);
	mpcTokenTree->EndTree();

	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseDefinition(CDefinition* pcDefinition, int* iNumElements, int iNodeDepth)
{
	TRISTATE	tResult;
	char*		szName;

	if (!pcDefinition)
	{
		gcLogger.Error("Definition is NULL");
		return TRIERROR;
	}

	szName = pcDefinition->mszName.Text();
	if (!szName) 
	{
		szName = ".main.";
	}

	Output("Definition '", szName, "'", NULL, ' ', iNodeDepth);

	tResult = TokeniseElement(pcDefinition, 0, NULL, iNumElements, NULL, iNodeDepth);
	
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
// 
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseElement(CDefinition* pcDefinition, int iNextElement, int* piNewElement, int* iNumElements, EElement* peModifier, int iNodeDepth)
{
	int				iCurrElement;
	EElement		eElement;
	void*			pvElement;
	CDefinition*	pcDeeperDefinition;
	int				iTemp;
	int				iSam;
	TRISTATE		tResult;
	int				iHeaderSize;

	iTemp = 0;
	iCurrElement = iNextElement;
	eElement = (EElement)(pcDefinition->mcElements).GetValue(iCurrElement);
	pvElement = (void*)(pcDefinition->mcElements).Get(iCurrElement);

	SafeAssign(piNewElement, iNextElement);
	SafeAssign(peModifier, NM_NotANode);

	switch (eElement)
	{
	case ET_Text:
		tResult = ParseText((STextElement*)pvElement, iNodeDepth);
		ReturnOnError(tResult);
		BreakOnFalse(tResult);
		iTemp++;
		break;

	case ET_Identifier:
		tResult = ParseIdentifier((SIdentifierElement*)pvElement, iNodeDepth);  //Why is the element passed in?
		ReturnOnError(tResult);
		BreakOnFalse(tResult);
		iTemp++;
		break;

	case ET_Char:
		tResult = ParseChar((SCharElement*)pvElement, iNodeDepth);
		BreakOnErrorAndFalse(tResult);
		iTemp++;
		break;

	case ET_Node:
		iHeaderSize = ((SNodeElement*)pvElement)->HeaderSize();
		tResult = TokeniseNode(pcDefinition, (SNodeElement*)pvElement, iCurrElement + iHeaderSize, &iSam, iNodeDepth);
		ReturnOnError(tResult);
		SafeAssign(peModifier, ((SNodeElement*)pvElement)->sNode.eModifier);
		BreakOnFalse(tResult);
		iTemp += iSam;
		break;

	case ET_Definition:
		pcDeeperDefinition = mpcMeta->GetDefinition(((SDefinitionElement*)pvElement)->iDefinitionNum);
		tResult = TokeniseDefinition(pcDeeperDefinition, &iSam, iNodeDepth);
		ReturnOnError(tResult);
		BreakOnFalse(tResult);
		iTemp += iSam;
		break;

	case ET_PrimitiveType:
		tResult = ParsePrimitiveType((STypeElement*)pvElement, iNodeDepth);
		ReturnOnError(tResult);
		BreakOnFalse(tResult);
		iTemp++;
		break;

	case ET_End:
		tResult = ParseEnd(iNodeDepth);
		break;

	default:
		gcLogger.Error("Found an unknown token type");
		tResult = TRIERROR;
		break;
	}

	if ((tResult == TRIFALSE) || (tResult == TRIERROR))
	{
		iTemp = 0;
	}

	iTemp = FixBool(iTemp);
	SafeAssign(iNumElements, iTemp);
	ReturnOnErrorAndFalse(tResult);

	GetNextElementPosition(pcDefinition, iNextElement, piNewElement);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CMetaTokeniser::GetNextElement(int* pvElement)
{
	int				iSize;

	iSize = GetElementSize(pvElement);
	if (iSize == 0)
	{
		return NULL;
	}
	return (int*)RemapSinglePointer(pvElement, iSize*4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::GetNextElementPosition(CDefinition* pcDefinition, int iNextElement, int* piNewElementPosition)
{
	int*			pvElement;
	int				iSize;

	pvElement = (pcDefinition->mcElements).Get(iNextElement);
	iSize = GetElementSize(pvElement);
	if (iSize == 0)
	{
		return TRIERROR;
	}
	iNextElement += iSize;
	SafeAssign(piNewElementPosition, iNextElement);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMetaTokeniser::GetElementSize(int* pvElement)
{
	EElement		eElement;

	if (pvElement == NULL)
	{
		return 0;
	}

	eElement = (EElement)(*pvElement);
	switch (eElement)
	{
	case ET_Text:
		return ((STextElement*)pvElement)->Size();

	case ET_Identifier:
		return ((SIdentifierElement*)pvElement)->Size();

	case ET_Char:
		return ((SCharElement*)pvElement)->Size();

	case ET_Node:
		return ((SNodeElement*)pvElement)->Size();

	case ET_Definition:
		return ((SDefinitionElement*)pvElement)->Size();

	case ET_PrimitiveType:
		return ((STypeElement*)pvElement)->Size();

	case ET_End:
		return ((SEndElement*)pvElement)->Size();

	default:
		 return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseOrderedNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth)
{
	TRISTATE		tResult;
	int				iNewElement;
	int				i;
	int				iNodeFound;
	char*			szOldParserPos;

	*piNumElements = 0;
	for (i = 0; i < psNodeElement->sNode.iNumElementsInNode; i++)
	{
		szOldParserPos = mpcParser->mszParserPos;
		tResult = TokeniseElement(pcDefinition, iNextElement, &iNewElement, &iNodeFound, NULL, iNodeDepth);
		ReturnOnError(tResult);
		if (tResult != TRITRUE)
		{
			*piNumElements = 0;
			return tResult;
		}

		Output("Ordered Node got element", NULL, NULL, NULL, ' ', iNodeDepth);

		iNextElement = iNewElement;
	}
	*piNumElements = 1;
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseExclusiveNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth)
{
	TRISTATE		tResult;
	int				i;
	int				iNodeFound;
	SParseState	sTextPosition;
	BOOL			bInnerEmptyNode;

	mpcParser->SaveState(&sTextPosition);
	bInnerEmptyNode = FALSE;
	for (i = 0; i < psNodeElement->sNode.iNumElementsInNode; i++)
	{
		tResult = TokeniseElement(pcDefinition, iNextElement, NULL, &iNodeFound, NULL, iNodeDepth);
		if (tResult == TRIERROR)
		{
			sTextPosition.Kill();
			return TRIERROR;
		}
		if (tResult == TRITRUE)
		{
			if (iNodeFound == 0)
			{
				//Try and get something better than nothing.
				bInnerEmptyNode = TRUE;
			}
			else
			{
				Output("Exclusive Node got non-empty element", NULL, NULL, NULL, ' ', iNodeDepth);

				sTextPosition.Kill();
				*piNumElements = 1;
				return TRITRUE;
			}
		}
		mpcParser->LoadState(&sTextPosition);
		GetNextElementPosition(pcDefinition, iNextElement, &iNextElement);
	}

	sTextPosition.Kill();
	if (bInnerEmptyNode)
	{
		Output("Exclusive Node got empty element", NULL, NULL, NULL, ' ', iNodeDepth);

		//We couldn't match anything but at least one node allowed zero elements.
		//So we can parse this node successfully.
		*piNumElements = 0;
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseInclusiveNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth)
{
	TRISTATE		tResult;
	int				iNumElementsInNode;
	int				i;
	int				iNodeFound;
	SParseState	sTextPosition;
	SParseState	sTruePosition;
	int				iTotalNodes;
	EElement		eModifier;
	int				iNormalNodes;

	mpcParser->SaveState(&sTextPosition);
	mpcParser->SaveState(&sTruePosition);
	iNumElementsInNode = psNodeElement->sNode.iNumElementsInNode;
	iNormalNodes = 0;
	iTotalNodes = 0;
	for (i = 0; i < iNumElementsInNode; i++)
	{
		mpcParser->LoadState(&sTextPosition);  //Should be safe to always do this...

		tResult = TokeniseElement(pcDefinition, iNextElement, NULL, &iNodeFound, &eModifier, iNodeDepth);
		if (tResult != TRITRUE)
		{
			*piNumElements = 0;
			sTextPosition.Kill();
			sTruePosition.Kill();
			return tResult;
		}
		iTotalNodes += iNodeFound;
		if (eModifier != NM_Inverse)
		{
			if (iNormalNodes != 0)
			{
				gcLogger.Error("Inclusive may only have one non-inverse node");
				sTextPosition.Kill();
				sTruePosition.Kill();
				return TRIERROR;
			}
			sTruePosition.Kill();
			mpcParser->SaveState(&sTruePosition);
			iNormalNodes++;
		}
		GetNextElementPosition(pcDefinition, iNextElement, &iNextElement);
	}
	mpcParser->LoadState(&sTruePosition);
	*piNumElements = FixBool(iTotalNodes);
	sTextPosition.Kill();
	sTruePosition.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseNodeInternal(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth)
{
	TRISTATE		tResult;

	switch (psNodeElement->sNode.eNodeType)
	{
	case NT_Single:
	case NT_Ordered:
		tResult = TokeniseOrderedNode(pcDefinition, psNodeElement, iNextElement, piNumElements, iNodeDepth);
		break;

	case NT_Exclusive:
		tResult = TokeniseExclusiveNode(pcDefinition, psNodeElement, iNextElement, piNumElements, iNodeDepth);
		break;

	case NT_Inclusive:
		tResult = TokeniseInclusiveNode(pcDefinition, psNodeElement, iNextElement, piNumElements, iNodeDepth);
		break;

	default:
		gcLogger.Error("Cannot tokenise bad node type");
		return TRIERROR;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::TokeniseNode(CDefinition* pcDefinition, SNodeElement* psNodeElement, int iNextElement, int* piNumElements, int iNodeDepth)
{
	TRISTATE		tResult;
	int				iNodes;
	SParseState	sPosition;
	BOOL			bSucceeded;

	if ((psNodeElement->sNode.eModifier == NM_Inverse) && (!((psNodeElement->sNode.iMinimum == 1) && (psNodeElement->sNode.iMaximum == 1))))
	{
		gcLogger.Error("Inverse node must have max == min == 1");
		return TRIERROR;
	}
	if (psNodeElement->sNode.iMaximum == 0)
	{
		gcLogger.Error("Node cannot have max == 0");
		return TRIERROR;
	}

	if (psNodeElement->sNode.iNumElementsInNode <= 0)
	{
		gcLogger.Error("Node cannot have num elements in node <= 0");
		return TRIERROR;
	}

	Output("{ [", psNodeElement->szName, "]", NULL, ' ', iNodeDepth);

	mpcTokenTree->BeginNode(psNodeElement);

	sPosition.Init();
	for(iNodes = 0;;)
	{
		sPosition.Kill();
		mpcParser->SaveState(&sPosition);
		iNodeDepth++;
		tResult = TokeniseNodeInternal(pcDefinition, psNodeElement, iNextElement, piNumElements, iNodeDepth);
		iNodeDepth--;

		if (tResult == TRIERROR)
		{
			sPosition.Kill();
			Output("} [", psNodeElement->szName, "] ERROR!!!", NULL, ' ', iNodeDepth);
			mpcTokenTree->EndNode(EN_InternalError);
			return TRIERROR;
		}

		//Node count incremented here.
		iNodes++;
		switch (psNodeElement->sNode.eModifier)
		{
		case NM_Normal:
			break;
		case NM_Inverse:
			tResult = Negate(tResult);
			mpcParser->LoadState(&sPosition);
			sPosition.Kill();
			break;
		}

		OutputNodeCount(psNodeElement, iNodes, tResult, iNodeDepth);

		if (tResult == TRITRUE)
		{
			if (iNodes == psNodeElement->sNode.iMaximum)
			{
				mpcTokenTree->EndNode(EN_Parsed);
				OutputNodeContents("Perfect node!", &sPosition, psNodeElement, iNodeDepth);
				sPosition.Kill();
				return TRITRUE;
			}
		}
		if (tResult == TRIFALSE)
		{
			if (iNodes <= psNodeElement->sNode.iMinimum)
			{
				bSucceeded = mpcTokenTree->EndNode(EN_Failed);
				if (!bSucceeded)
				{
					GenerateUnexpectedTokenErrorMessage();
					sPosition.Kill();
					return TRIERROR;
				}
				OutputNodeContents("Too few elements", &sPosition, psNodeElement, iNodeDepth);
				mpcParser->LoadState(&sPosition);
				sPosition.Kill();
				return TRIFALSE;
			}
			else
			{
				mpcTokenTree->EndNode(EN_Parsed);
				OutputNodeContents("Okay node", &sPosition, psNodeElement, iNodeDepth);
				sPosition.Kill();
				return TRITRUE;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::OutputNodeContents(char* szMessage, SParseState* psPosition, SNodeElement* psNodeElement, int iNodeDepth)
{
	CChars	sz;

	sz.Init();
	sz.AppendSubString(psPosition->sCurrent.szParserPos, (int)(mpcParser->mszParserPos - psPosition->sCurrent.szParserPos));
	sz.Replace("\n", "\\n");
	Output(szMessage, " \"", sz.Text(), "\"", ' ', iNodeDepth);
	Output("} [", psNodeElement->szName, "]", NULL, ' ', iNodeDepth);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::OutputNodeCount(SNodeElement* psNodeElement, int iNodes, TRISTATE tResult, int iNodeDepth)
{
	CChars			cText;

	cText.Init();
	cText.Append(iNodes);
	cText.Append(":");
	cText.Append(psNodeElement->sNode.iMinimum);
	cText.Append("-");
	if (psNodeElement->sNode.iMaximum == MAXINT)
	{
		cText.Append("X");
	}
	else
	{
		cText.Append(psNodeElement->sNode.iMaximum);
	}
	cText.Append(" ");
	if (tResult == TRITRUE)
	{
		cText.Append("Passed");
	}
	else if (tResult == TRIFALSE)
	{

		cText.Append("Failed");
	}
	Output(cText.Text(), NULL, NULL, NULL, ' ', iNodeDepth);
	cText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseText(STextElement* psTextElement, int iNodeDepth)
{
	TRISTATE	tResult;

	tResult = mpcParser->GetExactCharacterSequence(psTextElement->szStringCharacters);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddKeywordText(psTextElement->szStringCharacters);
		Output("Text[", psTextElement->szStringCharacters, "]",  NULL, ' ', iNodeDepth);
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseChar(SCharElement* psCharElement, int iNodeDepth)
{
	TRISTATE	tResult;
	char		sz[2];

	tResult = mpcParser->GetExactCharacter(psCharElement->cChar);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddKeywordChar(psCharElement->cChar);
		sz[0] = psCharElement->cChar;
		sz[1] = 0;
		Output("Char[", sz, "]",  NULL, ' ', iNodeDepth);
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseIdentifier(SIdentifierElement* psIdentifierElement, int iNodeDepth)
{
	TRISTATE	tResult;
	char		szIdentifier[1024];

	tResult = mpcParser->GetIdentifier(szIdentifier);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddIdentifier(szIdentifier);
		Output("Identifier[", szIdentifier, "]",  NULL, ' ', iNodeDepth);
		return TRITRUE;
	}

	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParsePrimitiveType(STypeElement* pvPrimitiveElement, int iNodeDepth)
{
	EElement	eType;

	eType = pvPrimitiveElement->ePrimitiveType;
	switch(eType) 
	{
	case PT_IntegerType:
		return ParseInteger(iNodeDepth);
	case PT_RealType:
		return ParseReal(iNodeDepth);
	case PT_StringType:
		return ParseString(iNodeDepth);
	case PT_CharType:
		return ParseChar(iNodeDepth);
	default:
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseInteger(int iNodeDepth)
{
	int			i;
	TRISTATE	tResult;
	CChars		sz;

	tResult = mpcParser->GetInteger(&i);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddInteger(i);
		sz.Init();
		sz.Append(i);
		Output("Integer[", sz.Text(), "]",  NULL, ' ', iNodeDepth);
		sz.Kill();
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseReal(int iNodeDepth)
{
	double		f;
	TRISTATE	tResult;
	CChars		sz;

	tResult = mpcParser->GetFloat(&f);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddReal((float)f);
		sz.Init();
		sz.Append((float)f);
		Output("Real[", sz.Text(), "]",  NULL, ' ', iNodeDepth);
		sz.Kill();
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseString(int iNodeDepth)
{
	char		sz[16384];
	TRISTATE	tResult;

	tResult = mpcParser->GetString(sz);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddUnknownText(sz);
		Output("String[\"", sz, "\"]",  NULL, ' ', iNodeDepth);
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseChar(int iNodeDepth)
{
	char		c;
	TRISTATE	tResult;
	char		sz[2];

	ReturnFalseOnErrorAndFalse(mpcParser->GetExactCharacter('\''));
	ReturnOnErrorAndFalse(mpcParser->GetCharacter(&c));
	tResult = mpcParser->GetExactCharacter('\'', FALSE);
	if (tResult == TRITRUE)
	{
		mpcTokenTree->AddUnknownChar(c);
		sz[0] = c;
		sz[1] = 0;
		Output("Char['", sz, "']",  NULL, ' ', iNodeDepth);
		return TRITRUE;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CMetaTokeniser::ParseEnd(int iNodeDepth)
{
	mpcParser->SkipWhiteSpace();
	if (mpcParser->mbOutsideText)
	{
		Output("End of Text", NULL, NULL, NULL, ' ', iNodeDepth);
		return TRITRUE;
	}
	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::GenerateUnexpectedTokenErrorMessage(void)
{
	SParseState			sPosition;
	char*				szStart;
	CTokeniserError*	pcError;

	pcError = mcErrors.Add();
	pcError->Init();

	pcError->eSeverity = ES_Error;
	pcError->miColumn = mpcParser->Column();
	pcError->miLine = mpcParser->Line();
	pcError->mpsPosition = mpcParser->mszParserPos;

	pcError->szMessage.Append("Unexpected: \"");
	mpcParser->SaveState(&sPosition);
	mpcParser->SkipWhiteSpace();
	szStart = mpcParser->mszParserPos;
	mpcParser->FindWhiteSpace();
	pcError->szMessage.AppendSubString(szStart, mpcParser->mszParserPos);
	pcError->szMessage.Append("\".  Expected: ");
	GenerateNodeExpectation(&pcError->szMessage, mpcTokenTree->mpcCurrentNode->mpsNode);

	
	//EngineOutput(sz.Text());
	//EngineOutput("\n");
	mpcParser->LoadState(&sPosition);
	sPosition.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::GenerateNodeExpectation(CChars* sz, SNodeElement* psNode)
{
	RecurseElement(sz, (int*)psNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::RecurseElement(CChars* sz, int* pvElement)
{
	EElement		eElement;
	CDefinition*	pcDefinition;
	SNodeElement*	psNode;
	int*			pvNext;
	int				i;
	
	eElement = (EElement)(*pvElement);
	switch (eElement)
	{
	case ET_Node:
		psNode = ((SNodeElement*)pvElement);
		pvNext = (int*)RemapSinglePointer(psNode, psNode->HeaderSize()*4);
		RecurseElement(sz, pvNext);
		if (psNode->sNode.eNodeType == NT_Exclusive)
		{
			for (i = 1; i < psNode->sNode.iNumElementsInNode; i++)
			{
				sz->Append(" or ");
				pvNext = GetNextElement(pvNext);
				RecurseElement(sz, pvNext);
			}
		}
		break;

	case ET_Text:
		sz->AppendQuoted(((STextElement*)pvElement)->szStringCharacters);
		break;

	case ET_Identifier:
		sz->Append("<Identifier>");
		break;

	case ET_Char:
		sz->AppendQuoted(((SCharElement*)pvElement)->cChar);
		break;

	case ET_Definition:
		pcDefinition = mpcMeta->GetDefinition(((SDefinitionElement*)pvElement)->iDefinitionNum);
		sz->Append(pcDefinition->mszName);
		break;

	case ET_PrimitiveType:
		sz->Append('<');
		sz->Append(((STypeElement*)pvElement)->GetType());
		sz->Append('>');
		break;

	case ET_End:
		sz->Append("<End of Text>");
		break;

	default:
		gcLogger.Error("Don't know element type");
		sz->Append("<Error!>");
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::RollForward(void)
{
	mpcParser->SkipWhiteSpace();
	mpcParser->FindWhiteSpace();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMetaTokeniser::Output(char* szText1, char* szText2, char* szText3, char* szText4, char pad, int iNodeDepth)
{
	CChars sz;

	//sz.Init();
	//sz.Kill();
	sz.Init(pad, iNodeDepth);
	sz.AppendList(szText1, szText2, szText3, szText4, NULL);
	sz.Append('\n');
	EngineOutput(sz.Text());
	sz.Kill();
}

