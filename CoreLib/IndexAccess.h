/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INDEX_ACCESS_H__
#define __INDEX_ACCESS_H__
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"
#include "DurableFileController.h"


class CIndexDescriptorsFile;
class CIndexAccess
{
protected:
	CIndexDescriptorsFile*	mpcDescriptorsFile;

public:
	
			void	Init(CIndexDescriptorsFile* pcDescriptorsFile);
	virtual void 	Kill(void) =0;

	virtual BOOL 	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi) =0;
	virtual	BOOL 	Set(CIndexedDataDescriptor* pcDescriptor) =0;
	virtual BOOL 	Remove(OIndex oi) =0;
	virtual long long int Length(void) =0;

	virtual void 	Load(void) =0;
	virtual void 	Save(void) =0;
	virtual BOOL 	UpdateFile(void) =0;

	virtual BOOL 	IsHuge(void);
	virtual BOOL 	IsMemory(void);
	virtual BOOL 	IsDisk(void);

			void 	Begin(void);
			BOOL	HasBegun(void);
};


#endif // __INDEX_ACCESS_H__

