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
#ifndef __I_R_FUNCTION_H__
#define __I_R_FUNCTION_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringInt.h"
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/HashSetTemplate.h"
#include "IRBlock.h"


//Function and global attributes.
#define IRA_Local		0x01		//The unit's label, not .global.
#define IRA_Weak		0x02
#define IRA_Constant	0x04		//Globals: never written.
#define IRA_NoReturn	0x08		//Functions.
#define IRA_Inline		0x10		//Functions: may be inlined.
#define IRA_ThisCall	0x20		//Functions: the first parameter is a member function's this (in X on the 65C64).
#define IRA_Variadic	0x40		//Functions; written as ... after the parameters.
#define IRA_NoInline	0x80		//Functions: never inlined (__attribute__((noinline))).
#define IRA_NumWordAttributes	7


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
//A constant's HASH: its type's kind and space and its bits.
struct SHashOfIRConstant
{
	static uint32 Hash(const SIRConstant* psConstant)
	{
		uint64	ulliHash;

		ulliHash = (psConstant->ulliBits ^ ((uint64)psConstant->sType.uiKind << 56) ^ ((uint64)psConstant->sType.uiSpace << 48)) * 0x9E3779B97F4A7C15ull;
		return (uint32)(ulliHash >> 32);
	}

	static HashEquals Equals(void)
	{
		return &SHashOfIRConstant::ConstantsEqual;
	}

	static bool ConstantsEqual(const void* pvKey, const void* pvEntryKey)
	{
		const SIRConstant*	ps1;
		const SIRConstant*	ps2;

		ps1 = (const SIRConstant*)pvKey;
		ps2 = (const SIRConstant*)pvEntryKey;
		return (ps1->sType.uiKind == ps2->sType.uiKind) && (ps1->sType.uiSpace == ps2->sType.uiSpace) && (ps1->ulliBits == ps2->ulliBits);
	}
};


typedef CHashSetTemplate<SIRConstant, SHashOfIRConstant>	CIRConstants;
typedef CArrayTemplate<SIRReference>		CArrayIRReferences;
typedef CMapStringTemplate<CIRBlock*>		CMapIRBlocks;


//What a use of a value is (SIRUse): a phi's argument, an instruction's operand or a block's jump's argument.
enum EIRUseKind
{
	IRUK_Phi,
	IRUK_Instruction,
	IRUK_Jump,
};


//Where a walk of a value's uses is (CIRFunction::FirstUse, FirstTemporaryUse and NextUse): block by block, each
//block's phis' arguments, then its instructions' operands, then its jump.  Each step fetches the block, phi and
//instruction again by their indices, so a walk may add phis and instructions (at the ends of the blocks) as it
//goes; SetUse writes a use the same way.
struct SIRUse
{
	SIRReference		sValue;			//The value whose uses are walked...
	bool				bTemporaries;	//...or, when true, every temporary's.
	CArrayIRBlocks*		papcBlocks;		//The blocks walked, NULL for all the function's.
	size				uiBlock;		//The use: its block (in papcBlocks or the function)...
	EIRUseKind			eKind;
	size				uiIndex;		//...the phi or instruction...
	size				uiArgument;		//...and the phi's argument or the instruction's operand.
	CIRBlock*			pcBlock;		//Fetched by NextUse: the use's block...
	SIRPhi*				psPhi;			//...a phi argument's phi and argument...
	SIRPhiArgument*		psArgument;
	SIRInstruction*		psInstruction;	//...an operand's instruction...
	SIRReference*		psSlot;			//...and the use itself (until the blocks change).
};


//A call inlined into a function (DebuggerFPlan.txt F2): the function inlined (a symbol of the module) and the call's
//location in the function (an instruction's uiSource: a source token + 1, or an inline location).
struct SIRInline
{
	uint32	uiCallee;
	uint32	uiCall;
};


//An inlined instruction's location: the inline it came from and its source token + 1 there (0 for none).  An
//instruction's uiSource with IR_INLINE_LOCATION set is the index of one of its function's (CIRFunction::
//AddInlineLocation).
struct SIRInlineLocation
{
	uint32	uiInline;
	uint32	uiSource;
};


#define IR_INLINE_LOCATION	0x80000000


