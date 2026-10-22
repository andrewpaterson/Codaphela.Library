#include "BaseLib/Logger.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "ExcellonParser.h"
#include "GerberDrillFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberDrillFile::Init(void)
{
	mszFileName.Init();
	mcCommands.Init();
	mePlating = GDP_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberDrillFile::Kill(void)
{
	mcCommands.Kill();
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberDrillFile::Load(char* szPathName)
{
	CTextFile		cFile;
	CExcellonParser	cParser;
	CFileUtil		cFileUtil;
	TRISTATE		tResult;

	cFileUtil.GetFileName(&mszFileName, szPathName);

	cFile.Init();
	if (!cFile.Read(szPathName))
	{
		cFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read drill file [", szPathName, "].", NULL);
		return false;
	}

	//The Excellon parser accepts CRLF line endings.
	cParser.Init(cFile.Text(), cFile.Length(), szPathName, &mcCommands);
	tResult = cParser.Parse();
	cParser.Kill();
	cFile.Kill();

	if (tResult != TRITRUE)
	{
		gcLogger.Error2(__METHOD__, " Could not parse drill file [", szPathName, "].", NULL);
		return false;
	}

	IdentifyPlating();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberDrillFile::IdentifyPlating(void)
{
	size						i;
	CExcellonCommand*			pcCommand;
	CExcellonCommandAttribute*	pcAttribute;
	char*						szFunction;

	//eg: ; #@! TF.FileFunction,Plated,1,2,PTH
	mePlating = GDP_Unknown;
	for (i = 0; i < mcCommands.NumCommands(); i++)
	{
		pcCommand = mcCommands.GetCommand(i);
		if (!pcCommand->IsType(EC_FileAttribute))
		{
			continue;
		}

		pcAttribute = (CExcellonCommandAttribute*)pcCommand;
		if ((StringCompare(pcAttribute->NameText(), ".FileFunction") != 0) || (pcAttribute->NumValues() == 0))
		{
			continue;
		}

		szFunction = pcAttribute->ValueText(0);
		if (StringCompare(szFunction, "Plated") == 0)
		{
			mePlating = GDP_Plated;
		}
		else if (StringCompare(szFunction, "NonPlated") == 0)
		{
			mePlating = GDP_NonPlated;
		}
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CGerberDrillFile::GetFileName(void) {return mszFileName.Text();}
CExcellonCommands* CGerberDrillFile::GetCommands(void) {return &mcCommands;}
EGerberDrillPlating CGerberDrillFile::GetPlating(void) {return mePlating;}

