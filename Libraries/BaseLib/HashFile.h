/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __HASH_FILE_H__
#define __HASH_FILE_H__
#include "MD5.h"
#include "AbstractFile.h"


//A file that hashes what goes through it (CMD5HashFile), as CChunkFile uses one.  CChunkFile only calls it through
//this interface, so a program that only reads or writes chunk files without hashes (CChunkFile::InitUnhashed) doesn't
//link the hashing.
class CHashFile : public CAbstractFile
{
public:
	virtual void	StopHashing(void) = 0;
	virtual void	StartHashing(void) = 0;
	virtual void	ResetHash(void) = 0;
	virtual void	CopyContextToDest(SMD5Context* psDestContext) = 0;
	virtual void	CopyContextFromSource(SMD5Context* psSourceContext) = 0;
	virtual void	CopyDigestToDest(void* pvDest) = 0;
};


#endif // __HASH_FILE_H__
