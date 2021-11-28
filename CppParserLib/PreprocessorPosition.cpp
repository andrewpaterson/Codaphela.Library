#include "BaseLib/Chars.h"
#include "CFile.h"
#include "PreprocessorTokenParser.h"
#include "PreprocessorPosition.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(int iLine, int iColumn, char* szShortName)
{
	miLine = iLine;
	miColumn = iColumn;
	mszShortName = szShortName;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(CPreprocessorTokenParser* pcParser, CCFile* pcFile)
{
	char*	szShortName;

	if (pcFile)
	{
		szShortName = pcFile->ShortName();
	}
	else
	{
		szShortName = NULL;
	}
	Init(pcParser->Line(), pcParser->Column(), szShortName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* SPreprocessorPosition::Message(CChars* pszUninitialised)
{
	pszUninitialised->Init();
	pszUninitialised->Append(mszShortName);
	pszUninitialised->Append(" (");
	pszUninitialised->Append(miLine + 1);
	pszUninitialised->Append(", ");
	pszUninitialised->Append(miColumn + 1);
	pszUninitialised->Append("): ");
	return pszUninitialised;
}

