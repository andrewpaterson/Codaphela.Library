/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Material.h"


//////////////////////////////////////////////////////////////////////////
//	Material
//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Init(void)
{
	CStandardTrackerObject::Init();
	msDiffuse.Init();
	msDiffuseLevel.Init();
	msSpecular.Init();
	msIllumination.Init();
	msOpacity.Init();
	msBump.Init();
	msDetail.Init();
	msDecal.Init();
	msReflection.Init();
	msProperties.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Kill(void)
{
	msDiffuse.Kill();
	msDiffuseLevel.Kill();
	msSpecular.Kill();
	msIllumination.Kill();
	msOpacity.Kill();
	msBump.Kill();
	msDetail.Kill();
	msDecal.Kill();
	msReflection.Kill();
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Copy(CMaterial* pcMaterial)
{
	//Implement me please.
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMaterial::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	ReturnOnFalse(msProperties.Load(pcFile));
	ReturnOnFalse(msDiffuse.Load(pcFile));
	ReturnOnFalse(msDiffuseLevel.Load(pcFile));
	ReturnOnFalse(msSpecular.Load(pcFile));
	ReturnOnFalse(msIllumination.Load(pcFile));
	ReturnOnFalse(msOpacity.Load(pcFile));
	ReturnOnFalse(msBump.Load(pcFile));
	ReturnOnFalse(msDetail.Load(pcFile));
	ReturnOnFalse(msDecal.Load(pcFile));
	ReturnOnFalse(msReflection.Load(pcFile));

	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMaterial::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));

	ReturnOnFalse(msProperties.Save(pcFile));
	ReturnOnFalse(msDiffuse.Save(pcFile));
	ReturnOnFalse(msDiffuseLevel.Save(pcFile));
	ReturnOnFalse(msSpecular.Save(pcFile));
	ReturnOnFalse(msIllumination.Save(pcFile));
	ReturnOnFalse(msOpacity.Save(pcFile));
	ReturnOnFalse(msBump.Save(pcFile));
	ReturnOnFalse(msDetail.Save(pcFile));
	ReturnOnFalse(msDecal.Save(pcFile));
	ReturnOnFalse(msReflection.Save(pcFile));

	return EndSaveStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Dump(void)
{
}


