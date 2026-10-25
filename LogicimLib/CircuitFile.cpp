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
#include "BaseLib/StringHelper.h"
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
	SaveCircuitXML(pcCircuit, NULL, NULL, pcWriter);
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation may be NULL.  If not, it's saved as the path has it.
//
//////////////////////////////////////////////////////////////////////////
void SaveCircuitXML(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CXMLWriter* pcWriter)
{
	pcWriter->Begin("Logicim");
	pcWriter->Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	pcCircuit->SaveXML(pcWriter);
	if (pcSimulation != NULL)
	{
		SaveSimulationXML(pcWriter, pcCircuit, pcViewPath, pcSimulation);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitFile(CCircuitView* pcCircuit, const char* szFilename)
{
	return SaveCircuitFile(pcCircuit, NULL, NULL, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitFile(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, const char* szFilename)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	SaveCircuitXML(pcCircuit, pcViewPath, pcSimulation, &cWriter);
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
// Reads the file and checks its root, which must hold an szContents
// element.  The file must be killed whatever this returns.
//
//////////////////////////////////////////////////////////////////////////
CMarkupTag* ReadCircuitFileRoot(CXMLFile* pcFile, const char* szFilename, const char* szContents)
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
	if (pcRoot->GetTag(szContents) == NULL)
	{
		gcLogger.Error2(__METHOD__, " [", szFilename, "] has no [", szContents, "].", NULL);
		return NULL;
	}
	return pcRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the loaded circuit in the simulation for the path: from the
// file's saved simulation if it has one of this circuit, otherwise
// started afresh.
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitFileSimulation(CMarkupTag* pcRoot, CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CMarkupTag*	pcSimulationTag;
	char*		szCircuit;

	pcSimulationTag = pcRoot->GetTag("Simulation");
	if (pcSimulationTag != NULL)
	{
		szCircuit = pcSimulationTag->GetStringAttribute("circuit");
		if (StringCompare((szCircuit != NULL) ? szCircuit : "", pcCircuit->GetTypeName()) != 0)
		{
			pcSimulationTag = NULL;
		}
	}

	if (pcSimulationTag == NULL)
	{
		return pcCircuit->CreateSimulation(pcViewPath, pcSimulation, pcFamilies, true);
	}
	if (!pcCircuit->CreateSimulation(pcViewPath, pcSimulation, pcFamilies, false))
	{
		return false;
	}
	return LoadSimulationXML(pcSimulationTag, pcCircuit, pcViewPath, pcSimulation);
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
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename, "Circuit");
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
bool LoadCircuitFile(CCircuitView* pcCircuit, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, const char* szFilename)
{
	CXMLFile	cFile;
	CMarkupTag*	pcRoot;
	bool		bResult;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename, "Circuit");
	if (pcRoot == NULL)
	{
		cFile.Kill();
		return false;
	}

	bResult = pcCircuit->LoadXML(pcRoot->GetTag("Circuit"));
	if (bResult)
	{
		bResult = LoadCircuitFileSimulation(pcRoot, pcCircuit, pcViewPath, pcSimulation, pcFamilies);
	}
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole document of a project's circuits; the writer must be
// initialised and empty.  The simulation of the top circuit on its top
// path is saved unless the simulation is NULL.
//
//////////////////////////////////////////////////////////////////////////
void SaveCircuitsXML(CCircuitViews* pcCircuits, CCircuitView* pcTop, CCircuitSimulation* pcSimulation, CXMLWriter* pcWriter)
{
	pcWriter->Begin("Logicim");
	pcWriter->Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	pcCircuits->SaveXML(pcWriter);
	if (pcSimulation != NULL)
	{
		SaveSimulationXML(pcWriter, pcTop, pcCircuits->GetTopPath(pcTop), pcSimulation);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation may be NULL, as SaveCircuitsXML.
//
//////////////////////////////////////////////////////////////////////////
bool SaveCircuitsFile(CCircuitViews* pcCircuits, CCircuitView* pcTop, CCircuitSimulation* pcSimulation, const char* szFilename)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	SaveCircuitsXML(pcCircuits, pcTop, pcSimulation, &cWriter);
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
// Adds the file's circuits.  Without a simulation (NULL) a saved one is
// ignored.  With one, the circuit of type szTop is made in it on its top
// path: from the saved simulation if the file's is of that circuit,
// otherwise started afresh.
//
//////////////////////////////////////////////////////////////////////////
bool LoadCircuitsFile(CCircuitViews* pcCircuits, const char* szTop, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, const char* szFilename)
{
	CXMLFile		cFile;
	CMarkupTag*		pcRoot;
	bool			bResult;
	CCircuitView*	pcTop;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename, "Circuits");
	if (pcRoot == NULL)
	{
		cFile.Kill();
		return false;
	}

	bResult = pcCircuits->LoadXML(pcRoot->GetTag("Circuits"));
	if (bResult && (pcSimulation != NULL))
	{
		pcTop = pcCircuits->Get(szTop);
		if (pcTop == NULL)
		{
			bResult = gcLogger.Error2(__METHOD__, " [", szFilename, "] has no circuit [", szTop, "] to simulate.", NULL);
		}
		else
		{
			bResult = LoadCircuitFileSimulation(pcRoot, pcTop, pcCircuits->GetTopPath(pcTop), pcSimulation, pcFamilies);
		}
	}
	cFile.Kill();
	return bResult;
}

