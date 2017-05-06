/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/EnumeratorVoid.h"
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
void CMTLReader::Init(CMaterial* pcMaterial, char *szFileName)
{
	mpcMaterial = pcMaterial;
	mszFileName.Init(szFileName);
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
BOOL CMTLReader::Read(void)
{
//	CTextParser 		cTextParser;
	CTextFile			cTextFile;
	CEnumeratorVoid		cTypes;
//	int					iType;
//	int					iNumVerts;
	CArrayString		cLines;
//	CChars*				pszLine;
//	int					i;
//	TRISTATE			tResult;

	cTypes.Init();
	cTypes.Add("Ns",		WMTLP_Ns);
	cTypes.Add("Ni",		WMTLP_Ni);
	cTypes.Add("d",			WMTLP_d);
	cTypes.Add("Tr",		WMTLP_Tr);
	cTypes.Add("Tf",		WMTLP_Tf);
	cTypes.Add("illum",		WMTLP_illum);
	cTypes.Add("Ka",		WMTLP_Ka);
	cTypes.Add("Kd",		WMTLP_Kd);
	cTypes.Add("Ks",		WMTLP_Ks);
	cTypes.Add("Ke",		WMTLP_Ke);
	cTypes.Add("map_Ka",	WMTLP_map_Ka);
	cTypes.Add("map_Kd",	WMTLP_map_Kd);
	cTypes.Add("map_Ks",	WMTLP_map_Ks);
	cTypes.Add("map_Ke",	WMTLP_map_Ke);
	cTypes.Add("map_d",		WMTLP_map_d);
	cTypes.Add("map_bump",	WMTLP_map_bump);
	cTypes.Add("bump",		WMTLP_bump);
	cTypes.Add("map_refl",	WMTLP_map_refl);

	cTextFile.Init();
	if (!cTextFile.Read(mszFileName.Text()))
	{
		cTypes.Kill();
		cTextFile.Kill();
		return FALSE;
	}

	cTextFile.PassifyNewlines();
	cLines.Init(256, TRUE);
	cTextFile.mcText.SplitLines(&cLines);

	return TRUE;
}
