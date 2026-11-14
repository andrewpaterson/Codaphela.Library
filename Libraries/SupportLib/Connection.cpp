/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Connection.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CConnection> CConnection::Init(void)
{
	PreInit();

	mszName.Init();
	mpParent = NULL;
	mpBone = NULL;
	msWorldMatrix.Identity();
	msLocalMatrix.Identity();
	mbSelected = false;
	mbTopLevel = false;
	mbParity = false;
	muiUpdate = 0;

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Class(void)
{
	U_String(mszName);
	M_Pointer(mpParent);
	M_Pointer(mpBone);
	U_4x4Float32(msWorldMatrix);
	U_4x4Float32(msLocalMatrix);
	U_Bool(mbSelected);
	U_Bool(mbTopLevel);
	U_Bool(mbParity);
	U_Size(muiUpdate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Copy(CConnection* pcConnection)
{
	mszName.ReInit();
	mszName.Append(pcConnection->mszName.Text());
	mpParent = pcConnection->mpParent;
	mpBone = pcConnection->mpBone;
	msWorldMatrix = pcConnection->msWorldMatrix;
	msLocalMatrix = pcConnection->msLocalMatrix;
	mbSelected = pcConnection->mbSelected;
	mbTopLevel = pcConnection->mbTopLevel;
	mbParity = pcConnection->mbParity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::UpdateWorldMatrix(size uiUpdate)
{
	if (muiUpdate == uiUpdate)
	{
		return;
	}
	muiUpdate = uiUpdate;

	if (mpParent.IsNotNull())
	{
		mpParent->UpdateWorldMatrix(uiUpdate);
		Float4x4Multiply(&msWorldMatrix, &msLocalMatrix, &mpParent->msWorldMatrix);
	}
	else
	{
		msWorldMatrix = msLocalMatrix;
	}
	mbParity = msWorldMatrix.Determinant() < 0.0f;
}

