#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "GerberParser.h"
#include "GerberLayerFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerFile::Init(void)
{
	mszFileName.Init();
	mcCommands.Init();
	meLayer = GBL_Unknown;
	meSide = GBS_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerFile::Kill(void)
{
	mcCommands.Kill();
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberLayerFile::Load(char* szPathName)
{
	CTextFile		cFile;
	CGerberParser	cParser;
	CFileUtil		cFileUtil;
	TRISTATE		tResult;

	cFileUtil.GetFileName(&mszFileName, szPathName);

	cFile.Init();
	if (!cFile.Read(szPathName))
	{
		cFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read Gerber file [", szPathName, "].", NULL);
		return false;
	}

	//KiCad writes CRLF line endings.
	cFile.PassifyNewlines();

	cParser.Init(cFile.Text(), cFile.Length(), szPathName, &mcCommands);
	tResult = cParser.Parse();
	cParser.Kill();
	cFile.Kill();

	if (tResult != TRITRUE)
	{
		gcLogger.Error2(__METHOD__, " Could not parse Gerber file [", szPathName, "].", NULL);
		return false;
	}

	IdentifyLayer();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberLayerFile::IdentifyLayer(void)
{
	size							i;
	CGerberCommand*					pcCommand;
	CGerberCommandFileAttribute*	pcAttribute;
	char*							szFunction;
	size							uiNumValues;

	meLayer = GBL_Unknown;
	meSide = GBS_Unknown;
	for (i = 0; i < mcCommands.NumCommands(); i++)
	{
		pcCommand = mcCommands.GetCommand(i);
		if (!pcCommand->IsFileAttribute())
		{
			continue;
		}

		pcAttribute = (CGerberCommandFileAttribute*)pcCommand;
		uiNumValues = pcAttribute->NumValues();
		if ((StringCompare(pcAttribute->NameText(), ".FileFunction") != 0) || (uiNumValues == 0))
		{
			continue;
		}

		//eg: Copper,L1,Top  Soldermask,Bot  Paste,Top  Legend,Bot  Profile,NP
		szFunction = pcAttribute->ValueText(0);
		if (StringCompare(szFunction, "Copper") == 0)
		{
			meLayer = GBL_Copper;
			if (uiNumValues > 2)
			{
				meSide = GetSide(pcAttribute->ValueText(2));
			}
		}
		else if ((StringCompare(szFunction, "Profile") == 0) || (StringCompare(szFunction, "NonPlated") == 0) || (StringCompare(szFunction, "Plated") == 0))
		{
			meLayer = (StringCompare(szFunction, "Profile") == 0) ? GBL_Outline : GBL_Drill;
			meSide = GBS_Both;
		}
		else
		{
			if (StringCompare(szFunction, "Soldermask") == 0)
			{
				meLayer = GBL_SolderMask;
			}
			else if (StringCompare(szFunction, "Paste") == 0)
			{
				meLayer = GBL_Paste;
			}
			else if (StringCompare(szFunction, "Legend") == 0)
			{
				meLayer = GBL_Silk;
			}
			else if (StringCompare(szFunction, "Carbonmask") == 0)
			{
				meLayer = GBL_Carbon;
			}
			else if (StringCompare(szFunction, "AssemblyDrawing") == 0)
			{
				meLayer = GBL_Assembly;
			}
			if ((meLayer != GBL_Unknown) && (uiNumValues > 1))
			{
				meSide = GetSide(pcAttribute->ValueText(1));
			}
		}
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EGerberBoardSide CGerberLayerFile::GetSide(char* szSide)
{
	if (StringCompare(szSide, "Top") == 0)
	{
		return GBS_Top;
	}
	else if (StringCompare(szSide, "Bot") == 0)
	{
		return GBS_Bottom;
	}
	else if (StringCompare(szSide, "Inr") == 0)
	{
		return GBS_Internal;
	}
	return GBS_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberLayerFile::GetFileName(void) {return mszFileName.Text();}
CGerberCommands* CGerberLayerFile::GetCommands(void) {return &mcCommands;}
EGerberBoardLayer CGerberLayerFile::GetLayer(void) {return meLayer;}
EGerberBoardSide CGerberLayerFile::GetSide(void) {return meSide;}

