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
#ifndef __I_R_65_BACKEND_H__
#define __I_R_65_BACKEND_H__
#include "CPPErrors.h"
#include "IRModule.h"
#include "IRLiveness.h"
#include "IRDominators.h"


//Where a temporary is kept.
enum EIR65PlaceKind
{
	IR65P_None,				//Nowhere: left in A for the next instruction (bInA), or not used.
	IR65P_Frame,			//In the frame: [1 + offset + V].
	IR65P_Argument,			//A parameter where the caller pushed it: [1 + frame + 8 + offset + V].
	IR65P_FrameAddress,		//A slot's address, V + 1 + offset; not stored.
	IR65P_ArgumentAddress,	//An argaddr's address, V + 1 + frame + 8 + offset; not stored.
	IR65P_Register,			//X or Y (offset is IR65R_X or IR65R_Y).
	IR65P_Pseudo,			//A pseudo register: [offset + W].
	IR65P_Folded,			//A ptradd of sBase and offset only the next instruction uses, as its address.
};


//The registers a temporary can be kept in.  A and B are worked in.
enum EIR65Register
{
	IR65R_X,
	IR65R_Y,
	IR65R_Count
};


//A temporary's home (or, for a slot or argaddr, the address it is), from register allocation.
struct SIR65Place
{
	uint8			uiKind;
	int32			iOffset;
	int32			iBytes;		//What is kept: 4 or 8, a parameter's own size where the caller pushed it, a slot's size.
	bool			bInA;		//Only the next instruction uses it, and takes it from A.
	SIRReference	sBase;		//What a folded ptradd adds offset to.
};


typedef CArrayTemplate<SIR65Place>	CArrayIR65Places;


//The ranges of the temporaries given one home (a register, or four bytes of pseudo registers or frame), sorted.
struct SIR65Home
{
	CArrayIRRanges	asRanges;
	int32			iOffset;		//A frame unit's offset.
};


typedef CArrayTemplate<SIR65Home>	CArrayIR65Homes;


//How a value is extended to 32 bits when it is loaded.
enum EIR65Extend
{
	IR65E_Zero,
	IR65E_Sign,
};


//A copy a block's jump makes for a successor's phi (IR65Backend.cpp Edge).
struct SIR65Move
{
	SIRReference	sDestination;	//The phi.
	SIRReference	sSource;		//What it takes; IRNone once the copy is written.
	SIRType			sType;
	bool			bFromB;			//The source was copied to B to break a cycle.
};


typedef CArrayTemplate<SIR65Move>	CArrayIR65Moves;


//An edge's phi copies written after the function's ret: a branch goes to iLabel, which copies then jumps on.
struct SIR65Stub
{
	int32		iLabel;
	CIRBlock*	pcFrom;
	CIRBlock*	pcTo;
};


typedef CArrayTemplate<SIR65Stub>	CArrayIR65Stubs;


//A class of temporaries sharing a home and how much they are used, for the order homes are given in.
struct SIR65Weight
{
	int32	iLeader;
	uint32	uiWeight;
};


typedef CArrayTemplate<SIR65Weight>	CArrayIR65Weights;


#define IR65_PSEUDO_UNITS		32		//Four byte units of [$00 + W] to [$7F + W], and again from [$80 + W].
#define IR65_HOME_PSEUDO		IR65R_Count
#define IR65_HOME_SAVED			(IR65_HOME_PSEUDO + IR65_PSEUDO_UNITS)
#define IR65_HOME_FRAME			(IR65_HOME_SAVED + IR65_PSEUDO_UNITS)


