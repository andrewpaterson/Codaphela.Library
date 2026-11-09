/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ConvertOldDragon

Codaphela ConvertOldDragon is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ConvertOldDragon is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ConvertOldDragon.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <string.h>
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/LeakDetector.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/TypeNames.h"
#include "OldDragonLib/OldDragonConverter.h"
#include "OldDragonLib/OldDragonReader.h"


//ConvertOldDragon <old .DRG> <new .DRG> [--max <.max>] [--particles <.csv>]
//
//Converts one old scene to a scene file, reads that back, and compares it with what it should hold.  ConvertAll.py lists what was
//converted and where it went.
static bool Convert(const char* szOld, const char* szNew, const char* szMax, const char* szParticles)
{
	COldDragonScene	cScene;
	size			uiFilled;
	bool			bResult;

	cScene.Init();
	bResult = ReadOldDragonFile(szOld, &cScene);
	if (bResult && szMax)
	{
		bResult = AddOldDragonMaxFaces(szMax, &cScene, &uiFilled);
		if (bResult && (uiFilled != cScene.macMeshes.NumElements()))
		{
			printf("Only %d of %d meshes were filled from [%s].\n", (int)uiFilled, (int)cScene.macMeshes.NumElements(), szMax);
			bResult = false;
		}
	}
	if (bResult && szParticles)
	{
		bResult = ReadOldDragonParticles(szParticles, &cScene);
	}
	bResult = bResult && WriteOldDragonSceneFile(&cScene, szNew);
	bResult = bResult && VerifyOldDragonSceneFile(&cScene, szNew);

	if (bResult)
	{
		printf("%s: %d meshes, %d images, %d materials, %d connections, %d sequences, %d particles, %d old chunks skipped.\n", szNew, (int)cScene.macMeshes.NumElements(), (int)cScene.macImages.NumElements(),
			(int)cScene.macMaterials.NumElements(), (int)cScene.macConnections.NumElements(), (int)cScene.macSequences.NumElements(), (int)cScene.macParticles.NumElements(), (int)cScene.muiSkippedChunks);
	}
	else
	{
		printf("Could not convert [%s].\n", szOld);
	}
	cScene.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char* argv[])
{
	const char*	szMax;
	const char*	szParticles;
	int			i;
	bool		bResult;

	szMax = NULL;
	szParticles = NULL;
	for (i = 3; i + 1 < argc; i += 2)
	{
		if (strcmp(argv[i], "--max") == 0)
		{
			szMax = argv[i + 1];
		}
		else if (strcmp(argv[i], "--particles") == 0)
		{
			szParticles = argv[i + 1];
		}
		else
		{
			argc = 0;
		}
	}
	if ((argc < 3) || (i != argc))
	{
		printf("ConvertOldDragon <old .DRG> <new .DRG> [--max <.max>] [--particles <.csv>]\n");
		return 1;
	}

	TypesInit();
	TypeConverterInit();
	MemoryInit();

	bResult = Convert(argv[1], argv[2], szMax, szParticles);

	MemoryKill();
	TypeConverterKill();
	TypesKill();
	LeakDetection();
	return bResult ? 0 : 1;
}

