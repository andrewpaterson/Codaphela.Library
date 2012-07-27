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
#ifndef __INDEX_DISK_DIRECT_ACCESS_H__
#define __INDEX_DISK_DIRECT_ACCESS_H__
#include "DurableFile.h"
#include "IndexAccess.h"


class CIndexDiskDirectAccess : public CIndexAccess
{
public:
	void 			Init(CIndexDescriptorsFile* pcDescriptorsFile);
	void 			Kill(void);

	BOOL 			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL 			Set(CIndexedDataDescriptor* pcDescriptor);
	BOOL 			Remove(OIndex oi);
	long long int	Length(void);

	void 			Load(void);
	void 			Save(void);
	BOOL 			UpdateFile(void);

	BOOL			IsDisk(void);
};


#endif // __INDEX_DISK_DIRECT_ACCESS_H__

