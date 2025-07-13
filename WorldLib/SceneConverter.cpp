/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "SupportLib/Scene.h"
#include "SceneConverter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneConverter::Init(CScene* pcScene, CWorld* pcWorld)
{
	mpcScene = pcScene;
	mpcWorld = pcWorld;

	mcMaterialConverter.Init(this);
	mcMeshConverter.Init(this);
	mcAnimationConverter.Init(this);
	mcInstanceConverter.Init(this);
	mcTextureConverter.Init(this);
	mcMapper.Init();

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneConverter::Kill(void)
{
	mcMaterialConverter.Kill();
	mcMeshConverter.Kill();
	mcAnimationConverter.Kill();
	mcInstanceConverter.Kill();
	mcTextureConverter.Kill();
	mcMapper.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertTexture(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT d3dFormat, CImage* pcImage, bool bDiscard, int iWidth, int iHeight, EPrimitiveType eType, EChannel eFirst, ...)
{
	va_list				vaMarker;
	int					iCount;
	EChannel		eIC;
	CImage				cExport;
	bool				bResult;

	cExport.Init();
	iCount = 0;
	eIC = eFirst;

	cExport.BeginChange();
	va_start(vaMarker, eFirst);
	while (eIC != CHANNEL_ZERO)
	{
		cExport.AddChannel(eIC, eType);
		iCount++;
		eIC = va_arg(vaMarker, EChannel);
	}
	va_end(vaMarker);

	cExport.SetSize(iWidth, iHeight);
	cExport.SetData((void*)1);
	cExport.EndChange();

	bResult = mcTextureConverter.Convert(ppcGraphicsTexture, d3dFormat, pcImage, &cExport, bDiscard);
	cExport.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertMeshToGraphicsObject(CGraphicsObject** ppcGraphicsObject, CMeshObject** ppcMeshObject, CMesh* pcMesh, CArrayIntAndPointer* pcConnectionAndIndex)
{
	return mcMeshConverter.Convert(ppcGraphicsObject, ppcMeshObject, pcMesh, pcConnectionAndIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertMaterialToGraphicsMaterial(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState, CMaterial* pcMaterial)
{
	return mcMaterialConverter.Convert(ppcGraphicsMaterial, ppcGraphicsState, pcMaterial);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertSequenceToMovement(CMovement** ppcMovement, CSequence* pcSequence, SMatrix* psMatrix)
{
	return mcAnimationConverter.Convert(ppcMovement, pcSequence, psMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertInstanceToGraphicsInstance(CInstance* pcInstance)
{
	return mcInstanceConverter.Convert(pcInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertScene(void)
{
	int		iInstance;
	bool	bResult;

	bResult = true;
	for (iInstance = 0; iInstance < mpcScene->mcInstanceTracker.Num(); iInstance++)
	{
		if (!ConvertInstanceToGraphicsInstance((CInstance*)mpcScene->mcInstanceTracker.Get(iInstance)))
		{
			bResult = false;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSceneConverter::ConvertMeshes(void)
{
	int					iMesh;
	bool				bResult;
	CGraphicsObject*	pcGraphicsObject;
	CMeshObject*		pcMeshObject;
	CMesh*				pcMesh;
	CArrayIntAndPointer		cConnectionAndIndex;

	bResult = true;
	for (iMesh = 0; iMesh < mpcScene->mcMeshTracker.Num(); iMesh++)
	{
		pcMesh = (CMesh*)mpcScene->mcMeshTracker.Get(iMesh);
		if (!ConvertMeshToGraphicsObject(&pcGraphicsObject, &pcMeshObject, pcMesh, &cConnectionAndIndex))
		{
			bResult = false;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWorld* CSceneConverter::GetWorld(void) { return mpcWorld; }
CScene* CSceneConverter::GetScene(void) { return mpcScene; }
CConversionMapper* CSceneConverter::GetMapper(void) { return &mcMapper; }

