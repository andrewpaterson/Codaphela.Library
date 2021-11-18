/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "ErrorTypes.h"
#include "FileIO.h"
#include "PackFileNode.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileNode::Init(void)
{
	muiFilePos = 0;
	muiSize = 0;
	mbNameWritten = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileNode::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFileNode::Load(CFileReader* pcReader)
{
	ReturnOnFalse(pcReader->ReadInt(&muiFilePos));
	ReturnOnFalse(pcReader->ReadInt(&muiSize));
	mbNameWritten = TRUE;  //If we loaded it the name has to have been written.
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFileNode::Save(CFileWriter* pcWriter)
{
	ReturnOnFalse(pcWriter->WriteInt(muiFilePos));
	ReturnOnFalse(pcWriter->WriteInt(muiSize));
	mbNameWritten = TRUE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFileNode::FilePos(void)
{
	return muiFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFileNode::Size(void)
{
	return muiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFileNode::IsNameWritten(void)
{
	return mbNameWritten;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFileNode::IsInitialised(void)
{
	return muiFilePos != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileNode::SetFilePos(filePos iFilePos)
{
	muiFilePos = iFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFileNode::IncreaseSize(filePos iSize)
{
	muiSize += iSize;
}