typedef CArrayTemplate<SIRInline>			CArrayIRInlines;
typedef CArrayTemplate<SIRInlineLocation>	CArrayIRInlineLocations;


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
	uint32				muiRestrictParameters;	//A bit for each of the first 32 parameters that is a restrict pointer.

	CArrayIRTemporaries	masTemporaries;
	CArrayChars			maszTemporaryNames;
	CMapStringInt*		mpmTemporaryNames;

	CIRConstants		mcConstants;		//In the order added: a constant's index is its reference's.

	CArrayIRReferences	masKept;			//The kept slots (Slot's bKept) and the addresses in them: their loads and stores are volatile.

	CArrayIRBlocks		mapcBlocks;
	CMapIRBlocks*		mpmBlockNames;
	uint32*				mpuiSource;		//The module's source for instructions added now (CIRModule::muiSource).

	CArrayIRInlines			masInlines;			//The calls inlined into it, for a debugger...
	CArrayIRInlineLocations	masInlineLocations;	//...and the locations of their instructions.

public:
	void			Init(uint32 uiSymbol, SIRType sReturnType, uint32 uiAttributes, uint32* puiSource);
	void			Kill(void);
	void			CopyBody(CIRFunction* pcSource);

	SIRReference	AddTemporary(SIRType sType, const char* szName = NULL);
	SIRReference	AddParameter(SIRType sType, const char* szName = NULL);
	void			SetRestrictParameter(size uiParameter);
	bool			IsRestrictParameter(size uiParameter);
	int				FindParameter(SIRReference sValue);
	SIRReference	FindTemporary(const char* szName);
	size			NumTemporaries(void) const;
	SIRType			GetTemporaryType(SIRReference sTemporary);
	void			SetTemporaryType(SIRReference sTemporary, SIRType sType);
	char*			GetTemporaryName(SIRReference sTemporary);
	int32			GetDebugSlot(SIRReference sTemporary);
	void			ReplaceDebugValues(SIRReference sOld, SIRReference sNew, CArrayIRBlocks* papcBlocks);
	void			NameTemporary(SIRReference sTemporary, const char* szName);
	void			ForgetTemporaryName(SIRReference sTemporary);

	uint32			AddInline(uint32 uiCallee, uint32 uiCall);
	uint32			AddInlineLocation(uint32 uiInline, uint32 uiSource);
	uint32			SourceToken(uint32 uiLocation);
	SIRInline*		GetInline(uint32 uiLocation);
	SIRInlineLocation*	GetInlineLocation(uint32 uiLocation);

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
	size			NumBlocks(void) const;
	CIRBlock*		GetBlock(size uiIndex);
	void			SetBlockOrder(CArrayIRBlocks* papcBlocks);
	void			RemoveBlock(CIRBlock* pcBlock);
	void			Renumber(void);
	bool			IsNumbered(void);
	void			ComputePredecessors(void);
	void			CountUses(CArrayTemplate<uint32>* pauiUses);
	bool			FirstUse(SIRUse* psUse, SIRReference sValue, CArrayIRBlocks* papcBlocks = NULL, bool bTemporaries = false);
	bool			FirstTemporaryUse(SIRUse* psUse, CArrayIRBlocks* papcBlocks = NULL);
	bool			NextUse(SIRUse* psUse);
	void			SetUse(SIRUse* psUse, SIRReference sNew);
	void			ReplaceUses(SIRReference sOld, SIRReference sNew);

	SIRReference	Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, SIRReference sArgument1, const char* szName = NULL);
	SIRReference	Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, const char* szName = NULL);
	SIRReference	Load(CIRBlock* pcBlock, SIRType sType, SIRReference sAddress, bool bVolatile = false, const char* szName = NULL);
	void			Store(CIRBlock* pcBlock, SIRType sType, SIRReference sValue, SIRReference sAddress, bool bVolatile = false);
	SIRReference	Slot(CIRBlock* pcBlock, SIRType sPointerType, uint32 uiSize, uint32 uiAlign, const char* szName = NULL, bool bKept = false);
	bool			IsKept(SIRReference sAddress);
	void			PlainKeptAccesses(void);
	void			Argument(CIRBlock* pcBlock, SIRType sType, SIRReference sValue);
	SIRReference	Call(CIRBlock* pcBlock, SIRType sType, SIRReference sCallee, int iNamed = -1, bool bThisCall = false, const char* szName = NULL);
	SIRInstruction*	AddCall(CIRBlock* pcBlock, SIRType sType, SIRReference sResult, SIRReference sCallee, int iNamed, bool bThisCall = false);

protected:
	bool			IsName(const char* szName);
	void			UniqueName(const char* szName, CMapStringBlock* pmNames, CChars* pszDest);
};


//x / c or x % c with c a constant (Init): which it is, and c as the division reads it.
struct SIRConstantDivision
{
	bool	bSigned;
	bool	bRemainder;
	int64	lliDivisor;		//With its sign for a signed division, else the type's unsigned value.

	bool	Init(CIRFunction* pcFunction, SIRInstruction* psInstruction);
};


#endif // __I_R_FUNCTION_H__
