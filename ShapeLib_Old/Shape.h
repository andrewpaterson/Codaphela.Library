#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "VertexArray.h"
#include "CoreLib\LinkListUnknown.h"
#include "ShapeTransformer.h"
#include "CoreLib\ChunkFile.h"
#include "StandardLib\StandardHeader.h"


enum EShapeType
{
	ST_Shape = 0,	//ST_Shape is defined so compound shapes can be added into a hierachy.
					//An error will occur if an ST_Shape is added to an ST_Shape.  If a tree
					//type structure is needed use ST_Hierachy instead.

	ST_Hierachy,	//ST_Hierachy is defined so that hierachies can be added to compound shapes.
					//generally a complex shape will begin with a hierachy.  It's largely
					//pointless to have a hierachy shape as hierachy node.

	ST_Sphere,	//Both the untransformable (also called post-transformed) shapes and the
				//transformable shapes can be added to a shape.  Untransformable shapes do not
				//have the 'Trans' suffix.  Generally only the corresponding types will be
				//added to the ST_Shape and ST_ShapeTrans types.  It is allowable to add the
				//'wrong' type but automatic transformation cannot be done then.
	ST_Cone,

	//Unfinished...
	ST_Plane,
	ST_ArbitraryMesh,
	ST_Capsule,
	ST_ConvexHull,

	ST_LineSegment,


	ST_ShapeTrans,  //The transform shapes.  There is no transform hierachy.
	ST_SphereTrans,
	ST_ConeTrans,
	ST_LineSegmentTrans,

	ST_Collider  = 0x80000000,	//This shape can be used for finding collisions for physics.
	ST_Occluder = 0x40000000,	//Can be rendered to occlusion buffer for occlusion writing.
	ST_Bounder = 0x20000000,	//General bounding shape.  Can be tested against occlusion buffer.
	ST_Unused = 0x10000000		//Reserved for future expansion.
};


#define SHAPE_FLAGS_MASK 0xF0000000


class CShapeBase : public CStandardListObject
{
	CLinkListUnknown		mcLLShapes;  //The unknown 'inherited' type defines what type of shape it is from EShapeTypes.

public:
	void		Init(void);
	void		Kill(void);
	void*		AddShape(EShapeType eType);
	void		KillShape(EShapeType eType, void* pvShape);
	BOOL		SaveShape(void* pvShape, CChunkFile* pcFile);
	BOOL		LoadShape(CChunkFile* pcFile);
	static int	ShapeSize(EShapeType eType);

	BOOL		Load(CChunkFile* pcFile);
	BOOL		Load(CChunkFile* pcFile, int iChunkNum);
	BOOL		Save(CChunkFile* pcFile);
	BOOL		Copy(CShapeBase* pcShape);
	void*		GetFirst(int* piType);
	void*		GetNext(void* pvShape, int* piType);
};


class CShapeImm : public CShapeBase
{
public:
	CLineSegmentImm*	AddLineSegment(void);
	CSphereImm*			AddSphere(void);
	CPlaneImm*			AddPlane(void);
};


//I Suspect this one is a load of crap.
class CShapeTrans : public CShapeBase
{
public:
	CVertexArray*			mcVerts;
	CVertexArray*			mcNorms;
	CSkinnedVertexArray*	mcSkinnedVerts;
	CSkinnedVertexArray*	mcSkinnedNorms;

	void Init(void);
	void Kill(void);
	void Transform(void);

	CSphereTrans*	AddSphereTrans(void);
};


#endif __SHAPE_H__

