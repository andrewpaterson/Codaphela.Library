/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "FileNode.h"
#include "PackFileReadArrary.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileReadArray::Init(void)
{
	maPtrs.Init(8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileReadArray::Kill(void)
{
	maPtrs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFileReadArray::GetReadPos(CPackFileNode* psPackFile)
{
	int							i;
	SPackFileReadPtrPosition*	psPtr;

	for (i = 0; i < maPtrs.NumElements(); i++)
	{
		psPtr = maPtrs.Get(i);
		if (psPtr->psPackFile == psPackFile)
		{
			return psPtr->iLastReadPos;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileReadArray::SetReadPos(CPackFileNode* psPackFile, filePos iLastReadPos)
{
	int							i;
	SPackFileReadPtrPosition*	psPtr;

	for (i = 0; i < maPtrs.NumElements(); i++)
	{
		psPtr = maPtrs.Get(i);
		if (psPtr->psPackFile == psPackFile)
		{
			psPtr->iLastReadPos = iLastReadPos;
			return;
		}
	}

	psPtr = maPtrs.Add();
	psPtr->psPackFile = psPackFile;
	psPtr->iLastReadPos = iLastReadPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileReadArray::BeginReadPos(CPackFileNode* psPackFile)
{
	SPackFileReadPtrPosition*	psPtr;

	psPtr = maPtrs.Add();
	psPtr->psPackFile = psPackFile;
	psPtr->iLastReadPos = psPackFile->FilePos();
}

