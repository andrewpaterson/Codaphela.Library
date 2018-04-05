#include "Chars.h"
#include "LogString.h"
#include "IndexTreeRecursor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::Init(CIndexTreeNode* pcRoot)
{
	macKey.Init(256);
	mpcCurrent = pcRoot;

	mszBadKey.Init();
	mszBadNode.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::Kill(void)
{
	mszBadNode.Kill();
	mszBadKey.Kill();

	macKey.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::Push(CIndexTreeNode *pcChild, char c)
{
	macKey.Push(c);
	mpcCurrent = pcChild;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::Pop(void)
{
	macKey.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::GenerateBad(void)
{
	BOOL	bHex;

	bHex = GenerateBadKey();
	GenerateBadNode(bHex);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeRecursor::GenerateBadKey(void)
{
	int				i;
	int				iKeyLength;
	int				iPrintable;
	unsigned char	c;
	float			fPrintable;
	CChars			sz;

	iKeyLength = macKey.NumElements();
	if (iKeyLength > 0)
	{
		iPrintable = StrPrintable(macKey.GetData());
		fPrintable = (float)iPrintable / (float)iKeyLength;

		if (fPrintable >= 0.9f)
		{
			mszBadKey.Append(macKey.GetData(), iKeyLength);
			return FALSE;
		}
		else
		{
			mszBadKey.Append("0x");

			sz.Init();

			for (i = 0; i < iKeyLength; i++)
			{
				c = macKey.GetValue(i);

				sz.Clear();
				sz.Append((int)c, 16);
				sz.RightAlign('0', 2);

				mszBadKey.Append(sz);
				if (i != iKeyLength-1)
				{
					mszBadKey.Append(' ');
				}
			}

			sz.Kill();
		}
		return TRUE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeRecursor::GenerateBadNode(BOOL bHex)
{
	mpcCurrent->Print(&mszBadNode, bHex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeRecursor::GetBadKey(void)
{
	return mszBadKey.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIndexTreeRecursor::GetBadNode(void)
{
	return mszBadNode.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNode* CIndexTreeRecursor::GetNode(void) {	return mpcCurrent; }

