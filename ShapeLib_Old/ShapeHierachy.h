#ifndef __SHAPE_HIERACHY_H__
#define __SHAPE_HIERACHY_H__
#include "Shapes.h"
#include "CoreLib\TreeTemplate.h"


enum EExactness
{
	E_SuperObject,
	E_Object,
	E_SubObject
};


enum ECheckStatus
{
	CS_NotChecked,
	CS_CheckedAndPassed,
	CS_CheckedAndFailed,
};


struct SHierachyNode
{
	EShapeType		iType;
	EExactness		iExactness;
	ECheckStatus	iCheckStatus;
	void*			pvShape;
};


typedef CTreeTemplate<SHierachyNode>	CShapeTree;


class CShapeHierachy
{
public:
	CShapeTree	mcTree;
};



#endif __SHAPE_HIERACHY_H__

