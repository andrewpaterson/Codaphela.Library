#include "BaseLib/Logger.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "GerberFileSets.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberFileSets::Init(void)
{
	macSets.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGerberFileSets::Kill(void)
{
	size			i;
	CGerberFileSet*	pcSet;

	for (i = 0; i < macSets.NumElements(); i++)
	{
		pcSet = macSets.GetPtr(i);
		SafeKill(pcSet);
	}
	macSets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberFileSet* CGerberFileSets::Add(char* szDirectory)
{
	CGerberFileSet*	pcSet;

	pcSet = NewMalloc<CGerberFileSet>();
	pcSet->Init();
	if (!pcSet->Load(szDirectory))
	{
		SafeKill(pcSet);
		return NULL;
	}

	if (Get(pcSet->GetName()) != NULL)
	{
		gcLogger.Error2(__METHOD__, " A file set named [", pcSet->GetName(), "] has already been added.", NULL);
		SafeKill(pcSet);
		return NULL;
	}

	macSets.Add(pcSet);
	return pcSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGerberFileSets::AddDirectories(char* szParentDirectory)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszDirectories;
	size		i;
	bool		bResult;

	//Each directory below szParentDirectory is one board.
	aszDirectories.Init();
	cFileUtil.FindAllDirectories(szParentDirectory, &aszDirectories, false);
	aszDirectories.QuickSort();

	bResult = true;
	for (i = 0; i < aszDirectories.NumElements(); i++)
	{
		if (Add(aszDirectories.Get(i)->Text()) == NULL)
		{
			bResult = false;
			break;
		}
	}
	aszDirectories.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGerberFileSet* CGerberFileSets::Get(char* szName)
{
	size			i;
	CGerberFileSet*	pcSet;

	for (i = 0; i < macSets.NumElements(); i++)
	{
		pcSet = macSets.GetPtr(i);
		if (StringCompare(pcSet->GetName(), szName) == 0)
		{
			return pcSet;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGerberFileSets::NumSets(void) {return macSets.NumElements();}
CGerberFileSet* CGerberFileSets::Get(size uiIndex) {return macSets.SafeGetPtr(uiIndex);}

