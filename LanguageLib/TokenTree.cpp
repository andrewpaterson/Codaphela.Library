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
#include "BaseLib/Define.h"
#include "BaseLib/DebugOutput.h"
#include "TokenTree.h"
#include "TokenDumper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::Init(void)
{
	mcNodeNames.Init();
	mcKeywords.Init();
	mcIdentifiers.Init();
	mcTree.Init();
	mpcCurrentNode = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::Kill(void)
{
	void*				psNode;
	ETokenTreeNodeType	eNodeType;
	CTokenArray*		pcTokenArray;

	psNode = mcTree.StartTraversal();
	while(psNode)
	{
		eNodeType = (ETokenTreeNodeType)mcTree.GetNodeType(psNode);
		if (eNodeType == TTNT_Dynamic)
		{
			pcTokenArray = (CTokenArray*)psNode;
			pcTokenArray->Kill();
		}
		psNode = mcTree.TraverseFrom(psNode);
	}
	mcTree.Kill();
	mcIdentifiers.Kill();
	mcKeywords.Kill();
	mcNodeNames.Kill();
	mpcCurrentNode = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::BeginTree(void)
{
	CTokenArray*	pcTokenArray;

	pcTokenArray = (CTokenArray*)mcTree.InsertRoot(sizeof(CTokenArray), TTNT_Dynamic);
	pcTokenArray->Init(this, NULL);
	mpcCurrentNode = pcTokenArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::EndTree(void)
{
	Finalise();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::BeginNode(SNodeElement* psNode)
{
	CTokenArray*	pcTokenArray;

	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddNode();

		pcTokenArray = (CTokenArray*)mcTree.InsertOnRightOfChildren(mpcCurrentNode, sizeof(CTokenArray), TTNT_Dynamic);
		pcTokenArray->Init(this, psNode);
		mpcCurrentNode = pcTokenArray;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokenTree::EndNode(EEndNode eEnd)
{
	void*	pvParent;

	if (mpcCurrentNode == NULL)
	{
		return FALSE;
	}

	pvParent = mcTree.GetDown(mpcCurrentNode);
	if (eEnd != EN_Parsed)
	{
		return RollBack();
	}
	mpcCurrentNode = (CTokenArray*)pvParent;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddIdentifier(char* sz)
{
	if (mpcCurrentNode)
	{
		mcIdentifiers.Add(sz);
		mpcCurrentNode->AddIdentifier(sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddKeywordText(char* sz)
{
	if (mpcCurrentNode)
	{
		mcKeywords.Add(sz);
		mpcCurrentNode->AddKeywordText(sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddKeywordChar(char c)
{
	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddKeywordChar(c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddInteger(int i)
{
	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddInteger(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddReal(float f)
{
	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddReal(f);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddUnknownText(char* sz)
{
	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddUnknownText(sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::AddUnknownChar(char c)
{
	if (mpcCurrentNode)
	{
		mpcCurrentNode->AddUnknownChar(c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokenTree::RollBack(void)
{
	CTokenArray*	pcParent;
	int				iNodeNum;

	if (ContainsFinalNodes(mpcCurrentNode))
	{
		return FALSE;
	}

	iNodeNum = mcTree.GetChildNum(mpcCurrentNode);
	KillTokenArrays(mpcCurrentNode);

	pcParent = (CTokenArray*)mcTree.GetDown(mpcCurrentNode);
	mcTree.RemoveBranch(mpcCurrentNode);
	mpcCurrentNode = pcParent;

	pcParent->RemoveNode(iNodeNum);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokenTree::ContainsFinalNodes(CTokenArray* pcStartNode)
{
	CTokenArray*	pcNode;

	pcNode = (CTokenArray*)mcTree.TraverseSubTreeFrom(pcStartNode, pcStartNode);
	while (pcNode != NULL)
	{
		if (pcNode->mpsNode->sNode.bFinal)
		{
			return TRUE;
		}
		pcNode = (CTokenArray*)mcTree.TraverseSubTreeFrom(pcStartNode, pcNode);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::KillTokenArrays(CTokenArray* pcStartNode)
{
	CTokenArray*	pcNode;

	pcNode = (CTokenArray*)mcTree.TraverseSubTreeFrom(pcStartNode, pcStartNode);
	while (pcNode != NULL)
	{
		pcNode->Kill();
		pcNode = (CTokenArray*)mcTree.TraverseSubTreeFrom(pcStartNode, pcNode);
	}
	pcStartNode->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::Finalise(void)
{
	CTokenArray*	pcRoot;

	pcRoot = (CTokenArray*)mcTree.GetRoot();
	FinaliseNode(pcRoot);
	mpcCurrentNode = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::FinaliseNode(CTokenArray* pcParent)
{
	CTokenArray*	pcNode;
	int				iNumChildren;
	int				i;

	iNumChildren = mcTree.GetNumChildren(pcParent);
	for (i = iNumChildren-1; i >= 0; i--)
	{
		pcNode = (CTokenArray*)mcTree.GetChildNum(pcParent, i);
		FinaliseNode(pcNode);
		if (pcParent != (CTokenArray*)mcTree.GetRoot())
		{
			if (pcNode->mpsNode->sNode.eNecessity != NN_Necessary)
			{
				pcParent->ReplaceNode(i, pcNode);
				pcNode->Kill();
				mcTree.Remove(pcNode);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenTree::Dump(void)
{
	void*			pvNode;
	SUnknownType	sUnknown;
	CTokenArray*	pcTokenArray;
	CChars			sz;

	pvNode = mcTree.StartTraversal();
	while(pvNode)
	{
		sz.Init('.', mcTree.miLevel);
		EngineOutput(sz.Text());
		sz.Kill();
		mcTree.GetNodeTypeAndSize(pvNode, &sUnknown);
		if (sUnknown.miType == TTNT_Dynamic)
		{
			pcTokenArray = (CTokenArray*)pvNode;
			pcTokenArray->Dump();
		}
		else if (sUnknown.miType == TTNT_Flat)
		{
			DumpTokens((int*)pvNode, -1, &mcKeywords, &mcIdentifiers);
		}
		EngineOutput("\n");
		pvNode = mcTree.TraverseFrom(pvNode);
	}
	EngineOutput("\n");
}

