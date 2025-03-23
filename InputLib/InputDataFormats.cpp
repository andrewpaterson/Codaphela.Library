/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputDataFormats.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormats::Init(void)
{
	mlcFormats.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDataFormats::Kill(void)
{
	mlcFormats.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDataFormat* CInputDataFormats::Add(char* szName)
{
	CInputDataFormat*	pcInputDataFormat;

	pcInputDataFormat = mlcFormats.Add();
	pcInputDataFormat->Init(szName);
	return pcInputDataFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDataFormat* CInputDataFormats::Add(void)
{
	CInputDataFormat*	pcInputDataFormat;

	pcInputDataFormat = mlcFormats.Add();
	pcInputDataFormat->Init();
	return pcInputDataFormat;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDataFormat* CInputDataFormats::Get(char* szName)
{
	CInputDataFormat*	pcDataFormat;
	SSetIterator		sIter;

	pcDataFormat = mlcFormats.StartIteration(&sIter);
	while (pcDataFormat)
	{
		if (pcDataFormat->IsCommon())
		{
			if (pcDataFormat->IsCommonName(szName))
			{
				return pcDataFormat;
			}
		}
		else
		{
			break;
		}
		pcDataFormat = mlcFormats.Iterate(&sIter);
	}
	return NULL;
}

