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
#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "DurableFile.h"


//This class contains a file with every data descriptor in it - regardless of the size of the data it describes.  
//The actual data is contained in files in the CIndexedDataFiles class.

class CIndexedDataDescriptor;
class CDurableFileController;
class CIndexDescriptorsFile
{
protected:
	CDurableFile				mcIndexedDescriptorFile;
	
public:
	CDurableFileController*		mpcDurableFileControl;

	void			Init(CDurableFileController* pcDurableFileControl, char* szFileName, char* szRewriteName);
	void			Kill(void);
	BOOL			Close(void);
	filePos			NumDescriptors(void);
	filePos			Read(CIndexedDataDescriptor* pcDescriptor, int iPosition, int iNum = 1);
	filePos			Write(CIndexedDataDescriptor* pcDescriptor, int iPosition, int iNum = 1);
	CDurableFile*	GetDurableFile(void);
	BOOL			Delete(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

