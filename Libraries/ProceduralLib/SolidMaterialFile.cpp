/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FileUtil.h"
#include "StandardLib/ChunkFileObjectWriter.h"
#include "StandardLib/ChunkFileSystemObjectReader.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ExternalObjectDeserialiser.h"
#include "StandardLib/ExternalObjectSerialiser.h"
#include "StandardLib/ObjectFileGeneral.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Box.h"
#include "SupportLib/ShapeSmoothUnion.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/SolidChecker.h"
#include "SupportLib/Sphere.h"
#include "SolidAdjust.h"
#include "SolidBricks.h"
#include "SolidCavity.h"
#include "SolidCells.h"
#include "SolidChannel.h"
#include "SolidCombine.h"
#include "SolidFlat.h"
#include "SolidGradient.h"
#include "SolidMask.h"
#include "SolidMerge.h"
#include "SolidNoise.h"
#include "SolidRamp.h"
#include "SolidSlope.h"
#include "SolidWarp.h"
#include "SolidMaterialFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMaterial::Init(Ptr<CSolidTexture> pSurface)
{
	PreInit();

	mpSurface = pSurface;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMaterial::Class(void)
{
	M_Pointer(mpSurface);
}


void CSolidMaterial::Free(void) {}


//////////////////////////////////////////////////////////////////////////
//
// szFilename as the folder it's in and its name less the object files'
// extension, which is how the objects' reader and writer are told of a
// file.
//
//////////////////////////////////////////////////////////////////////////
static void SolidMaterialSplit(const char* szFilename, CChars* pszDirectory, CChars* pszName)
{
	CFileUtil	cFileUtil;
	CChars		szExtension;

	pszDirectory->Init(szFilename);
	cFileUtil.RemoveLastFromPath(pszDirectory);
	if (pszDirectory->Empty())
	{
		pszDirectory->Append(".");
	}
	pszName->Init();
	cFileUtil.GetFileName(pszName, szFilename);

	szExtension.Init(".");
	szExtension.Append(OBJECT_FILE_EXTENSION);
	if (pszName->EndsWithIgnoreCase(szExtension.Text()))
	{
		pszName->RemoveFromEnd(szExtension.Length());
	}
	szExtension.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The textures are written through a material named SOLID_MATERIAL_NAME,
// which is only there while they're written.
//
//////////////////////////////////////////////////////////////////////////
bool WriteSolidMaterial(Ptr<CSolidTexture> pSurface, const char* szFilename)
{
	Ptr<CSolidMaterial>			pMaterial;
	CExternalObjectSerialiser	cSerialiser;
	CChunkFileObjectWriter		cWriter;
	CChars						szDirectory;
	CChars						szName;
	bool						bResult;

	pMaterial = gcObjects.Malloc<CSolidMaterial>(SOLID_MATERIAL_NAME);
	if (pMaterial.IsNull())
	{
		return false;
	}
	pMaterial->Init(pSurface);

	SolidMaterialSplit(szFilename, &szDirectory, &szName);
	cWriter.Init(szDirectory.Text(), "", szName.Text());
	cSerialiser.Init(&cWriter);
	bResult = cSerialiser.Write(&pMaterial);
	cSerialiser.Kill();
	cWriter.Kill();
	szName.Kill();
	szDirectory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> ReadSolidMaterial(const char* szFilename)
{
	Ptr<CSolidMaterial>				pMaterial;
	Ptr<CSolidTexture>				pSurface;
	CExternalObjectDeserialiser		cDeserialiser;
	CChunkFileSystemObjectReader	cReader;
	CFileUtil						cFileUtil;
	CChars							szDirectory;
	CChars							szName;
	CChars							szFile;

	SolidMaterialSplit(szFilename, &szDirectory, &szName);
	szFile.Init(szDirectory);
	cFileUtil.AppendToPath(&szFile, szName.Text());
	szFile.Append(".");
	szFile.Append(OBJECT_FILE_EXTENSION);
	if (cFileUtil.Exists(szFile.Text()))
	{
		AddSolidTextureConstructors();
		cReader.Init(szDirectory.Text(), szName.Text());
		cDeserialiser.Init(&cReader, false, &gcObjects);
		pMaterial = Ptr<CSolidMaterial>(dynamic_cast<CSolidMaterial*>(cDeserialiser.Read(SOLID_MATERIAL_NAME)));
		cDeserialiser.Kill();
		cReader.Kill();
		if (pMaterial.IsNotNull())
		{
			pSurface = pMaterial->mpSurface;
		}
	}
	szFile.Kill();
	szName.Kill();
	szDirectory.Kill();
	return pSurface;
}


//////////////////////////////////////////////////////////////////////////
//
// Every class a material's graph may hold: the solid textures, and the
// shapes a cavity is of.  Told twice, a class is told once.
//
//////////////////////////////////////////////////////////////////////////
void AddSolidTextureConstructors(void)
{
	gcObjects.AddConstructor<CSolidMaterial>();
	gcObjects.AddConstructor<CSolidChecker>();
	gcObjects.AddConstructor<CSolidFlat>();
	gcObjects.AddConstructor<CSolidGradient>();
	gcObjects.AddConstructor<CSolidNoise>();
	gcObjects.AddConstructor<CSolidCells>();
	gcObjects.AddConstructor<CSolidBricks>();
	gcObjects.AddConstructor<CSolidSlope>();
	gcObjects.AddConstructor<CSolidCavity>();
	gcObjects.AddConstructor<CSolidRamp>();
	gcObjects.AddConstructor<CSolidAdjust>();
	gcObjects.AddConstructor<CSolidMerge>();
	gcObjects.AddConstructor<CSolidMask>();
	gcObjects.AddConstructor<CSolidWarp>();
	gcObjects.AddConstructor<CSolidChannel>();
	gcObjects.AddConstructor<CSolidCombine>();
	gcObjects.AddConstructor<CSphere>();
	gcObjects.AddConstructor<CBox>();
	gcObjects.AddConstructor<CShapeUnion>();
	gcObjects.AddConstructor<CShapeSmoothUnion>();
}

