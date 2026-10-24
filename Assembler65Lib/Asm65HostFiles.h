/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_HOST_FILES_H__
#define __ASM65_HOST_FILES_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/DiskFile.h"


//The host file device's registers at 00FF:0200 (CAsm65Memory::SetHostFiles): four 32 bit arguments, a 64 bit
//result and the command, which runs when it is written.  Libc65's host.cpp is the 65C64's side.
#define ASM65_HOST_BASE				0x00FF0200
#define ASM65_HOST_ARGUMENT_0		0x00
#define ASM65_HOST_ARGUMENT_1		0x04
#define ASM65_HOST_ARGUMENT_2		0x08
#define ASM65_HOST_ARGUMENT_3		0x0C
#define ASM65_HOST_RESULT			0x10
#define ASM65_HOST_RESULT_HIGH		0x14
#define ASM65_HOST_COMMAND			0x18
#define ASM65_HOST_SIZE				0x1C

//Commands.  Paths and buffers are addresses in the 65C64's memory; paths end with a zero.
#define ASM65_HOST_OPEN				1		//Path, mode (HOST_READ ...): the handle, 0 if it failed.
#define ASM65_HOST_CLOSE			2		//Handle: 1 if it closed.
#define ASM65_HOST_READ				3		//Handle, buffer, bytes: the bytes read.
#define ASM65_HOST_WRITE			4		//Handle, buffer, bytes: the bytes written.
#define ASM65_HOST_SEEK				5		//Handle, offset low, offset high, origin: 1 if it moved.
#define ASM65_HOST_TELL				6		//Handle: the position, -1 if it failed.
#define ASM65_HOST_FILE_LENGTH		7		//Handle: the file's size.
#define ASM65_HOST_TRUNCATE			8		//Handle, size low, size high: 1 if it worked.
#define ASM65_HOST_FLUSH			9		//Handle: 1 if it worked.
#define ASM65_HOST_DELETE			10		//Path: 1 if it is deleted (or did not exist).
#define ASM65_HOST_EXISTS			11		//Path: 1 if a file or directory is there.
#define ASM65_HOST_SIZE_OF_FILE		12		//Path: its size, -1 for a directory or nothing.
#define ASM65_HOST_MAKE_DIRECTORY	13		//Path: 1 if it was made.
#define ASM65_HOST_REMOVE_DIRECTORY	14		//Path: 1 if it and everything in it was removed.
#define ASM65_HOST_IS_DIRECTORY		15		//Path: 1 if it is a directory.
#define ASM65_HOST_TOUCH			16		//Path: 1 if it was created or emptied.
#define ASM65_HOST_CURRENT_DIRECTORY	17	//Buffer, size: the length written.
#define ASM65_HOST_FIND_FILE		18		//Directory, index, buffer, size: 0 none, 1 file, 2 directory.
#define ASM65_HOST_RENAME			19		//Old path, new path: 1 if it worked.

//HostOpen's modes.
#define ASM65_HOST_MODE_READ		0x01
#define ASM65_HOST_MODE_WRITE		0x02
#define ASM65_HOST_MODE_CREATE		0x04
#define ASM65_HOST_MODE_TRUNCATE	0x08
#define ASM65_HOST_MODE_APPEND		0x10

#define ASM65_HOST_MAX_PATH			4096


class CAsm65Memory;


//An open host file.
struct SAsm65HostFile
{
	CDiskFile	cFile;
	bool		bAppend;
};


typedef CArrayTemplatePtr<SAsm65HostFile>	CArrayAsm65HostFiles;


//The simulator's files, used by a 65C64 program through the host file device.  The paths are the host's, relative
//to the host's current directory or to SetDirectory's (which is then the program's current directory).  Open files
//are closed by Kill.
class CAsm65HostFiles
{
protected:
	CArrayAsm65HostFiles	mapsFiles;			//By handle - 1, NULL when the handle is free.
	CChars					mszListedDirectory;
	CChars					mszDirectory;		//Relative paths are in it, empty for the host's current directory.
	CArrayChars				maszListed;			//The listed directory's entries, a 'D' or 'F' then the name.

public:
	void			Init(void);
	void			Kill(void);
	void			SetDirectory(const char* szDirectory);

	uint64			Command(CAsm65Memory* pcMemory, uint8 uiCommand, uint32* auiArguments);

protected:
	void			ReadString(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* psz);
	void			ReadPath(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* psz);
	uint32			WriteString(CAsm65Memory* pcMemory, uint32 uiAddress, uint32 uiSize, const char* sz);
	SAsm65HostFile*	GetFile(uint32 uiHandle);
	uint32			Open(const char* szPath, uint32 uiMode);
	bool			Close(uint32 uiHandle);
	uint32			FindFile(const char* szDirectory, uint32 uiIndex, CChars* pszName);
	void			ListDirectory(const char* szDirectory);
};


#endif // __ASM65_HOST_FILES_H__

