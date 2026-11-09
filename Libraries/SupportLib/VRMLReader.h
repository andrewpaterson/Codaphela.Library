/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __VRML_READER_H__
#define __VRML_READER_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayUint32.h"
#include "BaseLib/Chars.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/MapStringInt.h"
#include "BaseLib/TextParser.h"
#include "Mesh.h"


#define VRML_MAX_NAME_LENGTH	256


//A Shape's geometry as it is in the file.  Positions and normals are shared by the Shape's instances.
struct SVRMLShape
{
	int		iMaterial;  //Index of a colour in mauiMaterials or -1.
	int		iFirstPosition;
	int		iNumPositions;
	int		iFirstNormal;
	int		iNumNormals;  //iNumPositions or 0.
	int		iFirstIndex;
	int		iNumIndices;  //Three for each triangle.
};


typedef CArrayTemplate<SVRMLShape>	CArrayVRMLShape;


//A Transform's children are completely read before it is finished so USE cannot make a cycle.
struct SVRMLTransform
{
	SFloat4x4	sMatrix;
	CArrayInt	aiChildren;  //Transforms are >= 0 and Shapes are ~iShape.
	int64		iNumCorners;  //Including the instances of all children.
	int64		iNumFaces;
};


typedef CArrayTemplate<SVRMLTransform>	CArrayVRMLTransform;


class CMesh;


bool LoadVRML(Ptr<CMesh> pMesh, const char* szFilename);


//Reads the VRML 2.0 that KiCad exports: Transform, Shape, Appearance, Material, IndexedFaceSet, Coordinate and Normal.
//The whole file is read before the mesh is built because CMeshNormals::AddFace inserts each face's normal in front of
//the corner normals, and adding every corner normal after the last face means that nothing has to move.
class CVRMLReader
{
protected:
	CMesh*					mpcMesh;
	CChars					mszFileName;
	CTextParser				mcParser;
	CArrayFloat32Vec3		masPositions;
	CArrayFloat32Vec3		masNormals;
	CArrayInt				maiIndices;
	CArrayUint32			mauiMaterials;
	CArrayVRMLShape			masShapes;
	CArrayVRMLTransform		masTransforms;
	CMapStringInt			mcTransformNames;
	CMapStringInt			mcShapeNames;
	CMapStringInt			mcMaterialNames;
	int						miDepth;
	int						miNumFaces;
	int						miNextNormal;

public:
	void 	Init(Ptr<CMesh> pMesh, const char* szFilename);
	void 	Kill(void);

	bool 	Read(void);
	bool 	Read(const char* szText, lsize uiLength);

protected:
	bool	Error(const char* szMessage);
	bool	Error(const char* szMessage, const char* szName);
	bool	ReadName(char* szName);
	bool	ReadExact(char c);
	bool	ReadFloat(float32* pf);
	bool	ReadFloats(float32* pf, int iNumFloats);
	bool	ReadInt(int32* pi);
	bool	ReadBool(bool* pb);
	bool	ReadVectors(CArrayFloat32Vec3* pasVectors);
	bool	ReadChildren(int iParent);
	bool	ReadNode(int iParent, const char* szFirstName);
	bool	ReadTransform(int iParent, const char* szDefName);
	bool	ReadShape(int iParent, const char* szDefName);
	bool	ReadAppearance(int* piMaterial, const char* szDefName);
	bool	ReadMaterial(int* piMaterial);
	bool	ReadIndexedFaceSet(SVRMLShape* psShape);
	bool	ReadCoordinate(SVRMLShape* psShape);
	bool	ReadNormal(SVRMLShape* psShape);
	bool	ReadCoordIndices(SVRMLShape* psShape);
	bool	ReadDefinition(char* szType);
	bool	AddChild(int iParent, int iChild, int64 iNumCorners, int64 iNumFaces);
	void	BuildMesh(void);
	void	AddFaces(int iTransform, SFloat4x4* psParent);
	void	AddNormals(int iTransform, SFloat4x4* psParent);
};


#endif // __VRML_READER_H__

