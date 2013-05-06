#include "Shape.h"
#include "CoreLib\FastFunctions.h"
#include "StandardLib\StandardChunkNames.h"


//////////////////////////////////////////////////////////////////////////
//	Shape Base
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeBase::Init(void)
{
	CStandardListObject::Init();
	mcLLShapes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeBase::Kill(void)
{
	void*			pvNode;
	EShapeType		eType;

	pvNode = mcLLShapes.GetHead();
	while (pvNode)
	{
		eType = (EShapeType)mcLLShapes.GetNodeType(pvNode);
		KillShape(eType, pvNode);
		pvNode = mcLLShapes.GetNext(pvNode);
	}
	mcLLShapes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::Load(CChunkFile* pcFile)
{
	int iChunkNum;

	iChunkNum = pcFile->FindFirstChunkWithName(CFN_Shape);
	return Load(pcFile, iChunkNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::Load(CChunkFile* pcFile, int iChunkNum)
{
	int		iNumShapes;
	int		i;

	//Check that the chunk we are loading is really an shape chunk.
	if (!pcFile->VerifyChunkName(iChunkNum, CFN_Shape))
	{
		gcUserError.Set("Could not load shape from chunk, not a shape.");
		return FALSE;
	}

	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));
	if (!pcFile->ReadData(this, sizeof(CStandardTrackerObject)))
	{
		gcUserError.Set("Could not read shape header.");
		return FALSE;
	}

	//Open the index for this shape.
	if (!pcFile->ReadIndexBegin())
	{
		gcUserError.Set("Could not read shape index.");
		return FALSE;
	}

	//Read in the shape header.
	iChunkNum = pcFile->FindFirstChunkWithName(CFN_ShapeHeader);
	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));
	pcFile->ReadData(&iNumShapes, sizeof(int));

	//Find the data chunk for this shape.
	iChunkNum = pcFile->FindFirstChunkWithName(CFN_ShapeData);
	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));
	for (i = 0; i < iNumShapes; i++)
	{
		LoadShape(pcFile);
	}

	//Close this index.
	if (!pcFile->ReadIndexEnd())
	{
		gcUserError.Set("Could not close image index.");
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::Save(CChunkFile* pcFile)
{
	void*	pvNode;
	int		iNumShapes;

	ReturnOnFalse(pcFile->WriteChunkBegin());
	if (!pcFile->WriteData(this, sizeof(CStandardTrackerObject)))
	{
		gcUserError.Set("Could not shape header.");
		return FALSE;
	}

	//Write the image header to the file.
	ReturnOnFalse(pcFile->WriteChunkBegin());
	iNumShapes = mcLLShapes.GetNumElements();
	pcFile->WriteData(&iNumShapes, sizeof(int));
	ReturnOnFalse(pcFile->WriteChunkEnd(CFN_ShapeHeader));

	//Write out the shapes.
	ReturnOnFalse(pcFile->WriteChunkBegin())
	pvNode = mcLLShapes.GetHead();
	while (pvNode)
	{
		SaveShape(pvNode, pcFile);
		pvNode = mcLLShapes.GetNext(pvNode);
	}
	ReturnOnFalse(pcFile->WriteChunkEnd(CFN_ShapeData));

	ReturnOnFalse(pcFile->WriteChunkEnd(CFN_Shape));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::SaveShape(void* pvShape, CChunkFile* pcFile)
{
	EShapeType		eType;
	CLineSegmentImm*	pcLineSegment;

	eType = (EShapeType)mcLLShapes.GetNodeType(pvShape);
	pcFile->WriteData((void*)&eType, sizeof(EShapeType));
	switch(eType) 
	{
	case ST_LineSegment:
		pcLineSegment = (CLineSegmentImm*)pvShape;
		pcLineSegment->Save(pcFile);
		return TRUE;
	default:
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::LoadShape(CChunkFile* pcFile)
{
	EShapeType		eType;
	CLineSegmentImm*	pcLineSegment;

	pcFile->ReadData((void*)&eType, sizeof(EShapeType));
	switch(eType) 
	{
	case ST_LineSegment:
		pcLineSegment = (CLineSegmentImm*)AddShape(eType);
		pcLineSegment->Load(pcFile);
		return TRUE;
	default:
		return FALSE;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CShapeBase::Copy(CShapeBase* pcShape)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CShapeBase::ShapeSize(EShapeType eType)
{
	//Assumes any flags have already been stripped from eType.
	switch(eType) 
	{
	case ST_Sphere:
		return sizeof(CSphereImm);
	case ST_SphereTrans:
		return sizeof(CSphereTrans);
	case ST_Cone:
		return sizeof(CConeImm);
	case ST_ConeTrans:
		return sizeof(CConeTrans);
	case ST_LineSegment:
		return sizeof(CLineSegmentImm);
	case ST_LineSegmentTrans:
		return sizeof(CLineSegmentTrans);
	case ST_Plane:
		return sizeof(CPlaneImm);
//	case ST_PlaneTrans:
//		return sizeof(CPlaneTrans);
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CShapeBase::AddShape(EShapeType eType)
{
	int		iByteSize;
	void*	pvShape;

	iByteSize = ShapeSize(eType);

	pvShape = mcLLShapes.InsertAfterTail(iByteSize, eType);
	memset_fast(pvShape, 0, iByteSize);
	return pvShape;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeBase::KillShape(EShapeType eType, void* pvShape)
{
//	switch(eType)
//	{
//	case ST_ConvexHull:
//		((CConvexHull*)pvShape)->Kill();
//		break;
//	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CShapeBase::GetFirst(int* piType)
{
	return mcLLShapes.GetHeadAndType(piType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CShapeBase::GetNext(void* pvShape, int* piType)
{
	return mcLLShapes.GetNextAndType(pvShape, piType);
}


//////////////////////////////////////////////////////////////////////////
//	Shape Trans
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeTrans::Init(void)
{
	mcVerts = NULL;
	mcNorms = NULL;
	mcSkinnedVerts = NULL;
	mcSkinnedNorms = NULL;
	CShapeBase::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShapeTrans::Kill(void)
{
	if (mcVerts != NULL)
	{
		mcVerts->Kill();
		mcVerts = NULL;
	}
	if (mcNorms != NULL)
	{
		mcNorms->Kill();
		mcNorms = NULL;
	}
	if (mcSkinnedVerts != NULL)	
	{
		mcSkinnedVerts->Kill();
		mcSkinnedVerts = NULL;
	}
	if (mcSkinnedNorms != NULL)
	{
		mcSkinnedNorms->Kill();
		mcSkinnedNorms = NULL;
	}

	CShapeBase::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSphereTrans* CShapeTrans::AddSphereTrans(void)
{
	return (CSphereTrans*)AddShape(ST_SphereTrans);
}


//////////////////////////////////////////////////////////////////////////
//	Shape
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLineSegmentImm* CShapeImm::AddLineSegment(void)
{
	return (CLineSegmentImm*)AddShape(ST_LineSegment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSphereImm* CShapeImm::AddSphere(void)
{
	return (CSphereImm*)AddShape(ST_Sphere);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPlaneImm* CShapeImm::AddPlane(void)
{
	return (CPlaneImm*)AddShape(ST_Plane);
}

