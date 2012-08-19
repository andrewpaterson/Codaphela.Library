/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "DependentObjectWriter.h"
#include "ObjectGraphWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphWriter::Init(CObjectWriter* pcWriter)
{
	mpcWriter = pcWriter;
	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphWriter::Kill(void)
{
	mcDependentObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphWriter::Write(CBaseObject* pcObject)
{
	CBaseObject*	pcUnwritten;

	AddDependent(pcObject);

	for (;;)
	{
		pcUnwritten = mcDependentObjects.GetUnwritten();
		if (pcUnwritten)
		{
			ReturnOnFalse(WriteUnwritten(pcUnwritten));
		}
		else
		{
			break;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphWriter::WriteUnwritten(CBaseObject* pcObject)
{
	CDependentObjectWriter	cWriter;
	BOOL					bResult;
	CChars					szName;
	OIndex					oi;

	cWriter.Init(this, pcObject);
	bResult = pcObject->Save(&cWriter);
	if (!bResult)
	{
		return FALSE;
	}
	
	oi = pcObject->GetOI();
	
	if (pcObject->IsNamed())
	{
		szName.Init(pcObject->GetName());
	}
	else
	{
		szName.Init("Unnamed_");
		szName.Append(oi);
	}

	ReturnOnFalse(mpcWriter->Write(oi, szName.Text(), cWriter.GetData(), cWriter.GetLength()));

	MarkWritten(pcObject);
	szName.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphWriter::AddDependent(CBaseObject* pcObject)
{
	mcDependentObjects.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphWriter::MarkWritten(CBaseObject* pcObject)
{
	mcDependentObjects.Mark(pcObject);
}
