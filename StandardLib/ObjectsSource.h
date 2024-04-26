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
#ifndef __OBJECTS_SOURCE_H__
#define __OBJECTS_SOURCE_H__
#include "BaseLib/Files.h"
#include "ArrayUnknown.h"
#include "MapStringUnknown.h"
#include "Unknown.h"


class CObjectSource;
class CObjectsSource : public CUnknown
{
CONSTRUCTABLE(CObjectsSource);
protected:
	CFiles				mcFiles;
	CArrayUnknown		macSources;
	CMapStringUnknown	mmszpcConverters;

public:
								void			Init(void);
								void			Kill(void);
	template<class TConverter>	TConverter*		AddConverter(void);
	template<class TConverter>	CObjectSource*	AddSource(CAbstractFile* pcFile, char* szFilename);
								CObjectSource*	GetSource(char* szObjectName);

};

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TConverter>
CObjectSource* CObjectsSource::AddSource(CAbstractFile* pcFile, char* szFilename)
{
	TConverter*		pcConveter;
	CObjectSource*	pcSource;

	pcConveter = AddConverter<TConverter>();
	pcSource = pcConveter->CreateSource(pcFile, szFilename);
	if (pcSource)
	{
		macSources.Add(pcSource);
		return pcSource;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class TConverter>
TConverter* CObjectsSource::AddConverter(void)
{
	TConverter*	pcConveter;
	TConverter	t;
	const char*	szClassName;
	CUnknown*	pcExistingConverter;

	szClassName = t.ClassName();
	pcExistingConverter = mmszpcConverters.Get((char*)szClassName);
	if (!pcExistingConverter)
	{
		pcConveter = gcUnknowns.Add<TConverter>();
		pcConveter->Init();

		mmszpcConverters.Put((char*)szClassName, pcConveter);
		return pcConveter;
	}
	else
	{
		return (TConverter*)pcExistingConverter;
	}

}

#endif // __OBJECTS_SOURCE_H__

