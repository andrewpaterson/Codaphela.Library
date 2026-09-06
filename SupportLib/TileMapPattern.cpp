#include "BaseLib/Logger.h"
#include "TileMapPattern.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::Init(char* szConstantSource, size iType, char* szConstantName, char* szPattern)
{
	miType = iType;
	mszConstantName = szConstantName;
	mszPattern.Init(szPattern);
	mszConstantSource = szConstantSource;
	return Done();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapPattern::Init(char* szConstantSource, size iType, char* szConstantName)
{
	miType = iType;
	mszConstantName = szConstantName;
	mszPattern.Init();
	mszConstantSource = szConstantSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapPattern::Kill(void)
{
	mszPattern.Kill();
	mszConstantName = NULL;
	miType = 0;
	mszConstantSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapPattern::AddRow(char* szRowPattern)
{
	mszPattern.Append(szRowPattern);
	mszPattern.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::Done(void)
{
	size	iIndex;
	size	iExpected;
	size	iLastIndex;

	iLastIndex = ARRAY_ELEMENT_NOT_FOUND;
	iIndex = mszPattern.Find('\n');
	if ((iIndex > 0) && (iIndex != ARRAY_ELEMENT_NOT_FOUND))
	{
		iExpected = iIndex;
		do
		{
			iLastIndex = iIndex;
			iIndex = mszPattern.Find(iIndex + 1, '\n');
			if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
			{
				if (iIndex - iLastIndex - 1!= iExpected)
				{
					return gcLogger.Error2(__METHOD__, " Expected every row to be a multiple of [", SizeToString(iExpected), "].", NULL);
				}
			}
		} while (iIndex != ARRAY_ELEMENT_NOT_FOUND);

		if (iLastIndex != mszPattern.Length() - 1)
		{
			return gcLogger.Error2(__METHOD__, " Expected every a final new line.", NULL);
		}
		return true;
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Expected at least one non-zero length line.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::IsNamed(size iType, char* szConstantName)
{
	return (mszConstantName == szConstantName) && (miType == iType);
}

