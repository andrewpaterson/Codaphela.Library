/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/ArrayString.h"
#include "BaseLib/Logger.h"
#include "InputCategories.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputCategories::Init(CInputDevices* pcDevices)
{
	mpcDevices = pcDevices;
	mlcCategories.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputCategories::Kill(void)
{
	mlcCategories.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInputCategories::AddCategory(char* szName)
{
	CInputCategory*		pcInputCategory;

	pcInputCategory = mlcCategories.Add();
	pcInputCategory->Init(szName, mpcDevices);
	return pcInputCategory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInputCategories::GetCategory(char* szName)
{
	CInputCategory*		pcInputCategory;
	SSetIterator		sIter;

	pcInputCategory = mlcCategories.StartIteration(&sIter);
	while (pcInputCategory)
	{
		if (pcInputCategory->Is(szName))
		{
			return pcInputCategory;
		}
		pcInputCategory = mlcCategories.Iterate(&sIter);
	}
	return NULL;
}
