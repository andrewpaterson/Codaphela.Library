#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "GerberLayerMerger.h"
#include "ExcellonDrillMerger.h"
#include "GerberPanelFiles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelFiles::Init(CGerberPanel* pcPanel)
{
	mpcPanel = pcPanel;
	mcOutlines.Init();
	mcTabs.Init();
	mcEdgeCuts.Init();
	maszFiles.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelFiles::Kill(void)
{
	maszFiles.Kill();
	mcEdgeCuts.Kill();
	mcTabs.Kill();
	mcOutlines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelFiles::Calculate(void)
{
	return mcOutlines.Read(mpcPanel) && mcTabs.Calculate(mpcPanel, &mcOutlines) && mcEdgeCuts.Calculate(&mcOutlines, &mcTabs);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberPanelFiles::GetSuffix(const char* szFileName, CChars* pszSuffix)
{
	const char*	szDash;

	//KiCad names files <project>-<layer>.<extension>.
	szDash = strrchr(szFileName, '-');
	pszSuffix->Append(szDash ? szDash + 1 : szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelFiles::WriteFile(const char* szDirectory, const char* szName, const char* szSuffix, CChars* pszText)
{
	CChars		szFileName;
	CChars		szPathName;
	CTextFile	cFile;
	bool		bResult;

	szFileName.Init(szName);
	szFileName.Append('-');
	szFileName.Append(szSuffix);
	szPathName.Init(szDirectory);
	szPathName.Append(FILE_SEPARATOR);
	szPathName.Append(&szFileName);

	cFile.Init(pszText->Text());
	bResult = cFile.Write(szPathName.Text());
	cFile.Kill();
	if (bResult)
	{
		maszFiles.Add(szFileName.Text());
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not write [", szPathName.Text(), "].", NULL);
	}
	szPathName.Kill();
	szFileName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelFiles::Write(const char* szDirectory, const char* szName)
{
	CChars				szText;
	CChars				szSuffix;
	CGerberLayerFile*	pcOutline;
	bool				bResult;

	maszFiles.ReInit();
	if (!WriteLayers(szDirectory, szName))
	{
		return false;
	}

	//The panel's own edge cuts replace the boards'.
	szText.Init();
	szSuffix.Init();
	pcOutline = mpcPanel->GetPlacement((size)0)->mpcSet->GetLayer(GBL_Outline, GBS_Both);
	GetSuffix(pcOutline ? pcOutline->GetFileName() : "Edge_Cuts.gm1", &szSuffix);
	bResult = mcEdgeCuts.Write(&szText) && WriteFile(szDirectory, szName, szSuffix.Text(), &szText);
	szSuffix.Kill();
	szText.Kill();
	if (!bResult)
	{
		return false;
	}

	return WriteDrills(szDirectory, szName, GDP_Plated, "PTH.drl") && WriteDrills(szDirectory, szName, GDP_NonPlated, "NPTH.drl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelFiles::WriteLayers(const char* szDirectory, const char* szName)
{
	size					i;
	size					j;
	size					k;
	CGerberFileSet*			pcSet;
	CGerberLayerFile*		pcLayer;
	CGerberLayerMerger		cMerger;
	CChars					szText;
	CChars					szSuffix;
	bool					bDone;
	bool					bResult;

	//Every layer any board has, except the edge cuts, once each.
	for (i = 0; i < mpcPanel->NumPlacements(); i++)
	{
		pcSet = mpcPanel->GetPlacement(i)->mpcSet;
		for (j = 0; j < pcSet->NumLayers(); j++)
		{
			pcLayer = pcSet->GetLayer(j);
			if ((pcLayer->GetLayer() == GBL_Outline) || (pcLayer->GetLayer() == GBL_Unknown))
			{
				continue;
			}
			bDone = false;
			for (k = 0; (k < i) && !bDone; k++)
			{
				bDone = mpcPanel->GetPlacement(k)->mpcSet->GetLayer(pcLayer->GetLayer(), pcLayer->GetSide()) != NULL;
			}
			if (bDone)
			{
				continue;
			}

			szText.Init();
			szSuffix.Init();
			GetSuffix(pcLayer->GetFileName(), &szSuffix);
			cMerger.Init(mpcPanel);
			bResult = cMerger.Merge(pcLayer->GetLayer(), pcLayer->GetSide(), &szText) && WriteFile(szDirectory, szName, szSuffix.Text(), &szText);
			cMerger.Kill();
			szSuffix.Kill();
			szText.Kill();
			if (!bResult)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberPanelFiles::WriteDrills(const char* szDirectory, const char* szName, EGerberDrillPlating ePlating, const char* szDefaultSuffix)
{
	size					i;
	CGerberDrillFile*		pcDrill;
	CExcellonDrillMerger	cMerger;
	CChars					szText;
	CChars					szSuffix;
	CArrayFloat64Vec2		asHoles;
	CArrayChars				aszFunction;
	CNumber					cDiameter;
	bool					bResult;

	szSuffix.Init();
	for (i = 0; i < mpcPanel->NumPlacements(); i++)
	{
		pcDrill = mpcPanel->GetPlacement(i)->mpcSet->GetDrill(ePlating);
		if (pcDrill)
		{
			GetSuffix(pcDrill->GetFileName(), &szSuffix);
			break;
		}
	}
	if (szSuffix.Empty())
	{
		szSuffix.Append(szDefaultSuffix);
	}

	cMerger.Init(mpcPanel);
	bResult = cMerger.AddBoards(ePlating);
	if (bResult && (ePlating == GDP_NonPlated))
	{
		//The mouse bites.
		asHoles.Init();
		mcTabs.GetMouseBites(&asHoles);
		MillimetresToNumber(mcTabs.GetHoleDiameter(), &cDiameter);
		aszFunction.Init();
		aszFunction.Add("NonPlated");
		aszFunction.Add("NPTH");
		aszFunction.Add("OtherDrill");
		aszFunction.Add("MouseBite");
		cMerger.AddHoles(&cDiameter, &aszFunction, asHoles.GetData(), asHoles.NumElements());
		aszFunction.Kill();
		asHoles.Kill();
	}

	szText.Init();
	bResult = bResult && cMerger.Write(&szText) && WriteFile(szDirectory, szName, szSuffix.Text(), &szText);
	szText.Kill();
	cMerger.Kill();
	szSuffix.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberPanelOutlines* CGerberPanelFiles::GetOutlines(void) {return &mcOutlines;}
CGerberPanelTabs* CGerberPanelFiles::GetTabs(void) {return &mcTabs;}
CGerberPanelEdgeCuts* CGerberPanelFiles::GetEdgeCuts(void) {return &mcEdgeCuts;}
size CGerberPanelFiles::NumFiles(void) {return maszFiles.NumElements();}
char* CGerberPanelFiles::GetFile(size uiIndex) {return maszFiles.Get(uiIndex)->Text();}

