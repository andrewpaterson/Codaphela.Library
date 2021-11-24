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
	Init(pcParser->Line(), pcParser->Column(), pcFile->ShortName());
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
	pszUninitialised->Append(miLine);
	pszUninitialised->Append(", ");
	pszUninitialised->Append(miColumn);
	pszUninitialised->Append("): ");
	return pszUninitialised;
}

