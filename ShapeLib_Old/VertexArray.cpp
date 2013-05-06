#include "CoreLib\ErrorHandler.h"
#include "VertexArray.h"


void CVertexArray::Init(EVertexArrayType iType)
{
	miType = iType;
	maMatrixAffects.Init(4);
	maVectorSource.Init(100);
	maVectorTransformed.Init();
}


void CVertexArray::Kill(void)
{
	maVectorSource.Kill();
	maVectorTransformed.Kill();
	maMatrixAffects.Kill();
	miType = VAT_None;
}


void CVertexArray::SetVectorSize(int iNumVectors)
{
	maVectorSource.Kill();
	maVectorSource.Allocate(iNumVectors);
	if (maVectorTransformed.pvArray == NULL)
	{
		maVectorTransformed.Allocate(iNumVectors+1);
	}
}


void CVertexArray::SetMatrixSize(int iNumMatricies)
{
	maMatrixAffects.Kill();
	maMatrixAffects.Allocate(iNumMatricies);
}


void CVertexArray::Set(void)
{
	//The size + 1 is to stop directX overwriting memory when it transforms the last vertex in the array.
	if (maVectorTransformed.pvArray == NULL)
	{
		maVectorTransformed.Allocate(maVectorSource.miUsedElements+1);
	}
	if (maVectorTransformed.miUsedElements != maVectorSource.miUsedElements+1)
	{
		gcUserError.Set("Vertex array size and destination mismatch");
	}
}


void CVertexArray::Transform(D3DXMATRIX** ppMatricies)
{	
	int				i;
	SMatrixAffect*	psMatrixAffectStart;
	SMatrixAffect*	psMatrixAffectEnd;
	D3DXMATRIX*		psMatrix;
	int				iNumVectors;

	//Could do with some optimisation.
	if (miType == VAT_Vertex)
	{
		for (i = 0; i < maMatrixAffects.miUsedElements - 1; i++)
		{
			psMatrixAffectStart = maMatrixAffects.Get(i);
			psMatrixAffectEnd = maMatrixAffects.Get(i + 1);
			psMatrix = ppMatricies[psMatrixAffectStart->iMatrix];
			iNumVectors = psMatrixAffectEnd->iStartVertex - psMatrixAffectStart->iStartVertex;

			D3DXVec3TransformArray((D3DXVECTOR4*)maVectorTransformed.Get(psMatrixAffectStart->iStartVertex), sizeof(D3DXVECTOR3), (D3DXVECTOR3*)maVectorSource.Get(psMatrixAffectStart->iStartVertex), sizeof(D3DXVECTOR3), psMatrix, iNumVectors);
		}
	}
	else if (miType == VAT_Normal)
	{
		for (i = 0; i < maMatrixAffects.miUsedElements - 1; i++)
		{
			psMatrixAffectStart = maMatrixAffects.Get(i);
			psMatrixAffectEnd = maMatrixAffects.Get(i + 1);
			psMatrix = ppMatricies[psMatrixAffectStart->iMatrix];
			iNumVectors = psMatrixAffectEnd->iStartVertex - psMatrixAffectStart->iStartVertex;

			D3DXVec3TransformNormalArray((D3DXVECTOR3*)maVectorTransformed.Get(psMatrixAffectStart->iStartVertex), sizeof(D3DXVECTOR3), (D3DXVECTOR3*)maVectorSource.Get(psMatrixAffectStart->iStartVertex), sizeof(D3DXVECTOR3), psMatrix, iNumVectors);
		}
	}
}


void CVertexArray::Transform(D3DXMATRIX* pMatrix)
{
	if (miType == VAT_Vertex)
	{
		D3DXVec3TransformArray((D3DXVECTOR4*)maVectorTransformed.Get(0), sizeof(D3DXVECTOR3), (D3DXVECTOR3*)maVectorSource.Get(0), sizeof(D3DXVECTOR3), pMatrix, maVectorSource.miUsedElements);
	}
	else if (miType == VAT_Normal)
	{
		D3DXVec3TransformNormalArray((D3DXVECTOR3*)maVectorTransformed.Get(0), sizeof(D3DXVECTOR3), (D3DXVECTOR3*)maVectorSource.Get(0), sizeof(D3DXVECTOR3), pMatrix, maVectorSource.miUsedElements);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSkinnedVertexArray::Init(EVertexArrayType iType)
{
	miType = iType;
	maVectorSource.Init(100);
	maVectorTransformed.Init();
}


void CSkinnedVertexArray::Kill(void)
{
	miType = VAT_None;
	maVectorSource.Kill();
	maVectorTransformed.Kill();
}


void CSkinnedVertexArray::SetVectorSize(int iNumVectors)
{
}


void CSkinnedVertexArray::Transform(D3DXMATRIX** ppMatricies)
{
}

