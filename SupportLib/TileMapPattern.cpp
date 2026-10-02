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
	miBX = ARRAY_ELEMENT_NOT_FOUND;
	miBY = ARRAY_ELEMENT_NOT_FOUND;
	miWidth = ARRAY_ELEMENT_NOT_FOUND;
	miHeight = ARRAY_ELEMENT_NOT_FOUND;
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
	bool	bFound;

	mszPattern.Replace(" ", "");

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
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Expected at least one non-zero length line.", NULL);
	}

	miWidth = mszPattern.Find('\n');
	miHeight = mszPattern.Length() / (GetWidth() + 1);

	bFound = FindB(&miBX, &miBY);
	if (!bFound)
	{
		return gcLogger.Error2(__METHOD__, " Expected exactly one 'B' character.", NULL);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::IsNamed(size iType, char* szConstantName)
{
	return (mszConstantName == szConstantName) && (miType == iType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::IsSource(char* szConstantSource)
{
	return mszConstantSource == szConstantSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileMapPattern::GetType(void)
{
	return miType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTileMapPattern::GetConstantName(void)
{
	return mszConstantName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileMapPattern::GetWidth(void)
{
	return miWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileMapPattern::GetHeight(void)
{
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CTileMapPattern::GetChar(size x, size y)
{
	return mszPattern.GetChar(x + y * (GetWidth() + 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapPattern::FindB(size* px, size* py)
{
	size	iIndex;
	size	iWidth;

	iIndex = mszPattern.Find('B');
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		*px = ARRAY_ELEMENT_NOT_FOUND;
		*py = ARRAY_ELEMENT_NOT_FOUND;
		return false;
	}


	iWidth = GetWidth();
	*px = iIndex % (iWidth + 1);
	*py = iIndex / (iWidth + 1);

	iIndex = mszPattern.Find(iIndex + 1, 'B');
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileMapPattern::GetBX(void)
{
	return miBX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTileMapPattern::GetBY(void)
{
	return miBY;
}

