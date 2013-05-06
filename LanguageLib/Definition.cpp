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
#include "Definition.h"
#include "TextElement.h"
#include "IdentifierElement.h"
#include "CharElement.h"
#include "NodeElement.h"
#include "TypeElement.h"
#include "DefinitionElement.h"
#include "EndElement.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::Init(char* szName, int iNumber)
{
	memset(this, 0, sizeof(CDefinition));
	mszName.Init(szName);
	miNumber = iNumber;
	mcElements.Init(10);
	msParserState.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::Kill(void)
{
	mszName.Kill();
	mcElements.Kill();
	msParserState.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::Copy(CDefinition* pcDefinition)
{
	Kill();
	Init(pcDefinition->mszName.Text(), pcDefinition->miNumber);
	mbComplete = pcDefinition->mbComplete;
	msParserState.Init();
	mcElements.Copy(&pcDefinition->mcElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddCharacter(char c)
{
	SCharElement*	psCharElement;
	int				iOldPos;

	psCharElement = NULL;
	iOldPos = mcElements.GrowByNumElements(psCharElement->Size());
	psCharElement = (SCharElement*)mcElements.Get(iOldPos);
	psCharElement->Init(c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddUnknownIdentifier(void)
{
	SIdentifierElement*		psIdentifier;
	int						iOldPos;

	//This identifier can be anything whatsoever.
	psIdentifier = NULL;
	iOldPos = mcElements.GrowByNumElements(psIdentifier->Size());
	psIdentifier = (SIdentifierElement*)mcElements.Get(iOldPos);
	psIdentifier->Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddPrimitiveType(EElement eType)
{
	STypeElement*	psPrimitiveElement;
	int				iOldPos;

	psPrimitiveElement = NULL;
	iOldPos = mcElements.GrowByNumElements(psPrimitiveElement->Size());
	psPrimitiveElement = (STypeElement*)mcElements.Get(iOldPos);
	psPrimitiveElement->Init(eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddText(char* szString)
{
	STextElement*	psTextElement;
	int				iOldPos;

	psTextElement = NULL;
	iOldPos = mcElements.GrowByNumElements(psTextElement->Size(szString));
	psTextElement = (STextElement*)mcElements.Get(iOldPos);
	psTextElement->Init(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddDefinition(int iDefinitionNum)
{
	SDefinitionElement*		psDefinitionElement;
	int						iOldPos;

	psDefinitionElement = NULL;
	iOldPos = mcElements.GrowByNumElements(psDefinitionElement->Size());
	psDefinitionElement = (SDefinitionElement*)mcElements.Get(iOldPos);
	psDefinitionElement->Init(iDefinitionNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddEnd(void)
{
	SEndElement*	psEnd;
	int				iOldPos;

	psEnd = NULL;
	iOldPos = mcElements.GrowByNumElements(psEnd->Size());
	psEnd = (SEndElement*)mcElements.Get(iOldPos);
	psEnd->Init();	
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CDefinition::NextElementPosition(void)
{
	return mcElements.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::AddNode(int iPos, CNodeConstructor* pcNode)
{
	SNodeElement*	psNodeElement;
	int				iElementsSize;

	iElementsSize =	mcElements.NumElements() - iPos;
	if (iElementsSize != 0)
	{
		psNodeElement = NULL;
		psNodeElement = (SNodeElement*)mcElements.InsertNumElementsAt(psNodeElement->HeaderSize(pcNode->mszName), iPos);
		psNodeElement->Init(pcNode, iElementsSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::SetFirstNodeName(char* szNodeName)
{
	SNodeElement*	psNodeElement;
	int				iCurrentSize;
	int				iNeededSize;

	psNodeElement = (SNodeElement*)mcElements.Get(0);
	if (psNodeElement->szName[0] == 0)
	{
		iCurrentSize = psNodeElement->HeaderSize();
		iNeededSize = psNodeElement->HeaderSize(szNodeName);
		mcElements.InsertNumElementsAt(iNeededSize-iCurrentSize, iCurrentSize-1);
		psNodeElement = (SNodeElement*)mcElements.Get(0);
		strcpy(psNodeElement->szName, szNodeName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::Dump(void)
{
	CChars		szString;
	szString.Init("DEFINE(");
	if (!mszName.Empty())
	{
		szString.Append(mszName);
		szString.Append(", ");
	}
	szString.Append(miNumber);
	szString.Append(")");
	szString.Append(" [");
	szString.Append(mcElements.NumElements());
	szString.Append("]\n");
	EngineOutput(szString.Text());
	szString.Kill();

	RecurseDump(0, 1, 0);

	EngineOutput("\n");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDefinition::RecurseDump(int iNextElement, int iNumElements, int iLevel)
{
	EElement	eElement;
	void*		pvElement;
	CChars		szString;
	int			i;
	int			iOldElement;

	if (mcElements.NumElements() == 0)
	{
		return;
	}

	for (i = 0; i < iNumElements; i++)
	{
		iOldElement = iNextElement;
		eElement = (EElement)mcElements.GetValue(iNextElement);
		pvElement = (void*)mcElements.Get(iNextElement);
		switch (eElement)
		{
		case ET_Text:
			((STextElement*)pvElement)->String(&szString);
			iNextElement += ((STextElement*)pvElement)->Size();
			break;

		case ET_Identifier:
			((SIdentifierElement*)pvElement)->String(&szString);
			iNextElement += ((SIdentifierElement*)pvElement)->Size();
			break;

		case ET_Char:
			((SCharElement*)pvElement)->String(&szString);
			iNextElement += ((SCharElement*)pvElement)->Size();
			break;

		case ET_Node:
			((SNodeElement*)pvElement)->String(&szString);
			iNextElement += ((SNodeElement*)pvElement)->Size();
			break;

		case ET_Definition:
			((SDefinitionElement*)pvElement)->String(&szString);
			iNextElement += ((SDefinitionElement*)pvElement)->Size();
			break;

		case ET_PrimitiveType:
			((STypeElement*)pvElement)->String(&szString);
			iNextElement += ((STypeElement*)pvElement)->Size();
			break;

		case ET_End:
			((SEndElement*)pvElement)->String(&szString);
			iNextElement += ((SEndElement*)pvElement)->Size();
			break;

		default:
			szString.Init("ERROR!");
			break;
		}

		CChars szTemp;
		CChars	szLineNumber;

		szTemp.Init();
		szTemp.Append(iOldElement);
		szLineNumber.Init();
		szLineNumber.RightAlign(szTemp, ' ', 3);
		szTemp.Kill();
		szLineNumber.Append(":  ");
		EngineOutput(szLineNumber.Text());
		szLineNumber.Kill();

		for (int j = 0; j < iLevel; j++)
		{
			EngineOutput("   ");
		}
		EngineOutput(szString.Text());
		EngineOutput("\n");
		szString.Kill();

		if (eElement == ET_Node)
		{
			RecurseDump(iOldElement + ((SNodeElement*)pvElement)->HeaderSize(), ((SNodeElement*)pvElement)->sNode.iNumElementsInNode, iLevel+1);
		}
	}
}