//Writes 65C64 assembly for IR functions.  Temporaries are given homes by register allocation (IR.txt 65C64 back
//end): X or Y, a pseudo register ([$00 + W] to [$7F + W] if no call is made while it is live, [$80 + W] to
//[$FF + W] if one is: the function keeps those), or the frame.  A phi shares its home with the values it takes
//when their live ranges allow, and a jump copies the others.  Instructions work in A (and B), or in a result's
//register or pseudo register when that is shorter.  The calling convention is the generator's
//(CPP65Generator.txt): arguments pushed last first at their own size, the caller removes them, this in X for a
//thiscall, the result in A (all 8 bytes of a 64 bit one).  Labels are L and a number from the generator's
//counter; the soft float routines called are added to its runtime list.
class CIR65Backend
{
protected:
	CIRModule*				mpcModule;
	int*					mpiLabels;
	CArrayChars*			mpaszRuntime;
	bool					mbSignedDivide;
	bool					mbSignedDivide64;

	CIRFunction*			mpcFunction;
	CChars*					mpszOut;
	CCPPErrors*				mpcErrors;
	CIRDominators			mcDominators;
	CIRLiveness				mcLiveness;
	CArrayIR65Places		masPlaces;			//Per temporary.
	CArrayTemplate<uint8>	mauiAddressed;		//Per parameter: 1 if an argaddr of it is taken.
	CArrayTemplate<int32>	maiClasses;			//Per temporary: the temporaries sharing a home are a list through this...
	CArrayTemplate<int32>	maiLeaders;			//...and per temporary the first of its list.
	CArrayIR65Homes			masHomes;			//X, Y, the pseudo registers, the saved pseudo registers, the frame.
	CArrayTemplate<int32>	maiCalls;			//Positions of instructions that change X, Y and [$00 + W] to [$7F + W]...
	CArrayTemplate<int32>	maiChangesXY;		//...that change X and Y...
	CArrayTemplate<int32>	maiChangesY;		//...and that change Y.
	CArrayTemplate<int32>	maiBlockLabels;		//Per block index.
	CArrayTemplate<uint32>	mauiUses;			//Per temporary: how many times it is used.
	CArrayIR65Moves			masMoves;
	CArrayIR65Stubs			masStubs;
	uint32					muiSaved;			//The [$80 + W] units used, a bit each.
	int32					miFrameBytes;
	int32					miArguments;		//Where the arguments start: 1 + frame + 8.
	int32					miThisOffset;		//A thiscall's this in the frame, for an argaddr of it; -1 for none.
	int32					miReturnLabel;
	bool					mbResult;
	SIRReference			msInA;				//The temporary the last instruction left in A (or IRNone).
	SIRReference			msAvailable;		//What A holds as the current instruction starts.
	SIRReference			msDeferred;			//A load the next instruction reads itself, from mszDeferred.
	CChars					mszDeferred;
	SIRInstruction*			mpsWritten;			//Written with the instruction before it: a store (ReadModifyWrite) or a div's partner (Divide).
	SIRInstruction*			mpsFlags;			//The last integer comparison a block (mpcFlags) jumped on: the flags its successors start with.
	CIRBlock*				mpcFlags;
	bool					mbFlagsKnown;		//The block's comparison uses the flags it is entered with (FlagsKnown).
	SIRReference			msFlagsValue;		//n and z are this value's while mpszOut is muiFlagsLength long (SetFlagsValue).
	size					muiFlagsLength;
	SIRReference			msEndFlags;			//The value n and z are at the end of mpcEndFlags, whose branch used them.
	CIRBlock*				mpcEndFlags;
	bool					mbFlagsUsed;		//The jump's comparison branched on the flags a cpy set.
	bool					mbRightImmediate;	//LoadOperands left the second operand a constant, for "op A, value".
	bool					mbFrameless;		//No frame: V isn't set up, the arguments are from S...
	char					mcFrame;			//...so the frame register is S, otherwise V.

public:
	void				Init(CIRModule* pcModule, int* piLabels, CArrayChars* paszRuntime);
	void				Kill(void);

	bool				GenerateModule(CChars* pszDest, CCPPErrors* pcErrors);
	bool				GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors);
	void				GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest);
	bool				UsesSignedDivide(void);
	bool				UsesSignedDivide64(void);

