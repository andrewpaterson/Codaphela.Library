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

** ------------------------------------------------------------------------ **/
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshSmoothing.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mauiGroups.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::Free(void)
{
	mauiGroups.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::Class(void)
{
	CMeshDetail::Class();
	U_Unknown(CArrayUint32, mauiGroups);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::Clear(void)
{
	mauiGroups.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::Copy(CMeshSmoothing* pcSource)
{
	size	i;

	mbInUse = pcSource->mbInUse;
	mauiGroups.ReInit();
	for (i = 0; i < pcSource->mauiGroups.NumElements(); i++)
	{
		mauiGroups.Add(pcSource->mauiGroups.GetValue(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothing::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(CMeshDetail::Load(pcFile));
	ReturnOnFalse(mauiGroups.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshSmoothing::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(CMeshDetail::Save(pcFile));
	ReturnOnFalse(mauiGroups.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::AddFace(void)
{
	if (mbInUse)
	{
		mauiGroups.Add((uint32)0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Turns it on, with every face in uiGroups.
//
//////////////////////////////////////////////////////////////////////////
void CMeshSmoothing::SetAll(size uiNumFaces, uint32 uiGroups)
{
	size	i;

	mbInUse = true;
	mauiGroups.Resize(uiNumFaces);
	for (i = 0; i < uiNumFaces; i++)
	{
		mauiGroups.SetValue(i, uiGroups);
	}
}

