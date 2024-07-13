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
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshTriangles.h"
#include "MeshConnectivity.h"
#include "MeshPositions.h"
#include "MeshLeaves.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRelativeSphereVolume::Init(float fVolume)
{
	this->fVolume = fVolume;
	eRelative[0] = RSV_Unknown;
	eRelative[1] = RSV_Unknown;
	eRelative[2] = RSV_Unknown;
	iNewIndex = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRelativeSphereVolume::IsAllGreaterThan(void)
{
	int		i;

	for (i = 0; i < 3; i++)
	{
		if (!((eRelative[i] == RSV_GreaterThan) || (eRelative[i] == RSV_Unknown)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRelativeSphereVolume::IsAllEqual(void)
{
	int		i;

	for (i = 0; i < 3; i++)
	{
		if (!(eRelative[i] == RSV_Equal))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRelativeSphereVolume::IsGreaterThanOrEqual(int iNumGreaterThan)
{
	int		i;
	int		iCount;
	int		iTotal;

	iCount = 0;
	iTotal = 0;
	for (i = 0; i < 3; i++)
	{
		if (eRelative[i] != RSV_Unknown)
		{
			if (eRelative[i] == RSV_GreaterThan)
			{
				iCount++;
			}
			if (eRelative[i] == RSV_LessThan)
			{
				return false;
			}
			iTotal++;
		}
	}
	if (iNumGreaterThan > iTotal)
	{
		iNumGreaterThan = iTotal;
	}
	if (iCount >= iNumGreaterThan)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRelativeSphereVolume::IsEqual(int iNumEqual)
{
	int		i;
	int		iCount;
	int		iTotal;

	iCount = 0;
	iTotal = 0;
	for (i = 0; i < 3; i++)
	{
		if (eRelative[i] != RSV_Unknown)
		{
			if (eRelative[i] == RSV_Equal)
			{
				iCount++;
			}
			if (eRelative[i] == RSV_LessThan)
			{
				return false;
			}
			iTotal++;
		}
	}
	if (iNumEqual > iTotal)
	{
		iNumEqual = iTotal;
	}
	if (iCount >= iNumEqual)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaf::Init(void)
{
	mcSphere.Init();
	maiTriangles.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaf::Kill(void)
{
	maiTriangles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshLeaf::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(mcSphere.Save(pcFile));
	ReturnOnFalse(maiTriangles.Write(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshLeaf::Load(CFileReader* pcFile)
{
	ReturnOnFalse(mcSphere.Load(pcFile));
	ReturnOnFalse(maiTriangles.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcLeaves.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::Free(void)
{
	CMeshLeaf*	pcLeaf;
	size		i;

	for (i = 0; i < mcLeaves.NumElements(); i++)
	{
		pcLeaf = mcLeaves.Get(i);
		pcLeaf->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::Class(void)
{
	CMeshDetail::Class();

	U_Unknown(CArrayMeshLeaf, mcLeaves);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshLeaves::Save(CObjectWriter* pcFile)
{
	size		i;
	CMeshLeaf*	pcLeaf;

	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(mcLeaves.WriteAllocatorAndHeader(pcFile));
	for (i = 0; i < mcLeaves.NumElements(); i++)
	{
		pcLeaf = mcLeaves.Get(i);
		ReturnOnFalse(pcLeaf->Save(pcFile));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshLeaves::Load(CObjectReader* pcFile)
{
	size		i;
	CMeshLeaf*	pcLeaf;

	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(mcLeaves.ReadAllocatorAndHeader(pcFile));
	for (i = 0; i < mcLeaves.NumElements(); i++)
	{
		pcLeaf = mcLeaves.Get(i);
		ReturnOnFalse(pcLeaf->Load(pcFile));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::PrivateAddLeafSphere(int iFaceNum, CMeshConnectivity* pcConn, CMeshTriangles* pcTriangles, CMeshPositions* pcPositions, STriangleSphere* pasSpheres)
{
	CTriangleIndexed*		pcTriangle;
	size					j;
	float					fRelativeVolume;
	CMeshLeaf*				pcNewMeshLeaf;
	float					fLargerVolume;
	float					fSmallerVolume;
	float					fIntersectionVolume;
	CSphere					cNewSphere;
	CArrayBlock				asTempVerts;
	SFloat3*				psCenter;
	CTriangleIndexed*		pcTriangleOther;
	STriangleSphere*		psSphere;
	STriangleSphere*		psSphereOther;
	size					iNewIndex;
	SAdjFaces				sAdjFaces;

	iNewIndex = mcLeaves.NumElements();

	sAdjFaces = pcConn->GetAdjacentFaces(iFaceNum);
	pcTriangle = pcTriangles->mcTriangles.Get(iFaceNum);

	psSphere = &pasSpheres[iFaceNum];
	fLargerVolume = pasSpheres[iFaceNum].cRelativeVolume.fVolume;

	asTempVerts.Init(sizeof(SFloat3));
	for (j = 0; j < NUM_FACE_EDGES; j++)
	{
		if (sAdjFaces.aiFace[j] != -1)
		{
			if (pasSpheres[sAdjFaces.aiFace[j]].cRelativeVolume.iNewIndex == -1)
			{
				psSphereOther = &pasSpheres[sAdjFaces.aiFace[j]];
				fSmallerVolume = psSphereOther->cRelativeVolume.fVolume;
				pcTriangleOther = pcTriangles->mcTriangles.Get(sAdjFaces.aiFace[j]);

				fIntersectionVolume = psSphere->cSphere.IntersectSphereVolume(&psSphereOther->cSphere);
				fRelativeVolume = fIntersectionVolume / fSmallerVolume;

				//Can the smaller sphere be considred to be within the larger sphere.
				if (fRelativeVolume >= SPHERE_INTERSECTION_VOLUME_SAME)
				{
					//Yes, if 80% of it's volume is within it.
					asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangleOther->maiPositions[0]));
					asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangleOther->maiPositions[1]));
					asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangleOther->maiPositions[2]));

					psSphereOther->cRelativeVolume.iNewIndex = iNewIndex;
				}
			}
		}
	}

	//Add the centeral triangle.
	asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangle->maiPositions[0]));
	asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangle->maiPositions[1]));
	asTempVerts.Add(pcPositions->mcPositions.Get(pcTriangle->maiPositions[2]));

	pasSpheres[iFaceNum].cRelativeVolume.iNewIndex = iNewIndex;

	psCenter = pcPositions->mcPositions.Add();
	cNewSphere.Init(psCenter);
	cNewSphere.SetFromPointsUsingBestFit((SFloat3*)asTempVerts.GetData(), sizeof(SFloat3), asTempVerts.NumElements());
	pcNewMeshLeaf = mcLeaves.Add();
	pcNewMeshLeaf->Init();
	pcNewMeshLeaf->mcSphere.mfRadius = cNewSphere.mfRadius;
	pcNewMeshLeaf->mcSphere.miPosition = pcPositions->mcPositions.NumElements()-1;

	for (j = 0; j < NUM_FACE_EDGES; j++)
	{
		if (sAdjFaces.aiFace[j] != -1)
		{
			PrivateAddContainedLeafSpheres(iFaceNum, sAdjFaces.aiFace[j], iNewIndex, pcConn, pcTriangles, pcPositions, pasSpheres);
		}
	}

	asTempVerts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::PrivateAddContainedLeafSpheres(int iCentralFaceNum, int iFaceNum, int iSphereIndex, CMeshConnectivity* pcConn, CMeshTriangles* pcTriangles, CMeshPositions* pcPositions, STriangleSphere* pasSpheres)
{
	SAdjFaces			sAdjFaces;
	int					i;
	CTriangleIndexed*	pcTriangle;
	CTriangle			cTriangle;

	sAdjFaces = pcConn->GetAdjacentFaces(iFaceNum);
	for (i = 0; i < NUM_FACE_EDGES; i++)
	{
		if (sAdjFaces.aiFace[i] != -1)
		{
			if (sAdjFaces.aiFace[i] != iCentralFaceNum)
			{
				if (pasSpheres[sAdjFaces.aiFace[i]].cRelativeVolume.iNewIndex == -1)
				{
					pcTriangle = pcTriangles->mcTriangles.Get(sAdjFaces.aiFace[i]);
					pcTriangle->Shape(&cTriangle, pcPositions->mcPositions.GetData(), NULL);

					if (pasSpheres[iCentralFaceNum].cSphere.Contains(&cTriangle))
					{
						pasSpheres[sAdjFaces.aiFace[i]].cRelativeVolume.iNewIndex = iSphereIndex;
					}
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::GenerateLeafSpheres(CMeshConnectivity* pcConn, CMeshPositions* pcPositions, CMeshTriangles* pcTriangles)
{
	int							i;
	CTriangleIndexed*			pcTriangle;
	int							iNumFaces;
	SFloat3						cCenter;
	CRelativeSphereVolume*		psVolume;
	SAdjFaces					sAdjFaces;
	int							j;
	float						fRelativeVolume;
	STriangleSphere*			asSpheres;

	iNumFaces = pcTriangles->mcTriangles.NumElements();

	asSpheres = (STriangleSphere*)malloc(sizeof(STriangleSphere) * iNumFaces);

	//First generate a sphere for each triangle and calculate it's volume.
	for (i = 0; i < iNumFaces; i++)
	{
		pcTriangle = pcTriangles->mcTriangles.Get(i);
		SetSphereFromTriangle(&asSpheres[i], pcTriangle, pcPositions);
	}

	//Then work out the relative sizes.
	for (i = 0; i < iNumFaces; i++)
	{
		sAdjFaces = pcConn->GetAdjacentFaces(i);
		for (j = 0; j < NUM_FACE_EDGES; j++)
		{
			if (sAdjFaces.aiFace[j] != -1)
			{
				fRelativeVolume = asSpheres[sAdjFaces.aiFace[j]].cRelativeVolume.fVolume / asSpheres[i].cRelativeVolume.fVolume;
				if (fRelativeVolume < SPHERE_RADIUS_LESS_THAN)
				{
					asSpheres[i].cRelativeVolume.eRelative[j] = RSV_GreaterThan;
				}
				else if (fRelativeVolume > SPHERE_RADIUS_GREATER_THAN)
				{
					asSpheres[i].cRelativeVolume.eRelative[j] = RSV_LessThan;
				}
				else
				{
					asSpheres[i].cRelativeVolume.eRelative[j] = RSV_Equal;
				}
			}
		}
	}

	mcLeaves.Init();
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->IsAllGreaterThan())
		{
			PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
		}
	}

	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			if (psVolume->IsGreaterThanOrEqual(2))
			{
				PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
			}
		}
	}
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			if (psVolume->IsGreaterThanOrEqual(1))
			{
				PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
			}
		}
	}
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			if (psVolume->IsAllEqual())
			{
				PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
			}
		}
	}
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			if (psVolume->IsEqual(2))
			{
				PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
			}
		}
	}
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			if (psVolume->IsEqual(1))
			{
				PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
			}
		}
	}
	for (i = 0; i < iNumFaces; i++)
	{
		psVolume = &asSpheres[i].cRelativeVolume;
		if (psVolume->iNewIndex == -1)
		{
			PrivateAddLeafSphere(i, pcConn, pcTriangles, pcPositions, asSpheres);
		}
	}

	free(asSpheres);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::MakeLeavesIntoPolygons(void)
{
	//CMeshPolygons*	pcPolygons;
	//CMeshPolygon*	psPolygon;

	//pcPolygons = mpcMesh->AddPolygons(2);
	//pcPolygons->ReInit(iNumFaces);
	//pcPolygons->GrowToNumPolygons(pacSphere->NumElements());
	//for (i = 0; i < iNumFaces; i++)
	//{
	//	psVolume = &acVolumes[i];
	//	psPolygon = pcPolygons->Get(psVolume->iNewIndex);
	//	psPolygon->aiFaces.Add(i);
	//	pcPolygons->maiFacesToPolygons.SetValue(i, psVolume->iNewIndex);
	//}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshLeaves::SetSphereFromTriangle(STriangleSphere* psSphere, CTriangleIndexed* pcTriangleIndexed, CMeshPositions* pcPositions)
{
	SFloat3		asCorners[3];

	asCorners[0] = *pcPositions->mcPositions.Get(pcTriangleIndexed->maiPositions[0]);
	asCorners[1] = *pcPositions->mcPositions.Get(pcTriangleIndexed->maiPositions[1]);
	asCorners[2] = *pcPositions->mcPositions.Get(pcTriangleIndexed->maiPositions[2]);
	psSphere->cSphere.mpsPosition = &psSphere->sCenter;
	psSphere->cSphere.SetFromPointsUsingAveragePosition(asCorners, sizeof(SFloat3), 3);
	psSphere->cRelativeVolume.Init(psSphere->cSphere.Volume());
}


