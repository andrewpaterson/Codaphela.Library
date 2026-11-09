#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "OldDragonLib/OldDragonReader.h"
#include "TestLib/Assert.h"


#define IOX_MESHES		".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "IOX2021" _FS_ "data" _FS_ "meshes" _FS_
#define MELEE			".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "StarControl1" _FS_ "StarControl1" _FS_ "Melee.DRG"
#define STAR_CONTROL	"D:/Work/Star Control/Builds/StarControl/Test.DRG"
#define GOOBER			"D:/Work/Star Control/Builds/ViewerV7beta2009.9.20/Goober.DRG"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertOldDragonVec3(float32 x, float32 y, float32 z, SOldDragonVec3* psVec3)
{
	AssertFloat(x, psVec3->x, 3);
	AssertFloat(y, psVec3->y, 3);
	AssertFloat(z, psVec3->z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertOldDragonFace(int32 i0, int32 i1, int32 i2, SOldDragonFace* psFace)
{
	AssertInt(i0, psFace->aiPositions[0]);
	AssertInt(i1, psFace->aiPositions[1]);
	AssertInt(i2, psFace->aiPositions[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonFileVersions(void)
{
	COldDragonFile		cFile;
	SOldDragonChunk*	psChunk;

	AssertTrue(cFile.Init("Input" _FS_ "Laser.DRG"));
	AssertInt(ODV_IOX, cFile.GetVersion());
	AssertFalse(cFile.IsDamaged());
	AssertSize(4, cFile.NumChunks());
	psChunk = cFile.GetChunk(1);
	AssertInt(6, psChunk->uiName);
	AssertSize(920, psChunk->uiDataSize);
	AssertNull((void*)cFile.GetChunkName(psChunk));
	cFile.Kill();

	AssertTrue(cFile.Init("Input" _FS_ "Box.DRG"));
	AssertInt(ODV_August2009, cFile.GetVersion());
	AssertFalse(cFile.IsDamaged());
	psChunk = cFile.GetChunk(1);
	AssertInt(26, psChunk->uiName);
	AssertInt(0, psChunk->iParent);
	AssertSize(284, psChunk->uiDataSize);
	cFile.Kill();

	AssertTrue(cFile.Init("Input" _FS_ "Empty.DRG"));
	AssertInt(ODV_August2009, cFile.GetVersion());
	AssertSize(1, cFile.NumChunks());
	AssertSize(0, cFile.GetChunk(0)->uiDataSize);
	cFile.Kill();

	AssertTrue(cFile.Init("Input" _FS_ "Plane.DRG"));
	AssertInt(ODV_Max, cFile.GetVersion());
	AssertFalse(cFile.IsDamaged());
	AssertSize(4, cFile.NumChunks());
	AssertString("CConnection", cFile.GetChunkName(cFile.GetChunk(1)));
	AssertString("CMesh", cFile.GetChunkName(cFile.GetChunk(2)));
	AssertString("CInstance", cFile.GetChunkName(cFile.GetChunk(3)));
	AssertSize(204, cFile.GetChunk(1)->uiDataSize);
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The expected values are from parsing the file separately.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadIOX(void)
{
	COldDragonScene			cScene;
	COldDragonMesh*			pcMesh;
	SOldDragonMaterial*		psMaterial;
	COldDragonInstance*		pcInstance;
	SOldDragonConnection*	psConnection;

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Laser.DRG", &cScene));
	AssertInt(ODV_IOX, cScene.meVersion);
	AssertSize(0, cScene.muiSkippedChunks);

	AssertSize(1, cScene.macMeshes.NumElements());
	pcMesh = cScene.macMeshes.Get(0);
	AssertString("Cylinder01", pcMesh->mszName);
	AssertInt(0, pcMesh->miID);
	AssertSize(18, pcMesh->macPositions.NumElements());
	AssertSize(32, pcMesh->macFaces.NumElements());
	AssertSize(0, pcMesh->macNormals.NumElements());
	AssertSize(0, pcMesh->macUVs.NumElements());
	AssertOldDragonVec3(0.0f, -50.0f, 0.0f, pcMesh->macPositions.Get(0));

	AssertSize(1, cScene.macMaterials.NumElements());
	psMaterial = cScene.macMaterials.Get(0);
	AssertString("20 - Default", psMaterial->szName);
	AssertInt(0xffffffffu, psMaterial->uiDiffuseColour);
	AssertInt(4293256677u, psMaterial->uiSpecularColour);

	AssertSize(1, cScene.macInstances.NumElements());
	pcInstance = cScene.macInstances.Get(0);
	AssertString("Cylinder01Link", pcInstance->mszName);
	AssertSize(1, pcInstance->maiConnections.NumElements());

	AssertSize(1, cScene.macConnections.NumElements());
	psConnection = cScene.macConnections.Get(0);
	AssertString("Cylinder01Link", psConnection->szName);
	AssertTrue(psConnection->bTopLevel);
	AssertFloat(50.0f, psConnection->sWorld.af[13], 3);
	AssertFloat(1.0f, psConnection->sWorld.af[15], 3);

	AssertSize(0, cScene.macImages.NumElements());
	AssertSize(0, cScene.macLights.NumElements());
	AssertSize(0, cScene.macCameras.NumElements());
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadAugust2009(void)
{
	COldDragonScene			cScene;
	COldDragonMesh*			pcMesh;
	SOldDragonConnection*	psConnection;
	COldDragonImage*		pcImage;
	SOldDragonCamera*		psCamera;
	size					i;

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Box.DRG", &cScene));
	AssertInt(ODV_August2009, cScene.meVersion);

	AssertSize(1, cScene.macMeshes.NumElements());
	pcMesh = cScene.macMeshes.Get(0);
	AssertString("Box01", pcMesh->mszName);
	AssertSize(8, pcMesh->macPositions.NumElements());
	AssertOldDragonVec3(-104.032f, 0.0f, 119.892f, pcMesh->macPositions.Get(0));
	AssertOldDragonVec3(104.032f, 190.111f, -119.892f, pcMesh->macPositions.Get(7));

	//A box: twelve triangles, with every vertex used and each triangle having three different corners.
	AssertSize(12, pcMesh->macFaces.NumElements());
	AssertOldDragonFace(0, 1, 3, pcMesh->macFaces.Get(0));
	AssertOldDragonFace(3, 2, 0, pcMesh->macFaces.Get(1));
	AssertOldDragonFace(4, 6, 7, pcMesh->macFaces.Get(2));
	for (i = 0; i < 12; i++)
	{
		AssertTrue(pcMesh->macFaces.Get(i)->aiPositions[0] != pcMesh->macFaces.Get(i)->aiPositions[1]);
		AssertTrue(pcMesh->macFaces.Get(i)->aiPositions[1] != pcMesh->macFaces.Get(i)->aiPositions[2]);
		AssertTrue(pcMesh->macFaces.Get(i)->aiPositions[2] != pcMesh->macFaces.Get(i)->aiPositions[0]);
	}

	AssertSize(2, cScene.macConnections.NumElements());
	psConnection = cScene.macConnections.Get(0);
	AssertString("Box01", psConnection->szName);
	AssertTrue(psConnection->bTopLevel);
	AssertFalse(psConnection->bSelected);
	AssertFalse(psConnection->bParity);
	AssertFloat(1.0f, psConnection->sWorld.af[15], 3);
	AssertString("Camera01", cScene.macConnections.Get(1)->szName);

	AssertSize(1, cScene.macCameras.NumElements());
	psCamera = cScene.macCameras.Get(0);
	AssertString("Camera01", psCamera->szName);
	AssertFloat(45.0f, psCamera->fFOV, 3);

	AssertSize(1, cScene.macImages.NumElements());
	pcImage = cScene.macImages.Get(0);
	AssertString("basn6a16.png", pcImage->mszName);
	AssertInt(32, pcImage->miWidth);
	AssertInt(32, pcImage->miHeight);
	AssertInt(4, pcImage->miChannels);
	AssertNotNull(pcImage->mpuiPixels);

	AssertSize(1, cScene.macMaterials.NumElements());
	AssertString("01 - Default", cScene.macMaterials.Get(0)->szName);
	AssertInt(9, cScene.macMaterials.Get(0)->iNumTextures);
	AssertInt(0, cScene.macMaterials.Get(0)->aiTextures[0]);
	AssertInt(OLD_DRAGON_NO_INDEX, cScene.macMaterials.Get(0)->aiTextures[1]);

	AssertSize(2, cScene.macInstances.NumElements());
	AssertInt(OLD_DRAGON_2009_INSTANCE_MESH, cScene.macInstances.Get(0)->miType);
	AssertInt(0, cScene.macInstances.Get(0)->miObject);
	AssertSize(1, cScene.macInstances.Get(0)->maiConnections.NumElements());
	AssertInt(0, *cScene.macInstances.Get(0)->maiConnections.Get(0));
	AssertInt(OLD_DRAGON_2009_INSTANCE_CAMERA, cScene.macInstances.Get(1)->miType);
	AssertInt(1, *cScene.macInstances.Get(1)->maiConnections.Get(0));
	AssertSize(0, cScene.muiSkippedChunks);
	cScene.Kill();

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Empty.DRG", &cScene));
	AssertSize(0, cScene.macMeshes.NumElements());
	AssertSize(0, cScene.macConnections.NumElements());
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadMax(void)
{
	COldDragonScene			cScene;
	COldDragonMesh*			pcMesh;
	SOldDragonConnection*	psConnection;
	COldDragonInstance*		pcInstance;

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Plane.DRG", &cScene));
	AssertInt(ODV_Max, cScene.meVersion);
	AssertFalse(cScene.mbDamaged);

	AssertSize(1, cScene.macConnections.NumElements());
	psConnection = cScene.macConnections.Get(0);
	AssertString("Plane01", psConnection->szName);
	AssertFalse(psConnection->bSelected);
	AssertTrue(psConnection->bTopLevel);
	AssertFalse(psConnection->bParity);
	AssertFloat(1.0f, psConnection->sWorld.af[0], 3);
	AssertFloat(1.0f, psConnection->sLocal.af[15], 3);

	//A two by two plane: nine positions and eight triangles.
	AssertSize(1, cScene.macMeshes.NumElements());
	pcMesh = cScene.macMeshes.Get(0);
	AssertSize(9, pcMesh->macPositions.NumElements());
	AssertOldDragonVec3(0.0f, 0.0f, 0.0f, pcMesh->macPositions.Get(0));
	AssertOldDragonVec3(0.5f, 0.0f, 0.0f, pcMesh->macPositions.Get(1));
	AssertOldDragonVec3(1.0f, 0.0f, 0.0f, pcMesh->macPositions.Get(2));
	AssertOldDragonVec3(0.0f, 0.0f, -0.5f, pcMesh->macPositions.Get(3));
	AssertSize(8, pcMesh->macFaces.NumElements());
	AssertOldDragonFace(0, 1, 4, pcMesh->macFaces.Get(0));

	AssertSize(1, cScene.macInstances.NumElements());
	pcInstance = cScene.macInstances.Get(0);
	AssertInt(1, pcInstance->miType);
	AssertInt(0, pcInstance->miObject);
	AssertSize(1, pcInstance->maiConnections.NumElements());
	AssertInt(0, *pcInstance->maiConnections.Get(0));
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Melee.DRG's 64 meshes were never closed.  Everything else in it is
// still there and has to be found without the index.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadDamagedMax(void)
{
	COldDragonScene		cScene;
	CFileUtil			cFileUtil;
	size				i;
	size				uiWithPixels;
	COldDragonSequence*	pcSequence;

	if (!cFileUtil.Exists(MELEE))
	{
		return;
	}

	cScene.Init();
	AssertTrue(ReadOldDragonFile(MELEE, &cScene));
	AssertInt(ODV_Max, cScene.meVersion);
	AssertTrue(cScene.mbDamaged);
	AssertSize(86, cScene.macConnections.NumElements());
	AssertSize(64, cScene.macSequences.NumElements());
	AssertSize(199, cScene.macMaterials.NumElements());
	AssertSize(13, cScene.macImages.NumElements());
	AssertSize(64, cScene.macMeshes.NumElements());
	AssertSize(1, cScene.macLights.NumElements());
	AssertSize(1, cScene.macCameras.NumElements());
	AssertSize(86, cScene.macInstances.NumElements());

	AssertSize(1704, cScene.macMeshes.Get(0)->macPositions.NumElements());
	AssertSize(2331, cScene.macMeshes.Get(1)->macPositions.NumElements());
	for (i = 0; i < cScene.macMeshes.NumElements(); i++)
	{
		AssertTrue(cScene.macMeshes.Get(i)->macPositions.NumElements() > 0);
		AssertSize(0, cScene.macMeshes.Get(i)->macFaces.NumElements());
	}
	AssertString("FDirect01", cScene.macLights.Get(0)->szName);
	AssertString("Ur-Quan Camera", cScene.macCameras.Get(0)->szName);
	AssertFloat(45.0f, cScene.macCameras.Get(0)->fFOV, 3);

	pcSequence = cScene.macSequences.Get(0);
	AssertString("Androsynth Guardian", pcSequence->mszName);
	AssertSize(143, pcSequence->macKeyFrames.NumElements());
	AssertFloat(480.0f, pcSequence->macKeyFrames.Get(0)->sMatrix.af[12], 3);

	uiWithPixels = 0;
	for (i = 0; i < cScene.macImages.NumElements(); i++)
	{
		if (cScene.macImages.Get(i)->mpuiPixels != NULL)
		{
			uiWithPixels++;
		}
	}
	AssertSize(13, uiWithPixels);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MultiplyOldDragonMatrices(SOldDragonMatrix* psDest, SOldDragonMatrix* psLeft, SOldDragonMatrix* psRight)
{
	int32	iRow;
	int32	iColumn;
	int32	k;
	float32	fSum;

	for (iRow = 0; iRow < 4; iRow++)
	{
		for (iColumn = 0; iColumn < 4; iColumn++)
		{
			fSum = 0.0f;
			for (k = 0; k < 4; k++)
			{
				fSum += psLeft->af[iRow * 4 + k] * psRight->af[k * 4 + iColumn];
			}
			psDest->af[iRow * 4 + iColumn] = fSum;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the sequence for a connection, as a sequence's miConnection and
// an instance's maiConnections both index the scene's connections.
//
//////////////////////////////////////////////////////////////////////////
static COldDragonSequence* FindOldDragonSequence(COldDragonScene* pcScene, int32 iConnection)
{
	size	i;

	for (i = 0; i < pcScene->macSequences.NumElements(); i++)
	{
		if (pcScene->macSequences.Get(i)->miConnection == iConnection)
		{
			return pcScene->macSequences.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Poses the mesh at a key frame and returns how much of its total edge
// length survived, and where the pose's lowest and highest z are.  The
// matrices only leave the edges alone if the skin has been read properly:
// a wrong order or a wrong bone tears the mesh apart.
//
//////////////////////////////////////////////////////////////////////////
static float32 SkinOldDragonMesh(COldDragonScene* pcScene, COldDragonMesh* pcMesh, COldDragonInstance* pcInstance, size uiKeyFrame, float32* pfMinZ, float32* pfMaxZ)
{
	CArrayTemplate<SOldDragonMatrix>	asPalette;
	CArrayTemplate<SOldDragonVec3>		asSkinned;
	COldDragonSequence*					pcSequence;
	SOldDragonMatrix*					psMatrix;
	SOldDragonVec3*						psPosition;
	SOldDragonVec3*						psSkinned;
	SOldDragonSkinVert*					psSkinVert;
	SOldDragonSkinWeight*				psWeight;
	SOldDragonFace*						psFace;
	SOldDragonVec3*						psA;
	SOldDragonVec3*						psB;
	SOldDragonVec3*						psRestA;
	SOldDragonVec3*						psRestB;
	size								i;
	int32								j;
	float32								x;
	float32								y;
	float32								z;
	float64								fSkinnedLength;
	float64								fRestLength;

	asPalette.Init();
	for (i = 0; i < pcInstance->maiConnections.NumElements(); i++)
	{
		pcSequence = FindOldDragonSequence(pcScene, *pcInstance->maiConnections.Get(i));
		AssertNotNull(pcSequence);
		MultiplyOldDragonMatrices(asPalette.Add(), pcMesh->macInverseSkinMatrices.Get(i), &pcSequence->macKeyFrames.Get(uiKeyFrame)->sMatrix);
	}

	asSkinned.Init();
	*pfMinZ = 1e30f;
	*pfMaxZ = -1e30f;
	for (i = 0; i < pcMesh->macPositions.NumElements(); i++)
	{
		psPosition = pcMesh->macPositions.Get(i);
		psSkinVert = pcMesh->macSkinVerts.Get(i);
		psSkinned = asSkinned.Add();
		psSkinned->x = 0.0f;
		psSkinned->y = 0.0f;
		psSkinned->z = 0.0f;
		for (j = 0; j < psSkinVert->iNumWeights; j++)
		{
			psWeight = pcMesh->macSkinWeights.Get((size)(psSkinVert->iFirstWeight + j));
			psMatrix = asPalette.Get((size)psWeight->iMatrix);
			x = psPosition->x * psMatrix->af[0] + psPosition->y * psMatrix->af[4] + psPosition->z * psMatrix->af[8] + psMatrix->af[12];
			y = psPosition->x * psMatrix->af[1] + psPosition->y * psMatrix->af[5] + psPosition->z * psMatrix->af[9] + psMatrix->af[13];
			z = psPosition->x * psMatrix->af[2] + psPosition->y * psMatrix->af[6] + psPosition->z * psMatrix->af[10] + psMatrix->af[14];
			psSkinned->x += x * psWeight->fWeight;
			psSkinned->y += y * psWeight->fWeight;
			psSkinned->z += z * psWeight->fWeight;
		}
		if (psSkinned->z < *pfMinZ)
		{
			*pfMinZ = psSkinned->z;
		}
		if (psSkinned->z > *pfMaxZ)
		{
			*pfMaxZ = psSkinned->z;
		}
	}

	fSkinnedLength = 0.0;
	fRestLength = 0.0;
	for (i = 0; i < pcMesh->macFaces.NumElements(); i++)
	{
		psFace = pcMesh->macFaces.Get(i);
		for (j = 0; j < 3; j++)
		{
			psA = asSkinned.Get((size)psFace->aiPositions[j]);
			psB = asSkinned.Get((size)psFace->aiPositions[(j + 1) % 3]);
			psRestA = pcMesh->macPositions.Get((size)psFace->aiPositions[j]);
			psRestB = pcMesh->macPositions.Get((size)psFace->aiPositions[(j + 1) % 3]);
			fSkinnedLength += sqrt((psA->x - psB->x) * (psA->x - psB->x) + (psA->y - psB->y) * (psA->y - psB->y) + (psA->z - psB->z) * (psA->z - psB->z));
			fRestLength += sqrt((psRestA->x - psRestB->x) * (psRestA->x - psRestB->x) + (psRestA->y - psRestB->y) * (psRestA->y - psRestB->y) + (psRestA->z - psRestB->z) * (psRestA->z - psRestB->z));
		}
	}

	asSkinned.Kill();
	asPalette.Kill();
	return (float32)(fSkinnedLength / fRestLength);
}


//////////////////////////////////////////////////////////////////////////
//
// A skinned mesh walking, which is the most any of these files holds.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadGoober(void)
{
	COldDragonScene		cScene;
	CFileUtil			cFileUtil;
	COldDragonMesh*		pcMesh;
	COldDragonInstance*	pcInstance;
	SOldDragonMaterial*	psMaterial;
	COldDragonSequence*	pcSequence;
	SOldDragonSkinVert*	psSkinVert;
	SOldDragonFace*		psFace;
	size				i;
	int32				j;
	float32				fSum;
	float32				fTime;
	float32				fKept;
	float32				fMinZ;
	float32				fMaxZ;
	float32				fStartMinZ;
	float32				fStartMaxZ;
	float32				fStrideMinZ;
	float32				fStrideMaxZ;

	if (!cFileUtil.Exists(GOOBER))
	{
		return;
	}

	cScene.Init();
	AssertTrue(ReadOldDragonFile(GOOBER, &cScene));
	AssertInt(ODV_September2009, cScene.meVersion);
	AssertSize(0, cScene.muiSkippedChunks);

	AssertSize(1, cScene.macMeshes.NumElements());
	pcMesh = cScene.macMeshes.Get(0);
	AssertString("goober_", pcMesh->mszName);
	AssertSize(3241, pcMesh->macPositions.NumElements());
	AssertSize(6134, pcMesh->macFaces.NumElements());
	AssertSize(3997, pcMesh->macUVs.NumElements());
	AssertOldDragonVec3(14.98883f, 7.75423f, -2.49701f, pcMesh->macPositions.Get(0));

	psFace = pcMesh->macFaces.Get(0);
	AssertOldDragonFace(0, 1, 2, psFace);
	AssertInt(0, psFace->aiUVs[0]);
	AssertInt(1, psFace->aiUVs[1]);
	AssertInt(2, psFace->aiUVs[2]);
	AssertInt(1, psFace->uiSmoothing);
	AssertInt(0, psFace->iMaterial);
	for (i = 0; i < pcMesh->macFaces.NumElements(); i++)
	{
		for (j = 0; j < 3; j++)
		{
			AssertTrue(pcMesh->macFaces.Get(i)->aiUVs[j] != OLD_DRAGON_NO_INDEX);
		}
	}

	//Every weight in the file belongs to exactly one position and every position's weights add up to one.
	AssertSize(3241, pcMesh->macSkinVerts.NumElements());
	AssertSize(8124, pcMesh->macSkinWeights.NumElements());
	AssertSize(67, pcMesh->macInverseSkinMatrices.NumElements());
	for (i = 0; i < pcMesh->macSkinVerts.NumElements(); i++)
	{
		psSkinVert = pcMesh->macSkinVerts.Get(i);
		AssertTrue((psSkinVert->iNumWeights >= 1) && (psSkinVert->iNumWeights <= 4));
		fSum = 0.0f;
		for (j = 0; j < psSkinVert->iNumWeights; j++)
		{
			fSum += pcMesh->macSkinWeights.Get((size)(psSkinVert->iFirstWeight + j))->fWeight;
		}
		AssertFloat(1.0f, fSum, 2);
	}

	AssertSize(1, cScene.macInstances.NumElements());
	pcInstance = cScene.macInstances.Get(0);
	AssertInt(OLD_DRAGON_2009_INSTANCE_MESH, pcInstance->miType);
	AssertInt(0, pcInstance->miObject);
	AssertSize(67, pcInstance->maiConnections.NumElements());
	AssertSize(141, cScene.macConnections.NumElements());
	AssertString("goober_", cScene.macConnections.Get(0)->szName);
	AssertString("bone_goober_00", cScene.macConnections.Get((size)*pcInstance->maiConnections.Get(0))->szName);
	AssertString("bone_goober_01", cScene.macConnections.Get((size)*pcInstance->maiConnections.Get(1))->szName);

	AssertSize(1, cScene.macMaterials.NumElements());
	psMaterial = cScene.macMaterials.Get(0);
	AssertString("01 - Default", psMaterial->szName);
	AssertInt(9, psMaterial->iNumTextures);
	AssertInt(0, psMaterial->aiTextures[0]);
	AssertInt(1, psMaterial->aiTextures[5]);
	AssertInt(2, psMaterial->aiTextures[8]);
	AssertInt(OLD_DRAGON_NO_INDEX, psMaterial->aiTextures[1]);

	AssertSize(3, cScene.macImages.NumElements());
	AssertString("Goober_D.tga", cScene.macImages.Get(0)->mszName);
	AssertString("goober_n.tga", cScene.macImages.Get(1)->mszName);
	AssertInt(2048, cScene.macImages.Get(0)->miWidth);
	AssertInt(2048, cScene.macImages.Get(0)->miHeight);
	for (i = 0; i < cScene.macImages.NumElements(); i++)
	{
		AssertNotNull(cScene.macImages.Get(i)->mpuiPixels);
	}

	AssertSize(140, cScene.macSequences.NumElements());
	pcSequence = FindOldDragonSequence(&cScene, *pcInstance->maiConnections.Get(0));
	AssertString("bone_goober_00", pcSequence->mszName);
	AssertSize(81, pcSequence->macKeyFrames.NumElements());
	fTime = 0.0f;
	for (i = 1; i < pcSequence->macKeyFrames.NumElements() - 1; i++)
	{
		AssertFloat(1.0f / 60.0f, pcSequence->macKeyFrames.Get(i)->fTime, 3);
		fTime += pcSequence->macKeyFrames.Get(i)->fTime;
	}
	AssertTrue(fTime > 1.0f);

	//The walk: while a leg swings out and comes back, the mesh's edges add up to nearly what they did.  Joints do stretch, by
	//up to 3.3% at the longest stride, where a scrambled skin is far outside this.
	fKept = SkinOldDragonMesh(&cScene, pcMesh, pcInstance, 0, &fStartMinZ, &fStartMaxZ);
	AssertTrue((fKept > 0.95f) && (fKept < 1.05f));
	fKept = SkinOldDragonMesh(&cScene, pcMesh, pcInstance, 20, &fStrideMinZ, &fStrideMaxZ);
	AssertTrue((fKept > 0.95f) && (fKept < 1.05f));
	AssertTrue(fStrideMinZ < fStartMinZ - 20.0f);
	AssertTrue(fStrideMaxZ > fStartMaxZ + 10.0f);
	fKept = SkinOldDragonMesh(&cScene, pcMesh, pcInstance, 40, &fMinZ, &fMaxZ);
	AssertTrue((fKept > 0.95f) && (fKept < 1.05f));
	fKept = SkinOldDragonMesh(&cScene, pcMesh, pcInstance, 80, &fMinZ, &fMaxZ);
	AssertTrue((fKept > 0.95f) && (fKept < 1.05f));
	AssertTrue(fabsf(fMinZ - fStartMinZ) < 2.0f);
	AssertTrue(fabsf(fMaxZ - fStartMaxZ) < 2.0f);

	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// These are too big to keep with the test.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReadLargeFiles(void)
{
	COldDragonScene		cScene;
	CFileUtil			cFileUtil;
	size				i;
	static const char*	aszIOX[] =
	{
		"Blade.DRG", "Bomber.DRG", "Boss.DRG", "Bullets.DRG", "Dragon.DRG", "LevelOne.DRG", "LevelOneCollision.DRG",
		"LevelOneCollisiona.DRG", "Player.DRG", "PowerUps.DRG", "Rocket.DRG", "Slider.DRG", "Spinner.DRG", "Turret.DRG"
	};
	CChars				szPath;

	for (i = 0; i < sizeof(aszIOX) / sizeof(aszIOX[0]); i++)
	{
		szPath.Init(IOX_MESHES);
		szPath.Append(aszIOX[i]);
		if (cFileUtil.Exists(szPath.Text()))
		{
			cScene.Init();
			AssertTrue(ReadOldDragonFile(szPath.Text(), &cScene));
			AssertInt(ODV_IOX, cScene.meVersion);
			AssertSize(0, cScene.muiSkippedChunks);
			AssertTrue(cScene.macMeshes.NumElements() > 0);
			cScene.Kill();
		}
		szPath.Kill();
	}

	if (cFileUtil.Exists(STAR_CONTROL))
	{
		cScene.Init();
		AssertTrue(ReadOldDragonFile(STAR_CONTROL, &cScene));
		AssertInt(ODV_August2009, cScene.meVersion);
		AssertSize(4, cScene.macMeshes.NumElements());
		AssertSize(19, cScene.macMaterials.NumElements());
		AssertSize(6, cScene.macConnections.NumElements());
		AssertSize(6, cScene.macSequences.NumElements());
		AssertSize(1, cScene.macCameras.NumElements());
		AssertSize(5, cScene.macInstances.NumElements());
		AssertString("Life Planet", cScene.macConnections.Get(0)->szName);
		AssertInt(OLD_DRAGON_2009_INSTANCE_MESH, cScene.macInstances.Get(3)->miType);
		AssertInt(3, cScene.macInstances.Get(3)->miObject);
		AssertInt(3, *cScene.macInstances.Get(3)->maiConnections.Get(0));
		AssertInt(OLD_DRAGON_2009_INSTANCE_CAMERA, cScene.macInstances.Get(4)->miType);
		AssertInt(5, *cScene.macInstances.Get(4)->maiConnections.Get(0));
		cScene.Kill();
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonNotOldOrMissing(void)
{
	COldDragonFile	cFile;
	SLogConfig		sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cFile.Init("Input" _FS_ "NoSuchFile.DRG"));
	cFile.Kill();

	//Today's files share the extension and have their own readers.
	AssertFalse(cFile.Init(".." _FS_ ".." _FS_ "TestStandardLib" _FS_ "TestStandardLib" _FS_ "Input" _FS_ "OneA.DRG"));
	AssertInt(ODV_Current, cFile.GetVersion());
	cFile.Kill();
	AssertFalse(cFile.Init(".." _FS_ ".." _FS_ "TestStandardLib" _FS_ "TestStandardLib" _FS_ "Input" _FS_ "ChunkFile.DRG"));
	AssertInt(ODV_Current, cFile.GetVersion());
	cFile.Kill();
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ReadDamagedOldDragonFile(const char* szOutput, uint8* puiData, size uiSize)
{
	FILE*			pvFile;
	COldDragonScene	cScene;

	pvFile = fopen(szOutput, "wb");
	AssertNotNull(pvFile);
	fwrite(puiData, 1, uiSize, pvFile);
	fclose(pvFile);

	cScene.Init();
	ReadOldDragonFile(szOutput, &cScene);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether these read isn't the point, only that a bad file doesn't take
// the reader down or leak.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonDamagedFiles(void)
{
	static const char*	aszFiles[] = { "Input" _FS_ "Laser.DRG", "Input" _FS_ "Box.DRG", "Input" _FS_ "Plane.DRG" };
	static const uint8	auiValues[] = { 0x00, 0xff, 0x7f };
	CFileUtil			cFileUtil;
	SLogConfig			sLogConfig;
	FILE*				pvFile;
	uint8*				puiData;
	size				uiSize;
	size				i;
	size				uiPos;
	size				uiValue;
	uint8				uiOriginal;

	cFileUtil.TouchDir("Output");

	sLogConfig = gcLogger.SetSilent();
	for (i = 0; i < sizeof(aszFiles) / sizeof(aszFiles[0]); i++)
	{
		pvFile = fopen(aszFiles[i], "rb");
		AssertNotNull(pvFile);
		fseek(pvFile, 0, SEEK_END);
		uiSize = (size)ftell(pvFile);
		fseek(pvFile, 0, SEEK_SET);
		puiData = (uint8*)malloc(uiSize);
		AssertSize(uiSize, fread(puiData, 1, uiSize, pvFile));
		fclose(pvFile);

		//Cut short.
		for (uiPos = 0; uiPos < uiSize; uiPos += 61)
		{
			ReadDamagedOldDragonFile("Output" _FS_ "Damaged.DRG", puiData, uiPos);
		}

		//With bad values, which is hardest on headers and indexes near the start and end.
		for (uiPos = 0; uiPos < uiSize; uiPos += (uiPos < 128) ? 1 : 37)
		{
			uiOriginal = puiData[uiPos];
			for (uiValue = 0; uiValue < sizeof(auiValues); uiValue++)
			{
				puiData[uiPos] = auiValues[uiValue];
				ReadDamagedOldDragonFile("Output" _FS_ "Damaged.DRG", puiData, uiSize);
			}
			puiData[uiPos] = uiOriginal;
		}
		free(puiData);
	}
	gcLogger.SetConfig(&sLogConfig);
	cFileUtil.Delete("Output" _FS_ "Damaged.DRG");
}


//////////////////////////////////////////////////////////////////////////
//
// There's no small September file to keep with the test, and at 36MB this
// one can only be damaged in a few places.  They're in the mesh, as that's
// where a September file differs from an August one.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonDamagedSeptember(void)
{
	static const uint8	auiValues[] = { 0x00, 0xff };
	static const size	auiStarts[] = { 0x20db7c0, 0x20db820, 0x212b5d4, 0x2244eac, 0x2254c3c, 0x225ac34, 0x226a624, 0x228a204, 0x228b4fc };
	CFileUtil			cFileUtil;
	SLogConfig			sLogConfig;
	FILE*				pvFile;
	uint8*				puiData;
	size				uiSize;
	size				i;
	size				j;
	size				uiValue;
	size				uiPos;
	uint8				uiOriginal;

	if (!cFileUtil.Exists(GOOBER))
	{
		return;
	}

	cFileUtil.TouchDir("Output");
	pvFile = fopen(GOOBER, "rb");
	AssertNotNull(pvFile);
	fseek(pvFile, 0, SEEK_END);
	uiSize = (size)ftell(pvFile);
	fseek(pvFile, 0, SEEK_SET);
	puiData = (uint8*)malloc(uiSize);
	AssertSize(uiSize, fread(puiData, 1, uiSize, pvFile));
	fclose(pvFile);

	sLogConfig = gcLogger.SetSilent();
	for (i = 0; i < sizeof(auiStarts) / sizeof(auiStarts[0]); i++)
	{
		//The first few bytes hold each chunk's header, or an array's tag and count, which is what a reader trusts most.
		for (j = 0; j < 12; j += 4)
		{
			uiPos = auiStarts[i] + j;
			uiOriginal = puiData[uiPos];
			for (uiValue = 0; uiValue < sizeof(auiValues); uiValue++)
			{
				puiData[uiPos] = auiValues[uiValue];
				ReadDamagedOldDragonFile("Output" _FS_ "Damaged.DRG", puiData, uiSize);
			}
			puiData[uiPos] = uiOriginal;
		}

		ReadDamagedOldDragonFile("Output" _FS_ "Damaged.DRG", puiData, auiStarts[i] + 40);
	}
	gcLogger.SetConfig(&sLogConfig);

	free(puiData);
	cFileUtil.Delete("Output" _FS_ "Damaged.DRG");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonReader(void)
{
	BeginTests();

	TestOldDragonFileVersions();
	TestOldDragonReadIOX();
	TestOldDragonReadAugust2009();
	TestOldDragonReadMax();
	TestOldDragonReadDamagedMax();
	TestOldDragonReadGoober();
	TestOldDragonReadLargeFiles();
	TestOldDragonNotOldOrMissing();
	TestOldDragonDamagedFiles();
	TestOldDragonDamagedSeptember();

	TestStatistics();
}

