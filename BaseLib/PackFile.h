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
#ifndef __PACK_FILE_H__
#define __PACK_FILE_H__
#include "AbstractFile.h"
#include "FileNode.h"


class CPackFiles;
class CPackFile : public CAbstractFile
{
public:
	CPackFiles*		mpcPackFiles;
	CPackFileNode*	mpsPackFileNode;
	filePos			miPos;
	EFileMode		meMode;

	void		Init(CPackFiles* pcPackFiles, CPackFileNode* psPackFileNode);
	void		Kill(void);

	bool		Open(EFileMode eFileMode);
	bool		Close(void);
	filePos		Read(void* pvBuffer, filePos iSize, filePos iCount);
	bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	filePos		Write(const void* pvBuffer, filePos iSize, filePos iCount);
	filePos		Tell(void);
	bool		IsOpen(void);
	filePos		Size(void);
	bool		Truncate(filePos iSize);
	bool		Flush(void);
	bool		Delete(void);
	char*		GetFilename(void);
};


//Helper function to make creating basic files easier.
CPackFile* PackFile(CPackFiles* pcPackFiles, CPackFileNode* psPackFile);


#endif // !__PACK_FILE_H__

