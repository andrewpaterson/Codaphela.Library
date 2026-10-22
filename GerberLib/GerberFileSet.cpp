#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "GerberInterpreter.h"
#include "GerberOutlineBounds.h"
#include "GerberFileSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberFileSet::Init(void)
{
	mszName.Init();
	mszDirectory.Init();
	macLayers.Init();
	macDrills.Init();
	mbOutlineBounds = false;
	mcOutlineBounds.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberFileSet::Kill(void)
{
	size				i;
	CGerberLayerFile*	pcLayer;
	CGerberDrillFile*	pcDrill;

	for (i = 0; i < macDrills.NumElements(); i++)
	{
		pcDrill = macDrills.GetPtr(i);
		SafeKill(pcDrill);
	}
	macDrills.Kill();

	for (i = 0; i < macLayers.NumElements(); i++)
	{
		pcLayer = macLayers.GetPtr(i);
		SafeKill(pcLayer);
	}
	macLayers.Kill();

	mszDirectory.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::Load(char* szDirectory)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszFiles;
	CChars		szFileName;
	size		i;
	char*		szPathName;
	bool		bResult;

	mszDirectory.Append(szDirectory);
	if (mszDirectory.EndsWith(FILE_SEPARATOR))
	{
		mszDirectory.RemoveFromEnd(1);
	}
	cFileUtil.GetFileName(&mszName, mszDirectory.Text());

	aszFiles.Init();
	if (!cFileUtil.FindAllFiles(mszDirectory.Text(), &aszFiles, false, false))
	{
		aszFiles.Kill();
		gcLogger.Error2(__METHOD__, " Could not list directory [", mszDirectory.Text(), "].", NULL);
		return false;
	}
	aszFiles.QuickSort();

	bResult = true;
	for (i = 0; i < aszFiles.NumElements(); i++)
	{
		szPathName = aszFiles.Get(i)->Text();
		szFileName.Init();
		cFileUtil.GetFileName(&szFileName, szPathName);
		if (IsDrillFileName(&szFileName))
		{
			bResult = LoadDrill(szPathName);
		}
		else if (IsGerberFileName(&szFileName))
		{
			bResult = LoadLayer(szPathName);
		}
		szFileName.Kill();

		if (!bResult)
		{
			break;
		}
	}
	aszFiles.Kill();

	if (!bResult)
	{
		return false;
	}
	if (macLayers.NumElements() == 0)
	{
		gcLogger.Error2(__METHOD__, " No Gerber files in directory [", mszDirectory.Text(), "].", NULL);
		return false;
	}
	return CalculateOutlineBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::CalculateOutlineBounds(void)
{
	CGerberLayerFile*		pcOutline;
	CGerberInterpreter		cInterpreter;
	CGerberOutlineBounds	cBounds;
	bool					bResult;

	mbOutlineBounds = false;
	mcOutlineBounds.Init();

	pcOutline = GetLayer(GBL_Outline, GBS_Both);
	if (pcOutline == NULL)
	{
		return true;
	}

	cBounds.Init();
	cInterpreter.Init(pcOutline->GetCommands(), &cBounds);
	bResult = cInterpreter.Interpret();
	cInterpreter.Kill();
	if (bResult)
	{
		mbOutlineBounds = cBounds.GetBounds(&mcOutlineBounds);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not interpret edge cuts file [", pcOutline->GetFileName(), "].", NULL);
	}
	cBounds.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::LoadLayer(char* szPathName)
{
	CGerberLayerFile*	pcLayer;

	pcLayer = NewMalloc<CGerberLayerFile>();
	pcLayer->Init();
	macLayers.Add(pcLayer);
	return pcLayer->Load(szPathName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::LoadDrill(char* szPathName)
{
	CGerberDrillFile*	pcDrill;

	pcDrill = NewMalloc<CGerberDrillFile>();
	pcDrill->Init();
	macDrills.Add(pcDrill);
	return pcDrill->Load(szPathName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::IsGerberFileName(CChars* szFileName)
{
	size	iDot;
	char*	szExtension;

	//KiCad's Protel extensions (.gtl .gbl .gts .gm1 ...) or .gbr.  Not the .gbrjob job file.
	iDot = szFileName->FindFromEnd('.');
	if (iDot == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	szExtension = szFileName->Text(iDot + 1);
	if (szFileName->EndsWithIgnoreCase(".gbr"))
	{
		return true;
	}
	return (szFileName->Length() - (iDot + 1) == 3) && ((szExtension[0] == 'g') || (szExtension[0] == 'G'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSet::IsDrillFileName(CChars* szFileName)
{
	return szFileName->EndsWithIgnoreCase(".drl");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberLayerFile* CGerberFileSet::GetLayer(EGerberBoardLayer eLayer, EGerberBoardSide eSide)
{
	size				i;
	CGerberLayerFile*	pcLayer;

	for (i = 0; i < macLayers.NumElements(); i++)
	{
		pcLayer = macLayers.GetPtr(i);
		if ((pcLayer->GetLayer() == eLayer) && (pcLayer->GetSide() == eSide))
		{
			return pcLayer;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberDrillFile* CGerberFileSet::GetDrill(EGerberDrillPlating ePlating)
{
	size				i;
	CGerberDrillFile*	pcDrill;

	for (i = 0; i < macDrills.NumElements(); i++)
	{
		pcDrill = macDrills.GetPtr(i);
		if (pcDrill->GetPlating() == ePlating)
		{
			return pcDrill;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberFileSet::GetName(void) {return mszName.Text();}
char* CGerberFileSet::GetDirectory(void) {return mszDirectory.Text();}
size CGerberFileSet::NumLayers(void) {return macLayers.NumElements();}
CGerberLayerFile* CGerberFileSet::GetLayer(size uiIndex) {return macLayers.SafeGetPtr(uiIndex);}
size CGerberFileSet::NumDrills(void) {return macDrills.NumElements();}
CGerberDrillFile* CGerberFileSet::GetDrill(size uiIndex) {return macDrills.SafeGetPtr(uiIndex);}
bool CGerberFileSet::HasOutlineBounds(void) {return mbOutlineBounds;}
CFloat64Rect* CGerberFileSet::GetOutlineBounds(void) {return &mcOutlineBounds;}

