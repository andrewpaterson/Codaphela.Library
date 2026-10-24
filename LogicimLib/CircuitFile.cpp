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
#include "MicroprocessorLib/CircuitSimulation.h"
#include "SimulationStateXML.h"
#include "CircuitFile.h"


//////////////////////////////////////////////////////////////////////////
//
// The whole document; the writer must be initialised and empty.
//
//////////////////////////////////////////////////////////////////////////
void SaveCircuitXML(CCircuitView* pcCircuit, CXMLWriter* pcWriter)
{
	SaveCircuitXML(pcCircuit, NULL, pcWriter);
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation may be NULL.
//
//////////////////////////////////////////////////////////////////////////
void SaveCircuitXML(CCircuitView* pcCircuit, CCircuitSimulation* pcSimulation, CXMLWriter* pcWriter)
{
	pcWriter->Begin("Logicim");
	pcWriter->Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	pcCircuit->SaveXML(pcWriter);
	if (pcSimulation != NULL)
	{
		SaveSimulationXML(pcWriter, pcCircuit, pcSimulation);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitFile(CCircuitView* pcCircuit, const char* szFilename)
{
	return SaveCircuitFile(pcCircuit, NULL, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitFile(CCircuitView* pcCircuit, CCircuitSimulation* pcSimulation, const char* szFilename)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	SaveCircuitXML(pcCircuit, pcSimulation, &cWriter);
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
// Reads the file and checks its root.  The file must be killed whatever
// this returns.
//
//////////////////////////////////////////////////////////////////////////
static CMarkupTag* ReadCircuitFileRoot(CXMLFile* pcFile, const char* szFilename)
{
	CFileUtil	cFileUtil;
	CChars		szName;
	CChars		szDirectory;
	CMarkupTag*	pcRoot;
	int32		iVersion;
	bool		bResult;

	szName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szFilename, &szName, &szDirectory);
	if (szDirectory.Empty())
	{
		szDirectory.Set(".");
	}
	bResult = pcFile->Read(szName.Text(), szDirectory.Text());
	szDirectory.Kill();
	szName.Kill();
	if (!bResult)
	{
		return NULL;
	}

	pcRoot = pcFile->mcMarkup.mpcDoc->GetRootTag();
	if ((pcRoot == NULL) || !pcRoot->Is("Logicim"))
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] isn't a Logicim circuit.", NULL);
		return NULL;
	}
	if (!pcRoot->GetInt32Attribute("version", &iVersion) || (iVersion != CIRCUIT_FILE_VERSION))
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] is not version [", IntToString((int32)CIRCUIT_FILE_VERSION), "].", NULL);
		return NULL;
	}
	if (pcRoot->GetTag("Circuit") == NULL)
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] has no [Circuit].", NULL);
		return NULL;
	}
	return pcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// The views only; a saved simulation is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitFile(CCircuitView* pcCircuit, const char* szFilename)
{
	CXMLFile	cFile;
	CMarkupTag*	pcRoot;
	bool		bResult;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename);
	if (pcRoot == NULL)
	{
		cFile.Kill();
		return false;
	}
	bResult = pcCircuit->LoadXML(pcRoot->GetTag("Circuit"));
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitFile(CCircuitView* pcCircuit, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, const char* szFilename)
{
	CXMLFile	cFile;
	CMarkupTag*	pcRoot;
	CMarkupTag*	pcSimulationTag;
	bool		bResult;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename);
	if (pcRoot == NULL)
	{
		cFile.Kill();
		return false;
	}

	bResult = pcCircuit->LoadXML(pcRoot->GetTag("Circuit"));
	if (bResult)
	{
		pcSimulationTag = pcRoot->GetTag("Simulation");
		if (pcSimulationTag == NULL)
		{
			bResult = pcCircuit->CreateSimulation(pcSimulation, pcFamilies, true);
		}
		else
		{
			bResult = pcCircuit->CreateSimulation(pcSimulation, pcFamilies, false);
			if (bResult)
			{
				bResult = LoadSimulationXML(pcSimulationTag, pcCircuit, pcSimulation);
			}
		}
	}
	cFile.Kill();
	return bResult;
}

