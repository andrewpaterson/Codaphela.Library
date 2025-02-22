#include "BaseLib/Chars.h"
#include "SourceFile.h"
#include "PreprocessorTokenParser.h"
#include "PreprocessorPosition.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(size iLine, size iColumn, char* szShortName)
{
	miLine = iLine;
	miColumn = iColumn;
	mszShortName = szShortName;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPreprocessorPosition::Init(CPreprocessorTokenParser* pcParser, char* szShortName)
{
	if (pcParser)
	{
		Init(pcParser->Line(), pcParser->Column(), szShortName);
	}
	else
	{
		Init(-1, -1, szShortName);
	}
	
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

