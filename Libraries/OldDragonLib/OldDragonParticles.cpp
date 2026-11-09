/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "OldDragonReader.h"


#define PARTICLE_MAX_COLUMNS	16
#define PARTICLE_FIELD_SIZE		64


enum EOldDragonParticleColumn
{
	OPC_Ignored,
	OPC_X,
	OPC_Y,
	OPC_Z,
	OPC_Red,
	OPC_Green,
	OPC_Blue,
	OPC_Radius,
	OPC_Intensity,
};


//////////////////////////////////////////////////////////////////////////
//
// Copies the next comma separated field on the line without the spaces
// around it, and returns what follows it: the end of the line, or just
// past the comma.  bMore says whether there was a comma.
//
//////////////////////////////////////////////////////////////////////////
static const char* NextParticleField(const char* szLine, const char* szEnd, char* szField, bool* pbMore)
{
	const char*	szStart;
	const char*	szStop;
	size		uiLength;

	szStart = szLine;
	szStop = szLine;
	while ((szStop < szEnd) && (*szStop != ','))
	{
		szStop++;
	}
	*pbMore = (szStop < szEnd);

	szLine = szStop;
	while ((szStart < szStop) && ((*szStart == ' ') || (*szStart == '\t')))
	{
		szStart++;
	}
	while ((szStop > szStart) && ((szStop[-1] == ' ') || (szStop[-1] == '\t') || (szStop[-1] == '\r')))
	{
		szStop--;
	}

	uiLength = (size)(szStop - szStart);
	if (uiLength >= PARTICLE_FIELD_SIZE)
	{
		uiLength = PARTICLE_FIELD_SIZE - 1;
	}
	memcpy(szField, szStart, uiLength);
	szField[uiLength] = 0;

	return *pbMore ? szLine + 1 : szLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static EOldDragonParticleColumn ParticleColumn(const char* szHeading)
{
	if (strcmp(szHeading, "X") == 0)
	{
		return OPC_X;
	}
	if (strcmp(szHeading, "Y") == 0)
	{
		return OPC_Y;
	}
	if (strcmp(szHeading, "Z") == 0)
	{
		return OPC_Z;
	}
	if (strcmp(szHeading, "R") == 0)
	{
		return OPC_Red;
	}
	if (strcmp(szHeading, "G") == 0)
	{
		return OPC_Green;
	}
	if (strcmp(szHeading, "B") == 0)
	{
		return OPC_Blue;
	}
	if (strcmp(szHeading, "Radius") == 0)
	{
		return OPC_Radius;
	}
	if (strcmp(szHeading, "Intensity") == 0)
	{
		return OPC_Intensity;
	}
	return OPC_Ignored;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetParticleField(SOldDragonParticle* psParticle, EOldDragonParticleColumn eColumn, float32 fValue)
{
	switch (eColumn)
	{
	case OPC_X:
		psParticle->sPosition.x = fValue;
		break;
	case OPC_Y:
		psParticle->sPosition.y = fValue;
		break;
	case OPC_Z:
		psParticle->sPosition.z = fValue;
		break;
	case OPC_Red:
		psParticle->fRed = fValue;
		break;
	case OPC_Green:
		psParticle->fGreen = fValue;
		break;
	case OPC_Blue:
		psParticle->fBlue = fValue;
		break;
	case OPC_Radius:
		psParticle->fRadius = fValue;
		break;
	case OPC_Intensity:
		psParticle->fIntensity = fValue;
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static const char* EndOfParticleLine(const char* szLine, const char* szFileEnd)
{
	while ((szLine < szFileEnd) && (*szLine != '\n'))
	{
		szLine++;
	}
	return szLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool ReadParticleLines(const char* szFilename, const char* szText, const char* szFileEnd, COldDragonScene* pcScene)
{
	EOldDragonParticleColumn	aeColumns[PARTICLE_MAX_COLUMNS];
	size						uiNumColumns;
	size						uiColumn;
	char						szField[PARTICLE_FIELD_SIZE];
	const char*					szLine;
	const char*					szLineEnd;
	bool						bMore;
	bool						bX;
	bool						bY;
	SOldDragonParticle*			psParticle;

	szLine = szText;
	szLineEnd = EndOfParticleLine(szLine, szFileEnd);
	uiNumColumns = 0;
	bX = false;
	bY = false;
	do
	{
		if (uiNumColumns == PARTICLE_MAX_COLUMNS)
		{
			return gcLogger.Error2(__METHOD__, " Particle file [", szFilename, "] has too many columns.", NULL);
		}
		szLine = NextParticleField(szLine, szLineEnd, szField, &bMore);
		aeColumns[uiNumColumns] = ParticleColumn(szField);
		bX |= (aeColumns[uiNumColumns] == OPC_X);
		bY |= (aeColumns[uiNumColumns] == OPC_Y);
		uiNumColumns++;
	} while (bMore);

	if (!bX || !bY)
	{
		return gcLogger.Error2(__METHOD__, " Particle file [", szFilename, "] needs headings X and Y.", NULL);
	}

	for (szLine = szLineEnd; szLine < szFileEnd; szLine = szLineEnd)
	{
		szLine++;
		szLineEnd = EndOfParticleLine(szLine, szFileEnd);

		//A blank line, and the one after the last newline, aren't points.
		NextParticleField(szLine, szLineEnd, szField, &bMore);
		if (!bMore && (szField[0] == 0))
		{
			continue;
		}

		psParticle = pcScene->macParticles.Add();
		memset(psParticle, 0, sizeof(SOldDragonParticle));
		psParticle->fRed = 1.0f;
		psParticle->fGreen = 1.0f;
		psParticle->fBlue = 1.0f;
		psParticle->fRadius = 1.0f;
		psParticle->fIntensity = 1.0f;

		uiColumn = 0;
		do
		{
			if (uiColumn == uiNumColumns)
			{
				return gcLogger.Error2(__METHOD__, " Particle file [", szFilename, "] has a line with too many values.", NULL);
			}
			szLine = NextParticleField(szLine, szLineEnd, szField, &bMore);
			SetParticleField(psParticle, aeColumns[uiColumn], (float32)atof(szField));
			uiColumn++;
		} while (bMore);

		if (uiColumn != uiNumColumns)
		{
			return gcLogger.Error2(__METHOD__, " Particle file [", szFilename, "] has a line with too few values.", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ReadOldDragonParticles(const char* szFilename, COldDragonScene* pcScene)
{
	CTextFile	cFile;
	bool		bResult;

	cFile.Init();
	bResult = cFile.Read(szFilename);
	if (bResult)
	{
		bResult = ReadParticleLines(szFilename, cFile.Text(), cFile.Text() + cFile.Length(), pcScene);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not read particle file [", szFilename, "].", NULL);
	}
	cFile.Kill();
	return bResult;
}

