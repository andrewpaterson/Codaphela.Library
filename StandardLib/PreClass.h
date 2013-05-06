/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __PRE_CLASS_H__
#define __PRE_CLASS_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ErrorHandler.h"
#include "BaseLib/EnumeratorBlock.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/AdditionalTypes.h"
#include "BaseLib/FreeListBlock.h"
#include "BaseLib/ChunkFile.h"
#include "CoreLib/FunctionCaller.h"


//Struct Flags
#define SF_PRIMITIVE	0x0001	//Is one of the standard primitive types.
#define SF_SIMPLE		0x0002	//Type contains no pointers.  (can be deep copied with memcpy)
#define SF_INCOMPLETE	0x0004	//Requires addition information before it can be used.
#define SF_INTERNAL		0x0010	//Any class with this bit set has been generated internally.
#define SF_ZERO_INIT	0x0040	//Type is initialised with all zeros.
#define SF_INSTANCES	0x0080  //Maintains a list of instances (Only valid with SF_SMART).
#define SF_CLASS		0x0100	//This class is a place holder if this is not set.
#define SF_BASIC		0x0200	//One of the basic C++ types.



class CClassField
{
public:
	int		iType;	//The class storage type ID for this member.
	int		iMemberOffset;	//Offset in bytes from the start of the class.
	CChars	szName;

	void 	Init(int iMemberOffset, int iType, char* szMemberName);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


typedef CArrayTemplate<CClassField>	SClassFieldArray;


enum EMethodConvention
{
	MC_Unknown,
	MC_CCall,
	MC_ThisCall,
	MC_ThisMultiCall,
	MC_StdCall,  //Ignored
};


class CClassMethodDesc
{
public:
	int					miArgumentType;
	int					miReturnType;
	EMethodConvention	meConvention;  //Calling convention: stdcall, cdecl or thiscall.  i386/Amd64
	CChars				mszName;
	int					miGeneric;  //-1 if there is no generic type.

	BOOL 	Init(int iArgumentType, int iReturnType, EMethodConvention eConvention, char* szName, int iGeneric);
	void 	Copy(CClassMethodDesc* psSource);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};

class CClassStorage;
class CClassMethod : public CClassMethodDesc
{
public:
	SFunctionPointer	mpvFunction;

	BOOL 	Init(int iGeneric, EMethodConvention eConvention, SFunctionPointer pvFunction, CClassStorage* pcClassStorage);
	BOOL 	Init(int iArgumentType, int iReturnType, EMethodConvention eConvention, SFunctionPointer pvFunction, char* szName);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);
};


typedef CArrayTemplate<CClassMethod>	CClassMethodArray;


class CAbstractClass;
typedef CArrayTemplate<CAbstractClass*>		CArrayClassPtr;


#endif // __PRE_CLASS_H__

