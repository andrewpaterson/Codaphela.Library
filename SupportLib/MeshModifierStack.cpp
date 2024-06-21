/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "MeshEditor.h"
#include "MeshModifierStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Init(void)
{
	macEditors.Init();
	miCurrent = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Kill(void)
{
	macEditors.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::AddModifier(CMeshModifier* pcEditor)
{
	macEditors.Add(pcEditor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::ApplyAll(CMeshEditor* pcMeshEditor)
{
	CMeshModifier*	pcEditor;
	int				iNumElements;

	iNumElements = macEditors.NumElements();
	for (miCurrent = 0; miCurrent < iNumElements; miCurrent++)
	{
		pcEditor = (CMeshModifier*)macEditors.Get(miCurrent);
		pcEditor->Apply(pcMeshEditor);
	}
	miCurrent = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshModifierStack::NumEditors(void)
{
	return macEditors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::ReInitConnectivity(void)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->ReInitConnectivity();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::AddPosition(SFloat3* psPosition)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->AddPosition(psPosition);
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::AddFace(int iCorner1, int iCorner2, int iCorner3)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->AddFace(iCorner1, iCorner2, iCorner3);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::RemoveFace(int iFace)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->RemoveFace(iFace);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::RemoveFaces(CArrayInt* paiFaces)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->RemoveFaces(paiFaces);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::RemoveCorner(int iCorner)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->RemoveCorner(iCorner);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::MovePosition(int iPosition, SFloat3* psDelta)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->MovePosition(iPosition, psDelta);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::MovePositions(CArrayInt* paiPositions, SFloat3* psDelta)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->MovePositions(paiPositions, psDelta);
		}
	}

}

