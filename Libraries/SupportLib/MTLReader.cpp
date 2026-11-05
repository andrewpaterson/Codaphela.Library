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
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextParser.h"
#include "Material.h"
#include "MTLReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMTLReader::Init(CMaterial* pcMaterial, char *szFilename)
{
	mpcMaterial = pcMaterial;
	mszFileName.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMTLReader::Kill(void)
{
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMTLReader::Read(void)
{
//	CTextParser 		cTextParser;
	CTextFile			cTextFile;
//	int					iType;
//	int					iNumVerts;
	CArrayChars		cLines;
//	CChars*				pszLine;
//	int					i;
//	TRISTATE			tResult;

	cTextFile.Init();
	if (!cTextFile.Read(mszFileName.Text()))
	{
		cTextFile.Kill();
		return false;
	}

	cTextFile.PassifyNewlines();
	cLines.Fake();
	cTextFile.mcText.SplitLines(&cLines);

	return true;
}
