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
#include "BaseLib/FastFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "GraphicsInstance.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Init(void)
{
	//Do nothing... You must call .Init below instead.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Init(CWorld* pcWorld)
{
	mapMatrices.Init();
	mapMaterials.Init();
	mapStates.Init();
	mapLights.Init();
	mpcGraphicsObject = NULL;
	miFlags = GRAPH_INST_FLAGS_DRAW;
	mpcWorld = pcWorld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Kill(void)
{
	mapMatrices.Kill();
	mapMaterials.Kill();
	mapStates.Kill();
	mapLights.Kill();
	mpcGraphicsObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Draw(void)
{

	if (!CanDraw())
	{
		return;
	}

	//Copy the instances controlling matrix pointers over the objects.  This allows object to be drawn at different positions in the safe frame.
	memcpy_fast(mpcGraphicsObject->GetMatricies()->GetData(), mapMatrices.GetData(), mapMatrices.ByteSize());

	//Copy the instances states over the objects.  This allows different types of filtering on the same object... is it really necessary?
	memcpy_fast(mpcGraphicsObject->GetStates()->GetData(), mapStates.GetData(), mapStates.ByteSize());

	//Copy the instances materials over the objects.  This allows different textures on the same object...
	memcpy_fast(mpcGraphicsObject->GetMaterials()->GetData(), mapMaterials.GetData(), mapMaterials.ByteSize());

	mpcWorld->UseLights(&mapLights);

	//Draw the updated object.
	ReverseCullAsNecessary();
	mpcGraphicsObject->Draw();
	UnreverseCullAsNecessary();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::ReverseCullAsNecessary(void)
{
	if (miFlags & GRAPH_INST_FLAGS_REVERSE_CULL)
	{
		gcD3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::UnreverseCullAsNecessary(void)
{
	if (miFlags & GRAPH_INST_FLAGS_REVERSE_CULL)
	{
		gcD3D.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Set(CGraphicsObject* pcGraphicsObject)
{
	mapMaterials.Kill();
	mapMaterials.Init(pcGraphicsObject->GetMaterials());

	mapStates.Kill();
	mapStates.Init(pcGraphicsObject->GetStates());

	mpcGraphicsObject = pcGraphicsObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::AddMatrixPtr(SMatrix* psMatrix)
{
	mapMatrices.Add(&psMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMatrix* CGraphicsInstance::GetMatrix(int iNum)
{
	return *mapMatrices.Get(iNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGraphicsInstance::CanDraw(void)
{
	return miFlags & GRAPH_INST_FLAGS_DRAW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::Update(void)
{
	CLightInstance*	pcLight;
	size			iLight;
	size			i;

	if (!CanDraw())
	{
		return;
	}

	iLight = 0;
	for (i = 0; i < mpcWorld->mcWorldLightList.NumElements(); i++)
	{
		pcLight = mpcWorld->mcWorldLightList.Get(i);
		if (pcLight->IsEnabled())
		{
			if (mapLights.NumElements() > iLight)
			{
				mapLights.Set(iLight, &pcLight);
			}
			else
			{
				mapLights.Add(&pcLight);
			}
			iLight ++;
		}
	}

	if (iLight != mapLights.NumElements())
	{
		mapLights.Resize(iLight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsInstance::SetFlag(int iFlag, int iFlagValue)
{
	::SetFlagInt(&miFlags, iFlag, iFlagValue);
}


CGraphicsObject* CGraphicsInstance::GetGraphicsObject(void)
{
	return mpcGraphicsObject;
}

