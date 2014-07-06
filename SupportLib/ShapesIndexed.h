/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SHAPES_INDEXED_H__
#define __SHAPES_INDEXED_H__
#include "BaseLib/ArrayIntAndPointer.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/ArraySimpleInt.h"
#include "StandardLib/NamedObject.h"
#include "Sphere.h"
#include "Triangle.h"


class CShapeIndexed
{
public:
};


class CSphereIndexed : public CShapeIndexed
{
public:
	int		miPosition;
	float	mfRadius;

	void 	Init(void);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
	BOOL	Shape(CSphere* pcShape, SFloat3* psVertexArray);
};


class CTriangleIndexed : public CShapeIndexed
{
public:
	int		maiPositions[3];
	int		miNormalIndex;

	void	Init(void);
	void 	Init(int p1, int p2, int p3, int n);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
	BOOL	Shape(CTriangle* pcShape, SFloat3* psVertexArray, SFloat3* psNormalArray);
};


class CBoxIndexed : public CShapeIndexed
{
public:
	int			miCenterIndex;

	int			miLongAxisIndex;
	int			miMiddleAxisIndex;
	int			miShortAxisIndex;

	float		mfLongAxisLength;
	float		mfMiddleAxisLength;
	float		mfShortAxisLength;

	void 	Init(void);
	void 	Init(int iCenterIndex, int iLongAxisIndex, float fLongAxisLength, int iMiddleAxisIndex, float fMiddleAxisLength, int iShortAxisIndex, float fShortAxisLength);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


class CPolygonIndexed : public CShapeIndexed
{
public:
	CArraySimpleInt		maiPositions;  //It's assumed all these points are in a plane.
	int					miNormalIndex;
	BOOL				mbConvex;

	void 	Init(void);
	void 	Kill(void);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


typedef CArrayTemplate<CPolygonIndexed>		CArrayPolygonIndexed;


class CConvexHullIndexed : public CShapeIndexed
{
public:
	CArrayPolygonIndexed	mcPolygons;

	void 	Init(void);
	void	Init(int iNumPolygons);
	void 	Kill(void);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


#endif // __SHAPES_INDEXED_H__

