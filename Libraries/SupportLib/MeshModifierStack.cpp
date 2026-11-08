/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/Objects.h"
#include "MeshEditor.h"
#include "MeshModifierStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Init(Ptr<CMesh> pSource)
{
	PreInit();

	macEditors.Init();
	miCurrent = -1;
	mpSource = pSource;
	mpOutput = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Free(void)
{
	macEditors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Class(void)
{
	U_SInt(miCurrent);
	U_Unknown(CArrayUnknown, macEditors);
	M_Pointer(mpSource);
	M_Pointer(mpOutput);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshModifierStack::Load(CObjectReader* pcFile)
{
	CObject::NotImplemented(__METHOD__);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshModifierStack::Save(CObjectWriter* pcFile)
{
	CObject::NotImplemented(__METHOD__);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::ReInit(void)
{
	macEditors.ReInit();
	miCurrent = -1;
	mpOutput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::AddModifier(CMeshModifier* pcEditor)
{
	macEditors.Add(pcEditor);
	mpOutput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::ApplyAll(CMeshEditor* pcMeshEditor)
{
	CMeshModifier*	pcEditor;
	int				iNumElements;

	iNumElements = (int)macEditors.NumElements();
	if (HasSourceModifier())
	{
		pcMeshEditor->RestoreBase();
		for (miCurrent = 0; miCurrent < iNumElements; miCurrent++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(miCurrent);
			pcEditor->ClearConnectivity();
			TellOfFaces(pcEditor, &pcMeshEditor->mpMesh);
		}
	}

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
void CMeshModifierStack::TellOfFaces(CMeshModifier* pcModifier, CMesh* pcMesh)
{
	int*	piCorners;
	int		i;
	int		iNumFaces;

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		pcModifier->AddFace(piCorners[0], piCorners[1], piCorners[2]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshModifierStack::HasSourceModifier(void)
{
	CMeshModifier*	pcModifier;
	size			i;
	size			uiNumElements;

	uiNumElements = macEditors.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcModifier = (CMeshModifier*)macEditors.Get(i);
		if (pcModifier->IsSource())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshModifierStack::GetSource(void)
{
	return mpSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshModifierStack::GetOutput(void)
{
	CMeshModifier*	pcModifier;
	Ptr<CMesh>		pMesh;
	size			i;
	size			uiNumElements;

	if (mpOutput.IsNull())
	{
		if (mpSource.IsNull())
		{
			mpSource = OMalloc<CMesh>();
		}

		pMesh = mpSource;
		uiNumElements = macEditors.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcModifier = (CMeshModifier*)macEditors.Get(i);
			pMesh = pcModifier->Modify(pMesh);
		}
		mpOutput = pMesh;
	}
	return mpOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Touch(void)
{
	mpOutput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::Flatten(void)
{
	mpSource = GetOutput();
	ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshModifierStack::NumEditors(void)
{
	return (int)macEditors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::ClearConnectivity(void)
{
	size			i;
	CMeshModifier*	pcEditor;

	if (miCurrent != -1)
	{
		for (i = miCurrent+1; i < macEditors.NumElements(); i++)
		{
			pcEditor = (CMeshModifier*)macEditors.Get(i);
			pcEditor->ClearConnectivity();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshModifierStack::AddPosition(SFloat32Vec3* psPosition)
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
void CMeshModifierStack::MovePosition(int iPosition, SFloat32Vec3* psDelta)
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
void CMeshModifierStack::MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psDelta)
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

