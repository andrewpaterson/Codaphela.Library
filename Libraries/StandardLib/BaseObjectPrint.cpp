/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Objects.h"
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::PrintIdentifier(CChars* psz)
{
	char*	szName;

	psz->Append(ClassName());
	psz->Append(":");
	psz->Append(GetIndex());
	szName = GetName();
	if (!StrEmpty(szName))
	{
		psz->Append(":\"");
		psz->Append(szName);
		psz->Append("\"");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFroms(void)
{
	CChars				sz;
	size				i;
	size				iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	size				j;
	size				iNumHeapFroms;
	CEmbeddedObject*	pcFromObject;
	size				iLength;
	CChars				szLine;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	sz.Insert(0, &szLine);
	sz.Append("Total Heap Froms [");
	sz.Append(NumHeapFroms());
	sz.Append("], ");

	sz.Append("Stack Froms [");
	sz.Append(NumStackFroms());
	sz.Append("]\n");

	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iNumHeapFroms = pcEmbedded->CEmbeddedObject::NumHeapFroms();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Heap Froms [");
		sz.Append(iNumHeapFroms);
		sz.Append("], ");
		sz.Append("Stack Froms [");
		sz.Append(pcEmbedded->CEmbeddedObject::NumStackFroms());
		sz.Append("]\n");

		for (j = 0; j < iNumHeapFroms; j++)
		{
			pcFromObject = pcEmbedded->GetHeapFrom(j);
			sz.Append(" ");
			pcFromObject->PrintObject(&sz);
			sz.AppendNewLine();
		}
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpPointerTos(void)
{
	CChars								sz;
	size								i;
	size								iNumEmbedded;
	CEmbeddedObject*					pcEmbedded;
	size								j;
	size								iNumTos;
	CEmbeddedObject**					ppcToObject;
	size								iLength;
	CChars								szLine;
	CArrayEmbeddedObjectPtr		acTos;
	CBaseObject*						pcToObject;
	size								iTotalTos;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	iTotalTos = 0;
	iNumEmbedded = NumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iTotalTos += pcEmbedded->NumPointerTos();
	}

	sz.Insert(0, &szLine);
	sz.Append("Total Tos [");
	sz.Append(iTotalTos);
	sz.Append("]\n");

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		acTos.Init();
		pcEmbedded->BaseGetPointerTos(&acTos);
		iNumTos = acTos.NumElements();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Tos [");
		sz.Append(iNumTos);
		sz.Append("]\n");

		for (j = 0; j < iNumTos; j++)
		{
			ppcToObject = acTos.Get(j);  //A pointed to never comes back NULL.
			sz.Append(" ");
				
			if ((*ppcToObject)->IsBaseObject())
			{
				pcToObject = (CBaseObject*)*ppcToObject;
				pcToObject->PrintObject(&sz);
			}
			else
			{
				sz.Append("HollwEmbeddedObject");
			}
			sz.AppendNewLine();
		}

		acTos.Kill();
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Print(CChars* psz)
{
	PrintState(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintState(CChars* psz)
{
	int		iDistToRoot;

	iDistToRoot = GetDistToRoot();
	PrintIdentifier(psz);
	psz->Append(" R");
	if (iDistToRoot >= 0)
	{
		psz->Append(iDistToRoot);
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		psz->Append("u");
	}
	else if (iDistToRoot == CLEARED_DIST_TO_ROOT)
	{
		psz->Append("c");
	}
	else
	{
		psz->Append("x");
	}
	psz->Append(" S");
	if (!IsAllocatedInObjects())
	{
		psz->Append("0");
	}
	else
	{
		psz->Append("u");
	}

	psz->Append(" (");
	PrintFlagsShorthand(psz);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintFlagsShorthand(CChars* psz)
{
	CChars	sz;
	size	uiEnd;
	size	ui;
	char	c;
	bool	bPrintNext;

	sz.Init();
	PrintFlags(&sz);

	bPrintNext = true;
	uiEnd = sz.Length();
	for (ui = 0; ui < uiEnd; ui++)
	{
		c = sz.GetChar(ui);
		if (c == ',')
		{
			psz->Append(c);
		}
		if (c == ' ')
		{
			psz->Append(c);
			bPrintNext = true;
		}
		else if (c == '_')
		{
			bPrintNext = true;
		}
		else
		{
			if (bPrintNext)
			{
				psz->Append(c);
				bPrintNext = false;
			}
		}
	}

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PrintFlags(CChars* psz)
{
	uint16	uiFlags;

	uiFlags = GetFlags();
	if (uiFlags & OBJECT_FLAGS_INVALIDATED)
	{
		psz->Append("INVALIDATED, ");
	}
	if (uiFlags & OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		psz->Append("CALLED_CONSTRUCTOR, ");
	}
	if (uiFlags & OBJECT_FLAGS_DIRTY)
	{
		psz->Append("DIRTY, ");
	}
	if (uiFlags & OBJECT_FLAGS_FREED)
	{
		psz->Append("FREED, ");
	}
	if (uiFlags & OBJECT_FLAGS_DUMPED)
	{
		psz->Append("DUMPED, ");
	}
	if (uiFlags & OBJECT_FLAGS_STORED)
	{
		psz->Append("STORED, ");
	}
	if (uiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY)
	{
		psz->Append("TESTED_FOR_SANITY, ");
	}
	if (uiFlags & OBJECT_FLAGS_CALLED_ALLOCATE)
	{
		psz->Append("CALLED_ALLOCATE, ");
	}
	if (uiFlags & OBJECT_FLAGS_CALLED_INIT)
	{
		psz->Append("CALLED_INIT, ");
	}
	if (uiFlags & OBJECT_FLAGS_CALLED_KILL)
	{
		psz->Append("CALLED_KILL, ");
	}
	if (uiFlags & OBJECT_FLAGS_CALLED_CLASS)
	{
		psz->Append("CALLED_CLASS, ");
	}

	if (psz->EndsWith(", "))
	{
		psz->RemoveFromEnd(2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFlags(void)
{
	CChars	sz;

	sz.Init();
	PrintFlags(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Dump(void)
{
}

