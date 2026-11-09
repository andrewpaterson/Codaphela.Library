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
#include <string.h>
#include "BaseLib/CPPIdentifier.h"
#include "BaseLib/CPPString.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/TextFile.h"
#include "ColourARGB32.h"
#include "Mesh.h"
#include "VRMLReader.h"


#define VRML_HEADER				"#VRML V2.0"
#define VRML_HEADER_LENGTH		10
#define VRML_MAX_DEPTH			256


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadVRML(Ptr<CMesh> pMesh, const char* szFilename)
{
	CVRMLReader	cReader;
	bool		bResult;

	cReader.Init(pMesh, szFilename);
	bResult = cReader.Read();
	cReader.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// VRML treats commas as whitespace and # starts a comment.
//
//////////////////////////////////////////////////////////////////////////
static void SkipVRMLWhitespace(CTextParser* pcParser)
{
	unsigned char	c;

	while (!pcParser->mbOutsideText)
	{
		c = (unsigned char)*pcParser->mszParserPos;
		if ((c <= ' ') || (c == ','))
		{
			pcParser->StepRight();
		}
		else if (c == '#')
		{
			while ((!pcParser->mbOutsideText) && (*pcParser->mszParserPos != '\n'))
			{
				pcParser->StepRight();
			}
		}
		else
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsVRMLNameEnd(char c)
{
	return ((unsigned char)c <= ' ') || (c == ',') || (c == '#') || (c == '{') || (c == '}') || (c == '[') || (c == ']');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVRMLReader::Init(Ptr<CMesh> pMesh, const char* szFilename)
{
	mpcMesh = &pMesh;
	mszFileName.Init(szFilename);
	masPositions.Init();
	masNormals.Init();
	maiIndices.Init();
	mauiMaterials.Init();
	masShapes.Init();
	masTransforms.Init();
	mcTransformNames.Init();
	mcShapeNames.Init();
	mcMaterialNames.Init();
	miDepth = 0;
	miNumFaces = 0;
	miNextNormal = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVRMLReader::Kill(void)
{
	size	i;
	size	uiNumElements;

	mcMaterialNames.Kill();
	mcShapeNames.Kill();
	mcTransformNames.Kill();

	uiNumElements = masTransforms.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		masTransforms.Get(i)->aiChildren.Kill();
	}
	masTransforms.Kill();

	masShapes.Kill();
	mauiMaterials.Kill();
	maiIndices.Kill();
	masNormals.Kill();
	masPositions.Kill();
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::Read(void)
{
	CTextFile	cTextFile;
	bool		bResult;

	cTextFile.Init();
	if (!cTextFile.Read(mszFileName.Text()))
	{
		cTextFile.Kill();
		return gcLogger.Error2(__METHOD__, " Could not read [", mszFileName.Text(), "].", NULL);
	}

	cTextFile.PassifyNewlines();
	bResult = Read(cTextFile.Text(), cTextFile.Length());
	cTextFile.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The text must not contain carriage returns.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::Read(const char* szText, lsize uiLength)
{
	STextParserConfig	sConfig;
	SVRMLTransform*		psRoot;
	bool				bResult;

	if ((uiLength < VRML_HEADER_LENGTH) || (memcmp(szText, VRML_HEADER, VRML_HEADER_LENGTH) != 0))
	{
		return gcLogger.Error2(__METHOD__, " [", mszFileName.Text(), "] is not VRML 2.0.", NULL);
	}

	sConfig.Init(SkipVRMLWhitespace, ParseCPPString, ParseCPPExactIdentifier, ParseCPPIdentifier);
	if (!mcParser.Init(szText, uiLength, &sConfig))
	{
		return false;
	}

	psRoot = masTransforms.Add();
	psRoot->sMatrix.Identity();
	psRoot->aiChildren.Init();
	psRoot->iNumCorners = 0;
	psRoot->iNumFaces = 0;

	bResult = ReadChildren(0);
	mcParser.Kill();

	if (bResult)
	{
		BuildMesh();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::Error(const char* szMessage)
{
	return Error(szMessage, "");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::Error(const char* szMessage, const char* szName)
{
	CChars	szError;

	szError.Init(" [");
	szError.Append(mszFileName.Text());
	szError.Append("] line ");
	szError.Append((int32)mcParser.miLine + 1);
	szError.Append(": ");
	szError.Append(szMessage);
	szError.Append(szName);
	gcLogger.Error2(__METHOD__, szError.Text(), NULL);
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// szName must be at least VRML_MAX_NAME_LENGTH long.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadName(char* szName)
{
	int		i;

	mcParser.SkipWhitespace();

	i = 0;
	while ((!mcParser.mbOutsideText) && (!IsVRMLNameEnd(*mcParser.mszParserPos)))
	{
		if (i == VRML_MAX_NAME_LENGTH - 1)
		{
			return Error("Name is too long.");
		}
		szName[i] = *mcParser.mszParserPos;
		i++;
		mcParser.StepRight();
	}
	szName[i] = '\0';

	if (i == 0)
	{
		return Error("Expected a name.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadExact(char c)
{
	char	sz[2];

	if (mcParser.GetExactCharacter(c) != TRITRUE)
	{
		sz[0] = c;
		sz[1] = '\0';
		return Error("Expected ", sz);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// KiCad writes exponents (-9.4382988e-05).
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadFloat(float32* pf)
{
	if (mcParser.GetFloatWithExponent(pf) != TRITRUE)
	{
		return Error("Expected a number.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadFloats(float32* pf, int iNumFloats)
{
	int		i;

	for (i = 0; i < iNumFloats; i++)
	{
		ReturnOnFalse(ReadFloat(&pf[i]));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadInt(int32* pi)
{
	if (mcParser.GetInteger(pi) != TRITRUE)
	{
		return Error("Expected an integer.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadBool(bool* pb)
{
	char	szValue[VRML_MAX_NAME_LENGTH];

	ReturnOnFalse(ReadName(szValue));
	if (StringCompare(szValue, "TRUE") == 0)
	{
		*pb = true;
		return true;
	}
	else if (StringCompare(szValue, "FALSE") == 0)
	{
		*pb = false;
		return true;
	}
	return Error("Expected TRUE or FALSE but got ", szValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadVectors(CArrayFloat32Vec3* pasVectors)
{
	float32			af[3];
	SFloat32Vec3*	psVector;

	ReturnOnFalse(ReadExact('['));
	while (mcParser.GetExactCharacter(']') != TRITRUE)
	{
		ReturnOnFalse(ReadFloats(af, 3));
		psVector = pasVectors->Add();
		psVector->Init(af[0], af[1], af[2]);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads nodes until the end of the text or the ] that ends a children field.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadChildren(int iParent)
{
	char	szName[VRML_MAX_NAME_LENGTH];

	miDepth++;
	if (miDepth > VRML_MAX_DEPTH)
	{
		return Error("Transforms are nested too deeply.");
	}

	for (;;)
	{
		mcParser.SkipWhitespace();
		if (mcParser.mbOutsideText)
		{
			if (iParent != 0)
			{
				return Error("Unexpected end of file.");
			}
			break;
		}
		if ((iParent != 0) && (*mcParser.mszParserPos == ']'))
		{
			mcParser.StepRight();
			break;
		}

		ReturnOnFalse(ReadName(szName));
		ReturnOnFalse(ReadNode(iParent, szName));
	}

	miDepth--;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadNode(int iParent, const char* szFirstName)
{
	char			szType[VRML_MAX_NAME_LENGTH];
	char			szDefName[VRML_MAX_NAME_LENGTH];
	int				iNode;
	SVRMLTransform*	psTransform;
	SVRMLShape*		psShape;

	if (StringCompare(szFirstName, "USE") == 0)
	{
		ReturnOnFalse(ReadName(szDefName));

		iNode = mcTransformNames.Get(szDefName, -1);
		if (iNode != -1)
		{
			psTransform = masTransforms.Get(iNode);
			return AddChild(iParent, iNode, psTransform->iNumCorners, psTransform->iNumFaces);
		}

		iNode = mcShapeNames.Get(szDefName, -1);
		if (iNode != -1)
		{
			psShape = masShapes.Get(iNode);
			return AddChild(iParent, ~iNode, psShape->iNumPositions, psShape->iNumIndices / 3);
		}
		return Error("USE of an unknown Transform or Shape: ", szDefName);
	}

	szDefName[0] = '\0';
	if (StringCompare(szFirstName, "DEF") == 0)
	{
		ReturnOnFalse(ReadName(szDefName));
		ReturnOnFalse(ReadName(szType));
	}
	else
	{
		strcpy(szType, szFirstName);
	}

	if (StringCompare(szType, "Transform") == 0)
	{
		return ReadTransform(iParent, szDefName);
	}
	else if (StringCompare(szType, "Shape") == 0)
	{
		return ReadShape(iParent, szDefName);
	}
	return Error("Unsupported node: ", szType);
}


//////////////////////////////////////////////////////////////////////////
//
// The matrix is the VRML one, -C -SR S SR R C T, in BaseLib's row vector order.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadTransform(int iParent, const char* szDefName)
{
	char			szField[VRML_MAX_NAME_LENGTH];
	float32			afCenter[3] = { 0.0f, 0.0f, 0.0f };
	float32			afRotation[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float32			afScale[3] = { 1.0f, 1.0f, 1.0f };
	float32			afScaleOrientation[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float32			afTranslation[3] = { 0.0f, 0.0f, 0.0f };
	SFloat4x4		sStep;
	SFloat4x4		sMatrix;
	SFloat32Vec3	sAxis;
	SVRMLTransform*	psTransform;
	int				iTransform;

	iTransform = (int)masTransforms.NumElements();
	psTransform = masTransforms.Add();
	psTransform->sMatrix.Identity();
	psTransform->aiChildren.Init();
	psTransform->iNumCorners = 0;
	psTransform->iNumFaces = 0;

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "center") == 0)
		{
			ReturnOnFalse(ReadFloats(afCenter, 3));
		}
		else if (StringCompare(szField, "rotation") == 0)
		{
			ReturnOnFalse(ReadFloats(afRotation, 4));
		}
		else if (StringCompare(szField, "scale") == 0)
		{
			ReturnOnFalse(ReadFloats(afScale, 3));
		}
		else if (StringCompare(szField, "scaleOrientation") == 0)
		{
			ReturnOnFalse(ReadFloats(afScaleOrientation, 4));
		}
		else if (StringCompare(szField, "translation") == 0)
		{
			ReturnOnFalse(ReadFloats(afTranslation, 3));
		}
		else if (StringCompare(szField, "children") == 0)
		{
			ReturnOnFalse(ReadExact('['));
			ReturnOnFalse(ReadChildren(iTransform));
		}
		else
		{
			return Error("Unsupported Transform field: ", szField);
		}
	}

	Float4x4Translation(&sMatrix, -afCenter[0], -afCenter[1], -afCenter[2]);

	sAxis.Init(afScaleOrientation[0], afScaleOrientation[1], afScaleOrientation[2]);
	sAxis.Normalize();
	Float4x4RotationVector(&sStep, &sAxis, -afScaleOrientation[3]);
	Float4x4Multiply(&sMatrix, &sMatrix, &sStep);

	sStep.Identity();
	sStep.x.x = afScale[0];
	sStep.y.y = afScale[1];
	sStep.z.z = afScale[2];
	Float4x4Multiply(&sMatrix, &sMatrix, &sStep);

	Float4x4RotationVector(&sStep, &sAxis, afScaleOrientation[3]);
	Float4x4Multiply(&sMatrix, &sMatrix, &sStep);

	sAxis.Init(afRotation[0], afRotation[1], afRotation[2]);
	sAxis.Normalize();
	Float4x4RotationVector(&sStep, &sAxis, afRotation[3]);
	Float4x4Multiply(&sMatrix, &sMatrix, &sStep);

	Float4x4Translation(&sStep, afCenter[0] + afTranslation[0], afCenter[1] + afTranslation[1], afCenter[2] + afTranslation[2]);
	Float4x4Multiply(&sMatrix, &sMatrix, &sStep);

	//Reading the children can move the transforms.
	psTransform = masTransforms.Get(iTransform);
	psTransform->sMatrix = sMatrix;

	if (szDefName[0] != '\0')
	{
		mcTransformNames.Put(szDefName, iTransform);
	}
	return AddChild(iParent, iTransform, psTransform->iNumCorners, psTransform->iNumFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadShape(int iParent, const char* szDefName)
{
	char		szField[VRML_MAX_NAME_LENGTH];
	char		szType[VRML_MAX_NAME_LENGTH];
	char		szAppearanceName[VRML_MAX_NAME_LENGTH];
	SVRMLShape	sShape;
	SVRMLShape*	psShape;
	int			iShape;
	int			i;

	sShape.iMaterial = -1;
	sShape.iFirstPosition = (int)masPositions.NumElements();
	sShape.iNumPositions = 0;
	sShape.iFirstNormal = (int)masNormals.NumElements();
	sShape.iNumNormals = 0;
	sShape.iFirstIndex = (int)maiIndices.NumElements();
	sShape.iNumIndices = 0;

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "appearance") == 0)
		{
			ReturnOnFalse(ReadName(szType));
			if (StringCompare(szType, "USE") == 0)
			{
				ReturnOnFalse(ReadName(szAppearanceName));
				sShape.iMaterial = mcMaterialNames.Get(szAppearanceName, -1);
				if (sShape.iMaterial == -1)
				{
					return Error("USE of an unknown Appearance: ", szAppearanceName);
				}
			}
			else
			{
				szAppearanceName[0] = '\0';
				if (StringCompare(szType, "DEF") == 0)
				{
					ReturnOnFalse(ReadName(szAppearanceName));
					ReturnOnFalse(ReadName(szType));
				}
				if (StringCompare(szType, "Appearance") != 0)
				{
					return Error("Unsupported appearance: ", szType);
				}
				ReturnOnFalse(ReadAppearance(&sShape.iMaterial, szAppearanceName));
			}
		}
		else if (StringCompare(szField, "geometry") == 0)
		{
			ReturnOnFalse(ReadDefinition(szType));
			if (StringCompare(szType, "IndexedFaceSet") != 0)
			{
				return Error("Unsupported geometry: ", szType);
			}
			ReturnOnFalse(ReadIndexedFaceSet(&sShape));
		}
		else
		{
			return Error("Unsupported Shape field: ", szField);
		}
	}

	if ((sShape.iNumNormals != 0) && (sShape.iNumNormals != sShape.iNumPositions))
	{
		return Error("A Normal must have a vector for each point.  The file has no normalIndex.");
	}

	for (i = 0; i < sShape.iNumIndices; i++)
	{
		if (maiIndices.GetValue(sShape.iFirstIndex + i) >= sShape.iNumPositions)
		{
			return Error("A coordIndex is past the last point.");
		}
	}

	iShape = (int)masShapes.NumElements();
	psShape = masShapes.Add();
	*psShape = sShape;

	if (szDefName[0] != '\0')
	{
		mcShapeNames.Put(szDefName, iShape);
	}
	return AddChild(iParent, ~iShape, sShape.iNumPositions, sShape.iNumIndices / 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadAppearance(int* piMaterial, const char* szDefName)
{
	char	szField[VRML_MAX_NAME_LENGTH];
	char	szType[VRML_MAX_NAME_LENGTH];

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "material") != 0)
		{
			return Error("Unsupported Appearance field: ", szField);
		}

		ReturnOnFalse(ReadDefinition(szType));
		if (StringCompare(szType, "Material") != 0)
		{
			return Error("Unsupported material: ", szType);
		}
		ReturnOnFalse(ReadMaterial(piMaterial));
	}

	if ((szDefName[0] != '\0') && (*piMaterial != -1))
	{
		mcMaterialNames.Put(szDefName, *piMaterial);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh's colour has only room for the diffuse colour and the opacity.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadMaterial(int* piMaterial)
{
	char	szField[VRML_MAX_NAME_LENGTH];
	float32	afDiffuse[3] = { 0.8f, 0.8f, 0.8f };
	float32	afIgnored[3];
	float32	fTransparency;

	fTransparency = 0.0f;

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "diffuseColor") == 0)
		{
			ReturnOnFalse(ReadFloats(afDiffuse, 3));
		}
		else if (StringCompare(szField, "transparency") == 0)
		{
			ReturnOnFalse(ReadFloat(&fTransparency));
		}
		else if ((StringCompare(szField, "emissiveColor") == 0) || (StringCompare(szField, "specularColor") == 0))
		{
			ReturnOnFalse(ReadFloats(afIgnored, 3));
		}
		else if ((StringCompare(szField, "ambientIntensity") == 0) || (StringCompare(szField, "shininess") == 0))
		{
			ReturnOnFalse(ReadFloat(afIgnored));
		}
		else
		{
			return Error("Unsupported Material field: ", szField);
		}
	}

	*piMaterial = (int)mauiMaterials.NumElements();
	mauiMaterials.Add(Set32BitColour(afDiffuse[0], afDiffuse[1], afDiffuse[2], 1.0f - fTransparency));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadIndexedFaceSet(SVRMLShape* psShape)
{
	char	szField[VRML_MAX_NAME_LENGTH];
	bool	bNormalPerVertex;

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "coord") == 0)
		{
			ReturnOnFalse(ReadCoordinate(psShape));
		}
		else if (StringCompare(szField, "coordIndex") == 0)
		{
			ReturnOnFalse(ReadCoordIndices(psShape));
		}
		else if (StringCompare(szField, "normal") == 0)
		{
			ReturnOnFalse(ReadNormal(psShape));
		}
		else if (StringCompare(szField, "normalPerVertex") == 0)
		{
			ReturnOnFalse(ReadBool(&bNormalPerVertex));
			if (!bNormalPerVertex)
			{
				return Error("normalPerVertex FALSE is not supported.");
			}
		}
		else
		{
			return Error("Unsupported IndexedFaceSet field: ", szField);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadCoordinate(SVRMLShape* psShape)
{
	char	szType[VRML_MAX_NAME_LENGTH];
	char	szField[VRML_MAX_NAME_LENGTH];

	ReturnOnFalse(ReadDefinition(szType));
	if (StringCompare(szType, "Coordinate") != 0)
	{
		return Error("Unsupported coord: ", szType);
	}

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "point") != 0)
		{
			return Error("Unsupported Coordinate field: ", szField);
		}
		ReturnOnFalse(ReadVectors(&masPositions));
	}

	psShape->iNumPositions = (int)masPositions.NumElements() - psShape->iFirstPosition;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadNormal(SVRMLShape* psShape)
{
	char	szType[VRML_MAX_NAME_LENGTH];
	char	szField[VRML_MAX_NAME_LENGTH];

	ReturnOnFalse(ReadDefinition(szType));
	if (StringCompare(szType, "Normal") != 0)
	{
		return Error("Unsupported normal: ", szType);
	}

	ReturnOnFalse(ReadExact('{'));
	while (mcParser.GetExactCharacter('}') != TRITRUE)
	{
		ReturnOnFalse(ReadName(szField));
		if (StringCompare(szField, "vector") != 0)
		{
			return Error("Unsupported Normal field: ", szField);
		}
		ReturnOnFalse(ReadVectors(&masNormals));
	}

	psShape->iNumNormals = (int)masNormals.NumElements() - psShape->iFirstNormal;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Faces are lists of corners ended by -1 (which the last face can leave out).  Faces with more than three corners are
// split into a fan of triangles because VRML's default is that they are convex.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadCoordIndices(SVRMLShape* psShape)
{
	int32	iIndex;
	int32	iFirst;
	int32	iPrevious;
	int		iNumCorners;
	bool	bEnd;

	iFirst = 0;
	iPrevious = 0;
	iNumCorners = 0;
	bEnd = false;

	ReturnOnFalse(ReadExact('['));
	while (!bEnd)
	{
		if (mcParser.GetExactCharacter(']') == TRITRUE)
		{
			iIndex = -1;
			bEnd = true;
		}
		else
		{
			ReturnOnFalse(ReadInt(&iIndex));
			if (iIndex < -1)
			{
				return Error("A coordIndex is negative.");
			}
		}

		if (iIndex == -1)
		{
			if ((iNumCorners != 0) && (iNumCorners < 3))
			{
				return Error("A face has fewer than three corners.");
			}
			iNumCorners = 0;
		}
		else
		{
			if (iNumCorners == 0)
			{
				iFirst = iIndex;
			}
			else if (iNumCorners >= 2)
			{
				maiIndices.Add(iFirst);
				maiIndices.Add(iPrevious);
				maiIndices.Add(iIndex);
				psShape->iNumIndices += 3;
			}
			iPrevious = iIndex;
			iNumCorners++;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the type of a node whose DEF name nothing can refer to.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::ReadDefinition(char* szType)
{
	char	szName[VRML_MAX_NAME_LENGTH];

	ReturnOnFalse(ReadName(szType));
	if (StringCompare(szType, "USE") == 0)
	{
		return Error("USE is only supported for Transforms, Shapes and Appearances.");
	}
	if (StringCompare(szType, "DEF") == 0)
	{
		ReturnOnFalse(ReadName(szName));
		ReturnOnFalse(ReadName(szType));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The mesh's corners and faces are ints.  Counting here also stops USE from multiplying a small file into a huge mesh.
//
//////////////////////////////////////////////////////////////////////////
bool CVRMLReader::AddChild(int iParent, int iChild, int64 iNumCorners, int64 iNumFaces)
{
	SVRMLTransform*	psParent;

	psParent = masTransforms.Get(iParent);
	psParent->iNumCorners += iNumCorners;
	psParent->iNumFaces += iNumFaces;
	if ((psParent->iNumCorners > MAX_INT) || (psParent->iNumFaces > MAX_INT))
	{
		return Error("The mesh would have too many corners or faces.");
	}

	psParent->aiChildren.Add(iChild);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVRMLReader::BuildMesh(void)
{
	SFloat4x4	sIdentity;
	size		i;
	size		uiNumElements;

	mpcMesh->ClearConnectivity();
	mpcMesh->mcNormals.mbInUse = true;
	mpcMesh->mcColours.mbInUse = true;

	uiNumElements = mauiMaterials.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		mpcMesh->mcColours.mcColours.Add(mauiMaterials.GetValue(i));
	}

	//mcNormals is 0 .. Flats, Flats .. End so the corner normals' indices start at the number of faces.
	miNumFaces = (int)masTransforms.Get(0)->iNumFaces;
	miNextNormal = miNumFaces;

	sIdentity.Identity();
	AddFaces(0, &sIdentity);
	AddNormals(0, &sIdentity);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVRMLReader::AddFaces(int iTransform, SFloat4x4* psParent)
{
	SVRMLTransform*		psTransform;
	SVRMLShape*			psShape;
	SFloat4x4			sMatrix;
	SFloat32Vec3		sPosition;
	SFloat32Vec3		sAcross;
	SMeshNormalFace*	psNormalFace;
	SMeshFaceColour*	psColourFace;
	size				i;
	size				uiNumChildren;
	int					iChild;
	int					j;
	int					iFirstCorner;
	int					ai[3];
	bool				bMirrored;
	CMFRet				r;

	psTransform = masTransforms.Get(iTransform);
	Float4x4Multiply(&sMatrix, &psTransform->sMatrix, psParent);

	//A mirrored transform turns faces inside out.
	Float32Vec3Cross(&sAcross, (SFloat32Vec3*)&sMatrix.y, (SFloat32Vec3*)&sMatrix.z);
	bMirrored = (sAcross.x * sMatrix.x.x + sAcross.y * sMatrix.x.y + sAcross.z * sMatrix.x.z) < 0.0f;

	uiNumChildren = psTransform->aiChildren.NumElements();
	for (i = 0; i < uiNumChildren; i++)
	{
		iChild = psTransform->aiChildren.GetValue(i);
		if (iChild >= 0)
		{
			AddFaces(iChild, &sMatrix);
			continue;
		}

		psShape = masShapes.Get(~iChild);
		iFirstCorner = mpcMesh->NumCorners();
		for (j = 0; j < psShape->iNumPositions; j++)
		{
			Float32Vec3TransformCoord(&sPosition, masPositions.Get(psShape->iFirstPosition + j), &sMatrix);
			mpcMesh->AddCorner(&sPosition);
		}

		for (j = 0; j < psShape->iNumIndices; j += 3)
		{
			ai[0] = maiIndices.GetValue(psShape->iFirstIndex + j);
			ai[1] = maiIndices.GetValue(psShape->iFirstIndex + j + (bMirrored ? 2 : 1));
			ai[2] = maiIndices.GetValue(psShape->iFirstIndex + j + (bMirrored ? 1 : 2));

			r = mpcMesh->AddFace(iFirstCorner + ai[0], iFirstCorner + ai[1], iFirstCorner + ai[2]);

			if (psShape->iNumNormals != 0)
			{
				psNormalFace = mpcMesh->mcNormals.mcFaces.Get(r.miFirstFace);
				psNormalFace->aiCornerNormals[0] = miNextNormal + ai[0];
				psNormalFace->aiCornerNormals[1] = miNextNormal + ai[1];
				psNormalFace->aiCornerNormals[2] = miNextNormal + ai[2];
			}

			psColourFace = mpcMesh->mcColours.mcFaces.Get(r.miFirstFace);
			psColourFace->aiCornerColours[0] = psShape->iMaterial;
			psColourFace->aiCornerColours[1] = psShape->iMaterial;
			psColourFace->aiCornerColours[2] = psShape->iMaterial;
		}
		miNextNormal += psShape->iNumNormals;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Must visit the shapes in the same order as AddFaces.  A normal is multiplied by the matrix's cofactors so that scales
// that are not uniform keep it perpendicular to its surface.
//
//////////////////////////////////////////////////////////////////////////
void CVRMLReader::AddNormals(int iTransform, SFloat4x4* psParent)
{
	SVRMLTransform*	psTransform;
	SVRMLShape*		psShape;
	SFloat4x4		sMatrix;
	SFloat32Vec3	sYZ;
	SFloat32Vec3	sZX;
	SFloat32Vec3	sXY;
	SFloat32Vec3*	psIn;
	SFloat32Vec3	sOut;
	size			i;
	size			uiNumChildren;
	int				iChild;
	int				j;
	float32			fSign;

	psTransform = masTransforms.Get(iTransform);
	Float4x4Multiply(&sMatrix, &psTransform->sMatrix, psParent);

	Float32Vec3Cross(&sYZ, (SFloat32Vec3*)&sMatrix.y, (SFloat32Vec3*)&sMatrix.z);
	Float32Vec3Cross(&sZX, (SFloat32Vec3*)&sMatrix.z, (SFloat32Vec3*)&sMatrix.x);
	Float32Vec3Cross(&sXY, (SFloat32Vec3*)&sMatrix.x, (SFloat32Vec3*)&sMatrix.y);
	fSign = ((sYZ.x * sMatrix.x.x + sYZ.y * sMatrix.x.y + sYZ.z * sMatrix.x.z) < 0.0f) ? -1.0f : 1.0f;

	uiNumChildren = psTransform->aiChildren.NumElements();
	for (i = 0; i < uiNumChildren; i++)
	{
		iChild = psTransform->aiChildren.GetValue(i);
		if (iChild >= 0)
		{
			AddNormals(iChild, &sMatrix);
			continue;
		}

		psShape = masShapes.Get(~iChild);
		for (j = 0; j < psShape->iNumNormals; j++)
		{
			psIn = masNormals.Get(psShape->iFirstNormal + j);
			sOut.x = fSign * (psIn->x * sYZ.x + psIn->y * sZX.x + psIn->z * sXY.x);
			sOut.y = fSign * (psIn->x * sYZ.y + psIn->y * sZX.y + psIn->z * sXY.y);
			sOut.z = fSign * (psIn->x * sYZ.z + psIn->y * sZX.z + psIn->z * sXY.z);
			sOut.Normalize();
			mpcMesh->AddNormal(sOut.x, sOut.y, sOut.z);
		}
	}
}

