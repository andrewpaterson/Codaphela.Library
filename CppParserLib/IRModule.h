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
#ifndef __I_R_MODULE_H__
#define __I_R_MODULE_H__
#include "IRFunction.h"


enum EIRDataKind
{
	IRDK_Value,			//An integer or float of sType (ulliValue the bits).
	IRDK_Symbol,		//A pointer of sType to symbol ulliValue + lliOffset.
	IRDK_Bytes,			//uiLength bytes from uiStart in the global's bytes.
	IRDK_Zero,			//ulliValue zero bytes.
};


struct SIRDataItem
{
	uint8	uiKind;
	SIRType	sType;
	uint64	ulliValue;
	int64	lliOffset;
	size	uiStart;
	size	uiLength;
};


typedef CArrayTemplate<SIRDataItem>	CArrayIRDataItems;
typedef CArrayTemplate<uint8>		CArrayIRBytes;


class CIRModule;


//A global's initial value is its items, one after another.
class CIRGlobal
{
public:
	uint32				muiSymbol;
	uint32				muiAlign;
	uint32				muiAttributes;
	CArrayIRDataItems	masItems;
	CArrayIRBytes		macBytes;

public:
	void	Init(uint32 uiSymbol, uint32 uiAlign, uint32 uiAttributes);
	void	Kill(void);

	void	AddValue(SIRType sType, uint64 ulliBits);
	void	AddSymbol(SIRType sType, uint32 uiSymbol, int64 lliOffset);
	void	AddBytes(const uint8* pucBytes, size uiLength);
	void	AddZero(size uiBytes);
	size	Size(CIRModule* pcModule);

protected:
	SIRDataItem*	AddItem(EIRDataKind eKind);
};


//What a symbol is defined as here; both NULL for a symbol from another unit.
struct SIRSymbolDefinition
{
	CIRFunction*	pcFunction;
	CIRGlobal*		pcGlobal;
};


typedef CArrayTemplate<SIRSymbolDefinition>	CArrayIRSymbolDefinitions;


typedef CArrayTemplatePtr<CIRFunction>	CArrayIRFunctions;
typedef CArrayTemplatePtr<CIRGlobal>	CArrayIRGlobals;
typedef CArrayTemplate<uint8>			CArrayIRSpaceSizes;


//One translation unit: address spaces, the symbols the functions and globals use (defined here or not), the
//functions and the globals.  A target's opcodes (machine IR) are numbered from IRO_NumGeneric.
class CIRModule
{
public:
	CArrayChars					maszSpaces;
	CArrayIRSpaceSizes			maucSpaceSizes;

	CArrayChars					maszSymbols;
	CMapStringInt				mmSymbols;
	CArrayIRSymbolDefinitions	masDefinitions;

	CArrayIRFunctions			mapcFunctions;
	CArrayIRGlobals				mapcGlobals;
	CArrayIRGlobals				mapcKnown;			//Read only data defined elsewhere (the generator's vtables): never written out.

	SIROpcode*					mpasTargetOpcodes;
	uint16						muiNumTargetOpcodes;

public:
	void			Init(void);
	void			Kill(void);

	uint8			AddSpace(const char* szName, int iPointerBytes);
	int				FindSpace(const char* szName);
	size			NumSpaces(void);
	char*			GetSpaceName(uint8 uiSpace);
	int				GetPointerSize(uint8 uiSpace);
	int				GetSize(SIRType sType);

	uint32			AddSymbol(const char* szName);
	int				FindSymbol(const char* szName);
	char*			GetSymbolName(uint32 uiSymbol);
	SIRReference	Symbol(const char* szName);

	CIRFunction*	AddFunction(const char* szName, SIRType sReturnType, uint32 uiAttributes = 0);
	CIRFunction*	FindFunction(uint32 uiSymbol);
	void			RemoveFunction(CIRFunction* pcFunction);
	size			NumFunctions(void);
	CIRFunction*	GetFunction(size uiIndex);

	CIRGlobal*		AddGlobal(const char* szName, uint32 uiAlign = 1, uint32 uiAttributes = 0);
	CIRGlobal*		FindGlobal(uint32 uiSymbol);
	size			NumGlobals(void);
	CIRGlobal*		GetGlobal(size uiIndex);
	bool			IsDefined(uint32 uiSymbol);

	CIRGlobal*		AddKnownData(const char* szName, uint32 uiAlign = 1);
	CIRGlobal*		FindConstantData(uint32 uiSymbol);

	void			SetTargetOpcodes(SIROpcode* pasOpcodes, uint16 uiNumOpcodes);
	SIROpcode*		GetOpcode(uint16 uiOpcode);
	uint16			FindOpcode(const char* szName);
};


#endif // __I_R_MODULE_H__
