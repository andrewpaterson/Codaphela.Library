/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/XMLFile.h"
#include "BaseLib/LogString.h"
#include "CircuitFile.h"


//////////////////////////////////////////////////////////////////////////
//
// The whole document; the writer must be initialised and empty.
//
//////////////////////////////////////////////////////////////////////////
void SaveCircuitXML(CCircuitView* pcCircuit, CXMLWriter* pcWriter)
{
	pcWriter->Begin("Logicim");
	pcWriter->Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	pcCircuit->SaveXML(pcWriter);
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitFile(CCircuitView* pcCircuit, const char* szFilename)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	SaveCircuitXML(pcCircuit, &cWriter);
	bResult = cWriter.Write(szFilename);
	cWriter.Kill();
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not write [", szFilename, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitFile(CCircuitView* pcCircuit, const char* szFilename)
{
	CXMLFile	cFile;
	CFileUtil	cFileUtil;
	CChars		szName;
	CChars		szDirectory;
	CMarkupTag*	pcRoot;
	CMarkupTag*	pcCircuitTag;
	int32		iVersion;
	bool		bResult;

	szName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szFilename, &szName, &szDirectory);
	if (szDirectory.Empty())
	{
		szDirectory.Set(".");
	}

	cFile.Init();
	bResult = cFile.Read(szName.Text(), szDirectory.Text());
	szDirectory.Kill();
	szName.Kill();
	if (!bResult)
	{
		cFile.Kill();
		return false;
	}

	pcRoot = cFile.mcMarkup.mpcDoc->GetRootTag();
	if ((pcRoot == NULL) || !pcRoot->Is("Logicim"))
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " [", szFilename, "] isn't a Logicim circuit.", NULL);
	}
	if (!pcRoot->GetInt32Attribute("version", &iVersion) || (iVersion != CIRCUIT_FILE_VERSION))
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " [", szFilename, "] is not version [", IntToString((int32)CIRCUIT_FILE_VERSION), "].", NULL);
	}

	pcCircuitTag = pcRoot->GetTag("Circuit");
	if (pcCircuitTag == NULL)
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " [", szFilename, "] has no [Circuit].", NULL);
	}

	bResult = pcCircuit->LoadXML(pcCircuitTag);
	cFile.Kill();
	return bResult;
}

