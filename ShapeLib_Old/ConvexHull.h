#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__
#include "Plane.h"
#include "CoreLib\ChunkFile.h"


class CConvexHullImm
{
public:
	CPlaneArray	mcPlanes;

	void 		Init(void);
	void 		Kill(void);
	CPlaneImm*	AddPlane(void);
	void		AddPlane(CPlaneImm* pcPlane);

	void 		Save(CChunkFile* pcFile);
	void 		Load(CChunkFile* pcFile);
};


#endif __CONVEX_HULL_H__

