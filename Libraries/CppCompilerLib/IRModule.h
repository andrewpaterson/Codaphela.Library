/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_MODULE_H__
#define __I_R_MODULE_H__
#include "IRFunction.h"
#include "BaseLib/HashMapString.h"
#include "CPPCompileStatistics.h"


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


#define IR_DEFAULT_POINTER_BYTES	4		//The default space's pointers, for a module that isn't a target's (see CCPPTarget::GetPointerBytes).


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


//What is known of a symbol: a set of these (SIRSymbolDefinition::uiAttributes).
#define IRSA_Restrict		0x01	//A pointer variable declared restrict: what is reached through its value is reached no other way (SetRestrict).
#define IRSA_Local			0x02	//The unit's own (a string, a static variable or function) even if nothing here defines it (SetLocal).
#define IRSA_RegisterCall	0x04	//A function a back end may pass arguments to in registers (SetRegisterCall).
#define IRSA_NoWrites		0x08	//A function that writes no memory its callers can see (SetNoWrites).
#define IRSA_NoThrow		0x10	//A function that is never left with an exception (SetNoThrow).

//Those that go with a function into another unit's module (CopySymbolAttributes: the imports, the inliner), being
//true of the symbol wherever it is named.  Not Local: another unit's symbol of the name is another thing.  Not
//NoWrites: it is worked out for a unit's own functions, and was never carried (OptimiserPlan.txt stage 3).
#define IRSA_Carried		(IRSA_Restrict | IRSA_RegisterCall | IRSA_NoThrow)


//The attributes written as lines of their own before the globals ("restrict @name"), in the order they are printed.
//Local is not one: it is written with the symbol's global or function.
struct SIRSymbolAttributeName
{
	uint8		uiAttribute;
	const char*	szName;
};


#define IRSA_NumWritten		4
extern SIRSymbolAttributeName	gasIRSymbolAttributeNames[IRSA_NumWritten];


//What a symbol is defined as here (both NULL for a symbol from another unit), and what is known of it.
struct SIRSymbolDefinition
{
	CIRFunction*	pcFunction;
	CIRGlobal*		pcGlobal;
	uint8			uiAttributes;	//IRSA_.
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
	CHashMapString<int32>		mmSymbols;
	CArrayIRSymbolDefinitions	masDefinitions;
	size						muiNumRestrict;

	CArrayIRFunctions			mapcFunctions;
	CArrayIRGlobals				mapcGlobals;
	CArrayIRGlobals				mapcKnown;			//Read only data defined elsewhere (the generator's vtables): never written out.

	SIROpcode*					mpasTargetOpcodes;
	uint16						muiNumTargetOpcodes;

	uint32						muiSource;			//The source token + 1 instructions added to the module's blocks now are for (0: none, as the optimiser's).
	CCPPCompileStatistics*		mpcStatistics;		//What compiling the unit's functions is counted and timed in (SetStatistics), or NULL.
	bool						mbDebugMarkers;		//Promoting a slot a debugger is told of leaves debug value markers (CCPPCompileOptions).

public:
	void			Init(void);
	void			Kill(void);
	void			SetStatistics(CCPPCompileStatistics* pcStatistics);
	void			Count(ECPPCompileCount eCount, uint64 uiCount = 1);
	void			Begin(ECPPCompilePhase ePhase);
	void			End(ECPPCompilePhase ePhase);

	uint8			AddSpace(const char* szName, int iPointerBytes);
	uint8			AddDefaultSpace(int iPointerBytes = IR_DEFAULT_POINTER_BYTES);
	int				FindSpace(const char* szName);
	size			NumSpaces(void) const;
	char*			GetSpaceName(uint8 uiSpace);
	int				GetPointerSize(uint8 uiSpace);
	int				GetSize(SIRType sType);

	uint32			AddSymbol(const char* szName);
	int				FindSymbol(const char* szName);
	char*			GetSymbolName(uint32 uiSymbol);
	SIRReference	Symbol(const char* szName);
	void			SetSymbolAttributes(uint32 uiSymbol, uint8 uiAttributes, bool bSet = true);
	uint8			GetSymbolAttributes(uint32 uiSymbol);
	void			CopySymbolAttributes(uint32 uiSymbol, CIRModule* pcFrom, uint32 uiFromSymbol, uint8 uiAttributes);
	void			SetRestrict(uint32 uiSymbol);
	bool			IsRestrict(uint32 uiSymbol);
	bool			HasRestrict(void) const;
	void			SetLocal(uint32 uiSymbol);
	bool			IsLocal(uint32 uiSymbol);
	void			SetRegisterCall(uint32 uiSymbol);
	bool			IsRegisterCall(uint32 uiSymbol);
	void			SetNoWrites(uint32 uiSymbol, bool bNoWrites);
	bool			IsNoWrites(uint32 uiSymbol);
	void			SetNoThrow(uint32 uiSymbol, bool bNoThrow);
	bool			IsNoThrow(uint32 uiSymbol);

	CIRFunction*	AddFunction(const char* szName, SIRType sReturnType, uint32 uiAttributes = 0);
	CIRFunction*	FindFunction(uint32 uiSymbol);
	CIRFunction*	CopyFunction(CIRFunction* pcFunction, const char* szName);
	void			RemoveFunction(CIRFunction* pcFunction);
	size			NumFunctions(void) const;
	CIRFunction*	GetFunction(size uiIndex);

	CIRGlobal*		AddGlobal(const char* szName, uint32 uiAlign = 1, uint32 uiAttributes = 0);
	CIRGlobal*		FindGlobal(uint32 uiSymbol);
	size			NumGlobals(void) const;
	CIRGlobal*		GetGlobal(size uiIndex);
	bool			IsDefined(uint32 uiSymbol);

	CIRGlobal*		AddKnownData(const char* szName, uint32 uiAlign = 1);
	CIRGlobal*		FindConstantData(uint32 uiSymbol);

	void			SetTargetOpcodes(SIROpcode* pasOpcodes, uint16 uiNumOpcodes);
	SIROpcode*		GetOpcode(uint16 uiOpcode);
	uint16			FindOpcode(const char* szName);
};


#endif // __I_R_MODULE_H__