protected:
	void				Binding(const char* szName, uint32 uiAttributes, CChars* pszDest);
	void				OrderBlocks(void);
	void				RotateLoops(void);
	void				AdjustComparisons(void);
	void				PreferSingleBranches(void);
	void				PairDivides(void);
	bool				DividePartner(SIRInstruction* psFirst, SIRInstruction* psPartner);
	void				SinkLoads(void);
	void				SinkToEnd(CIRBlock* pcBlock);
	bool				ConstantPtrAdd(SIRInstruction* psInstruction);
	void				CopyAddresses(void);
	void				MarkFolded(void);
	void				Peephole(size uiStart);
	void				Prologue(const char* szLabel);
	void				Epilogue(void);
	bool				Error(const char* szMessage, const char* szDetail = NULL);
	CChars*				Line(const char* szText);
	int32				NewLabel(void);
	void				Label(int32 iLabel);
	void				Branch(const char* szBranch, int32 iLabel);
	void				CallRuntime(const char* szName, int iBytes);
	int					ValueBytes(SIRType sType);
	bool				IsWide(SIRType sType);
	bool				IsNarrow(SIRType sType);
	int32				AddFrame(int32 iBytes);
	SIRInstruction*		FusedComparison(CIRBlock* pcBlock);
	void				MarkInA(void);
	bool				FoldedBase(SIRInstruction* psNext, SIRReference sValue);
	int32				ParameterOffset(size uiParameter);
	void				ArrivalPlace(size uiParameter, SIR65Place* psPlace);
	void				Layout(void);
	bool				NeedsHome(int32 iTemporary);
	void				Allocate(void);
	void				DiscountAddresses(CArrayIR65Weights* pasWeights);
	void				FindChanges(void);
	void				Coalesce(void);
	int32				Find(int32 iTemporary);
	bool				Interferes(int32 iLeader1, int32 iLeader2);
	void				Join(int32 iLeader1, int32 iLeader2);
	bool				Crosses(int32 iLeader, CArrayTemplate<int32>* paiPositions);
	bool				HomeFree(int32 iHome, int32 iLeader);
	int32				FindRange(CArrayIRRanges* pasRanges, SIRRange* psRange);
	void				TakeHome(int32 iHome, int32 iLeader);
	void				PlaceClass(int32 iLeader, uint8 uiKind, int32 iOffset, int32 iBytes);
	bool				TakeHomes(int32 iHome, int32 iUnits, int32 iLeader);
	void				AllocateClass(int32 iLeader, uint32 uiWeight);
	int32				FindUnits(int32 iFirstHome, int32 iUnits, int32 iLeader);
	SIR65Place*			PlaceOf(SIRReference sValue);
	bool				IsHome(SIR65Place* psPlace);
	bool				IsMemoryHome(SIR65Place* psPlace);
	bool				SamePlace(SIRReference sValue1, SIRReference sValue2);
	bool				Overlaps(SIR65Place* psPlace1, SIR65Place* psPlace2);
	bool				IsDeferred(SIRReference sValue);
	bool				IsInA(SIRReference sValue);
	bool				InRegister(SIRReference sValue);
	const char*			RegisterName(SIR65Place* psPlace);
	bool				IsFrameOperand(CChars* pszOperand);
	bool				IsLeaf(void);
	bool				ArgumentsReadFirst(void);
	bool				IsArrivalPlace(SIRReference sValue);
	int					NumSaved(void);
	void				AppendPlace(CChars* psz, SIR65Place* psPlace);
	void				AppendImmediate(CChars* psz, SIRReference sConstant, EIR65Extend eExtend);
	bool				Operand(SIRReference sValue, CChars* pszOperand, bool bArithmetic, EIR65Extend eExtend);
	bool				SmallImmediate(SIRReference sValue, int iMin, int iMax);
	const char*			WorkRegister(SIRInstruction* psInstruction);
	void				Extend(const char* szRegister, SIRType sType, EIR65Extend eExtend);
	void				Truncate(const char* szRegister, SIRType sType);
	void				LoadConstant(const char* szRegister, SIRType sType, uint64 ulliBits, EIR65Extend eExtend);
	void				LoadPlace(SIR65Place* psPlace, SIRType sType, const char* szRegister, EIR65Extend eExtend);
	void				StorePlace(SIR65Place* psPlace, SIRType sType, const char* szRegister);
	void				LoadValue(SIRReference sValue, const char* szRegister, EIR65Extend eExtend);
	void				StoreResult(SIRReference sResult, const char* szRegister = "A", bool bExtended = false);
	void				MemoryOperand(SIRReference sAddress, CChars* pszOperand, const char* szWidened);
	void				FoldedOperand(SIR65Place* psFolded, CChars* pszOperand, const char* szWidened);
	void				Push(SIRReference sValue, SIRType sType);
	void				Multiply(const char* szInstruction);
	bool				HasMoves(CIRBlock* pcFrom, CIRBlock* pcTo);
	bool				Skipped(SIRInstruction* psInstruction);
	size				NextWritten(CIRBlock* pcBlock, size uiAfter, size uiEnd);
	bool				Block(CIRBlock* pcBlock, CIRBlock* pcNext);
	bool				BranchOn(SIRInstruction* psFused, SIRReference sCondition, bool bJumpIf, int32 iLabel);
	bool				EnteredOnlyFrom(CIRBlock* pcBlock, CIRBlock* pcFrom);
	void				SetFlagsValue(SIRReference sValue);
	bool				FlagsHold(SIRReference sValue);
	bool				IsZero(SIRReference sValue);
	bool				FlagsKnown(CIRBlock* pcBlock, SIRInstruction* psFused);
	void				Edge(CIRBlock* pcFrom, CIRBlock* pcTo);
	void				Move(SIRReference sDestination, SIRReference sSource, SIRType sType, bool bFromB);
	void				TestZero(SIRReference sValue);
	bool				Jump(CIRBlock* pcBlock, CIRBlock* pcNext, SIRInstruction* psFused);
	bool				Instruction(CIRBlock* pcBlock, size uiIndex);
	bool				Commutes(uint16 uiOpcode);
	bool				ReadModifyWrite(CIRBlock* pcBlock, size uiIndex);
	bool				Integer(SIRInstruction* psInstruction);
	bool				IntegerWide(SIRInstruction* psInstruction);
	bool				MultiplyInstruction(SIRInstruction* psInstruction);
	bool				MultiplyByShifts(SIRInstruction* psInstruction);
	bool				ShiftSigned(SIRInstruction* psInstruction);
	bool				Divide(SIRInstruction* psInstruction, SIRInstruction* psNext);
	bool				Conversion(SIRInstruction* psInstruction);
	bool				FloatOperation(SIRInstruction* psInstruction);
	bool				FloatConversion(SIRInstruction* psInstruction);
	bool				Defer(CIRBlock* pcBlock, size uiIndex);
	bool				PureMemoryOperand(SIRReference sAddress, CChars* pszOperand, char* pcRegister);
	bool				Load(CIRBlock* pcBlock, size uiIndex);
	bool				Store(SIRInstruction* psInstruction);
	bool				Call(CIRBlock* pcBlock, size uiIndex);
	bool				Compare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	void				CompareOperands(SIRReference sFirst, SIRReference sSecond);
	bool				IntegerCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	bool				FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel);
	bool				TakesInA(SIRInstruction* psNext, SIRReference sValue);
	bool				JumpTakesInA(CIRBlock* pcBlock, SIRInstruction* psFused, SIRReference sValue);
	bool				Immediate(SIRReference sValue, SIRType sType);
	void				LoadOperands(SIRInstruction* psInstruction, EIR65Extend eExtend, bool bImmediate);
	bool				TakesInB(SIRInstruction* psNext, SIRReference sValue);
};


#endif // __I_R_65_BACKEND_H__
