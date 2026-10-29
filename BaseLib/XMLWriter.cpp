/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include "TextFile.h"
#include "XMLWriter.h"


//////////////////////////////////////////////////////////////////////////
//
// Starts with the XML declaration.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Init(void)
{
	mszText.Init("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	maszOpen.Init();
	mbStartOpen = false;
	mbHasChildren = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Kill(void)
{
	maszOpen.Kill();
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Indent(void)
{
	mszText.Append(' ', maszOpen.NumElements() * 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Ends the open start tag; the element is getting children.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::CloseStart(void)
{
	if (mbStartOpen)
	{
		mszText.Append(">\n");
		mbStartOpen = false;
	}
	mbHasChildren = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Begin(const char* szName)
{
	if (maszOpen.NumElements() > 0)
	{
		CloseStart();
	}
	Indent();
	mszText.Append('<');
	mszText.Append(szName);
	maszOpen.Add(szName);
	mbStartOpen = true;
	mbHasChildren = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Closes the innermost element.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::End(void)
{
	if (maszOpen.NumElements() == 0)
	{
		return;
	}

	if (mbStartOpen)
	{
		mszText.Append("/>\n");
		mbStartOpen = false;
		maszOpen.RemoveTail();
	}
	else
	{
		mszText.Append(' ', (maszOpen.NumElements() - 1) * 2);
		mszText.Append("</");
		mszText.Append(maszOpen.Tail()->Text());
		mszText.Append(">\n");
		maszOpen.RemoveTail();
	}
	//The element closed was a child of the one now innermost.
	mbHasChildren = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, const char* szValue)
{
	if (!mbStartOpen)
	{
		return;
	}
	mszText.Append(' ');
	mszText.Append(szName);
	mszText.Append("=\"");
	AppendEscaped(szValue, true);
	mszText.Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, int32 iValue)
{
	char	sz[32];

	snprintf(sz, sizeof(sz), "%d", (int)iValue);
	Attribute(szName, sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, int64 iValue)
{
	char	sz[32];

	snprintf(sz, sizeof(sz), "%lld", (long long)iValue);
	Attribute(szName, sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, bool bValue)
{
	Attribute(szName, bValue ? "true" : "false");
}


//////////////////////////////////////////////////////////////////////////
//
// Nine significant digits read back to the same float.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, float32 fValue)
{
	char	sz[48];

	snprintf(sz, sizeof(sz), "%.9g", (double)fValue);
	Attribute(szName, sz);
}


//////////////////////////////////////////////////////////////////////////
//
// Seventeen significant digits read back to the same double.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Attribute(const char* szName, float64 fValue)
{
	char	sz[48];

	snprintf(sz, sizeof(sz), "%.17g", fValue);
	Attribute(szName, sz);
}


//////////////////////////////////////////////////////////////////////////
//
// Text on a line of its own inside the innermost element.
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::Text(const char* szText)
{
	CloseStart();
	Indent();
	AppendEscaped(szText, false);
	mszText.Append('\n');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLWriter::AppendEscaped(const char* szText, bool bAttribute)
{
	const char*	pc;

	for (pc = szText; *pc != '\0'; pc++)
	{
		switch (*pc)
		{
		case '<':
			mszText.Append("&lt;");
			break;
		case '>':
			mszText.Append("&gt;");
			break;
		case '&':
			mszText.Append("&amp;");
			break;
		case '"':
			if (bAttribute)
			{
				mszText.Append("&quot;");
			}
			else
			{
				mszText.Append('"');
			}
			break;
		case '\'':
			if (bAttribute)
			{
				mszText.Append("&apos;");
			}
			else
			{
				mszText.Append('\'');
			}
			break;
		default:
			mszText.Append(*pc);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every element begun has ended.
//
//////////////////////////////////////////////////////////////////////////
bool CXMLWriter::IsComplete(void)
{
	return maszOpen.NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CXMLWriter::GetText(void)
{
	return mszText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLWriter::Write(const char* szFilename)
{
	CTextFile	cFile;
	bool		bResult;

	cFile.Init(mszText.Text());
	bResult = cFile.Write(szFilename);
	cFile.Kill();
	return bResult;
}

