/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_FUNCTION_H__
#define __I_R_FUNCTION_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringInt.h"
#include "BaseLib/MapStringTemplate.h"
#include "IRBlock.h"


//Function and global attributes.
#define IRA_Local		0x01		//The unit's label, not .global.
#define IRA_Weak		0x02
#define IRA_Constant	0x04		//Globals: never written.
#define IRA_NoReturn	0x08		//Functions.
#define IRA_Inline		0x10		//Functions: may be inlined.
#define IRA_ThisCall	0x20		//Functions: the first parameter is a member function's this (in X on the 65C64).
#define IRA_Variadic	0x40		//Functions; written as ... after the parameters.
#define IRA_NumWordAttributes	6


struct SIRAttributeName
{
	uint32		uiAttribute;
	const char*	szName;
};


extern SIRAttributeName	gasIRAttributeNames[IRA_NumWordAttributes];


struct SIRTemporary
{
	SIRType	sType;
	uint32	uiName;		//1 + the index of its name, 0 if it has none.
};


//Integers are kept truncated to their width (zero extended); floats are their bits.
struct SIRConstant
{
	SIRType	sType;
	uint64	ulliBits;
};


typedef CArrayTemplate<SIRTemporary>		CArrayIRTemporaries;
typedef CArrayTemplate<SIRConstant>			CArrayIRConstants;
typedef CArrayTemplate<SIRReference>		CArrayIRReferences;
typedef CMapStringTemplate<CIRBlock*>		CMapIRBlocks;


//A function: its blocks (the first is the entry), temporaries, constants and parameters.  Temporaries and blocks
//may have names (unique in the function; a name that is all digits is not kept); the others are numbered when
//printed.  The name maps are only made when the first name is given.
class CIRFunction
{
public:
	uint32				muiSymbol;
	SIRType				msReturnType;
	uint32				muiAttributes;
	CArrayIRReferences	masParameters;

	CArrayIRTemporaries	masTemporaries;
	CArrayChars			maszTemporaryNames;
	CMapStringInt*		mpmTemporaryNames;

	CArrayIRConstants	masConstants;
	CArrayTemplate<uint32>	mauiConstantHash;		//1 + a constant's index, 0 for an empty entry.

	CArrayIRBlocks		mapcBlocks;
	CMapIRBlocks*		mpmBlockNames;

public:
	void			Init(uint32 uiSymbol, SIRType sReturnType, uint32 uiAttributes = 0);
	void			Kill(void);

	SIRReference	AddTemporary(SIRType sType, const char* szName = NULL);
	SIRReference	AddParameter(SIRType sType, const char* szName = NULL);
	SIRReference	FindTemporary(const char* szName);
	size			NumTemporaries(void);
	SIRType			GetTemporaryType(SIRReference sTemporary);
	void			SetTemporaryType(SIRReference sTemporary, SIRType sType);
	char*			GetTemporaryName(SIRReference sTemporary);

	SIRReference	AddConstant(SIRType sType, uint64 ulliBits);
	SIRReference	AddInteger(SIRType sType, int64 lliValue);
	SIRReference	AddFloat(float fValue);
	SIRReference	AddDouble(double fValue);
	SIRConstant*	GetConstant(SIRReference sConstant);
	int64			GetSigned(SIRReference sConstant);

	SIRType			TypeOf(SIRReference sReference);

	CIRBlock*		AddBlock(const char* szName = NULL);
	CIRBlock*		FindBlock(const char* szName);
	CIRBlock*		GetEntry(void);
	size			NumBlocks(void);
	CIRBlock*		GetBlock(size uiIndex);
	void			SetBlockOrder(CArrayIRBlocks* papcBlocks);
	void			RemoveBlock(CIRBlock* pcBlock);
	void			Renumber(void);
	void			ComputePredecessors(void);
	void			CountUses(CArrayTemplate<uint32>* pauiUses);

	SIRReference	Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, SIRReference sArgument1, const char* szName = NULL);
	SIRReference	Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, const char* szName = NULL);
	SIRReference	Load(CIRBlock* pcBlock, SIRType sType, SIRReference sAddress, bool bVolatile = false, const char* szName = NULL);
	void			Store(CIRBlock* pcBlock, SIRType sType, SIRReference sValue, SIRReference sAddress, bool bVolatile = false);
	SIRReference	Slot(CIRBlock* pcBlock, SIRType sPointerType, uint32 uiSize, uint32 uiAlign, const char* szName = NULL);
	void			Argument(CIRBlock* pcBlock, SIRType sType, SIRReference sValue);
	SIRReference	Call(CIRBlock* pcBlock, SIRType sType, SIRReference sCallee, int iNamed = -1, bool bThisCall = false, const char* szName = NULL);
	SIRInstruction*	AddCall(CIRBlock* pcBlock, SIRType sType, SIRReference sResult, SIRReference sCallee, int iNamed, bool bThisCall = false);

protected:
	bool			IsName(const char* szName);
	void			UniqueName(const char* szName, CMapStringBlock* pmNames, CChars* pszDest);
	uint32			HashConstant(SIRType sType, uint64 ulliBits);
	void			GrowConstantHash(void);
};


#endif // __I_R_FUNCTION_H__
