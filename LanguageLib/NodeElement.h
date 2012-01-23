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
#ifndef __NODE_ELEMENT_H__
#define __NODE_ELEMENT_H__
#include "BaseLib/Logger.h"
#include "BaseLib/IntegerHelper.h"
#include "NodeConstructor.h"
#include "BaseLib/Chars.h"


struct SNodeElement
{
	EElement	eET_Node;	//This int must always have the value ET_Node.
	SNodeDetail	sNode;
	int			iIntSize;	//The total number of ints used by this node, including sub-nodes.
							//A zero value means the size cannot yet be calculated.
	char		szName[1];


	void Init(CNodeConstructor*	pcNode, int iElementSize)
	{
		if (!pcNode->TestForCorrectness())
		{
			return;
		}

		memcpy(&sNode, &pcNode->msNode, sizeof(SNodeDetail));
		eET_Node = ET_Node;
		strcpy(szName, pcNode->mszName);
		iIntSize = HeaderSize(szName) + iElementSize;
	}

	int Size(void)
	{
		return iIntSize;
	}

	int HeaderSize(char* szNodeName)
	{
		//This works because the char szName[1] provides enough space for three letters and the zero char.
		int	iNameLength;

		iNameLength = (int)strlen(szNodeName);
		return (sizeof(SNodeElement)+3)/4 + iNameLength/4;
	}

	int HeaderSize(void)
	{
		return HeaderSize(szName);
	}

	void String(CChars* szString)
	{
		szString->Init();
		szString->Append("NODE(");
		if (strlen(szName) > 0)
		{
			szString->Append('"');
			szString->Append(szName);
			szString->Append("\", ");
		}
		szString->Append(sNode.iNumElementsInNode);
		szString->Append(", ");

		switch(sNode.eNodeType) 
		{
		case NT_Single:
			szString->Append("Single, ");
			break;
		case NT_Ordered:
			szString->Append("Ordered, ");
			break;
		case NT_Exclusive:
			szString->Append("Exclusive, ");
			break;
		case NT_Inclusive:
			szString->Append("Inclusive, ");
			break;
		}

		switch(sNode.eModifier) 
		{
		case NM_Inverse:
			szString->Append("Inverse, ");
			break;
		}

		if (sNode.bFinal) 
		{
			szString->Append("Final, ");
		}

		switch(sNode.eNecessity) 
		{
		case NN_Necessary:
			szString->Append("Necessary, ");
			break;
		case NN_Empty:
			szString->Append("Empty, ");
			break;
		}

		szString->Append(sNode.iMinimum);
		szString->Append("-");

		if (sNode.iMaximum == MAXINT)
		{
			szString->Append('X');
		}
		else
		{
			szString->Append(sNode.iMaximum);
		}
		szString->Append(')');
	}
};


#endif //__NODE_ELEMENT_H__