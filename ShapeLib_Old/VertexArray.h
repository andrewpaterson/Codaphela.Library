#ifndef __VERTEX_ARRAY_H__
#define __VERTEX_ARRAY_H__
#include <d3dx9.h>
#include "CoreLib\PackedFloat.h"
#include "CoreLib\ArrayTemplate.h"


struct SSkinnedVector
{
	D3DVECTOR		sVector;
	unsigned int	iMatricies;
	CPackedFloat	cWeights;
};


struct SMatrixAffect
{
	int	iStartVertex;   //The first vertex this matrix affects.
	int iMatrix;		//Which matrix from the Matrix* array is this.
};


typedef CArrayTemplate<D3DVECTOR>		CArrayVector;
typedef CArrayTemplate<SSkinnedVector>	CArraySkinnedVector;
typedef CArrayTemplate<SMatrixAffect>	CArrayMatrixAffects;


enum EVertexArrayType
{
	VAT_Vertex,
	VAT_Normal,
	VAT_None = -1,
};


class CVertexArray
{
public:
	CArrayVector			maVectorSource;
	CArrayVector			maVectorTransformed;
	CArrayMatrixAffects		maMatrixAffects;
	EVertexArrayType		miType;
	
	void Init(EVertexArrayType iType);
	void Kill(void);
	void SetVectorSize(int iNumVectors);
	void SetMatrixSize(int iNumMatricies);
	void Set(void);
	void Transform(D3DXMATRIX** ppMatricies);  //When many matricies affect the vertex arrary.
	void Transform(D3DXMATRIX* pMatrix);  //When only one matrix affects the vertex array.
};


class CSkinnedVertexArray
{
public:
	CArraySkinnedVector		maVectorSource;
	CArraySkinnedVector		maVectorTransformed;
	EVertexArrayType		miType;
	
	void Init(EVertexArrayType iType);
	void Kill(void);
	void SetVectorSize(int iNumVectors);
	void Transform(D3DXMATRIX** ppMatricies);  //When many matricies affect the vertex arrary (which is always the case with a skinned mesh).
};



#endif __VERTEX_ARRAY_H__

