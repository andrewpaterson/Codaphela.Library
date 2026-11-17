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
#ifndef __I_R_INTERPRETER_H__
#define __I_R_INTERPRETER_H__
#include "BaseLib/ArrayChars.h"
#include "IRModule.h"


#define IR_INTERPRETER_STEPS	1000000		//Instructions run before a value is given up on.  [implimits]
#define IR_INTERPRETER_DEPTH	512			//Calls inside calls.
#define IR_VALUE_BYTES			8

//A byte's source: which object's pointer it is a byte of, and which byte (SIRValue, SIRObject).
#define IR_SOURCE(uiObject, uiByte)		(((uiObject) << 3) | (uint32)(uiByte))
#define IR_SOURCE_OBJECT(uiSource)		((uiSource) >> 3)
#define IR_SOURCE_BYTE(uiSource)		((uiSource) & 7)


//A value being worked out: an integer's or a float's bits, or a pointer.  A pointer is an object and an offset in
//it: the offset is the bits, and each of its bytes says which object's pointer it is a byte of and which byte, so a
//pointer copied a piece at a time as integers (as an object's bytes are) is still that pointer where it arrives.
struct SIRValue
{
	uint64	ulliBits;
	uint32	auiSource[IR_VALUE_BYTES];	//Per byte: (object << 3) | the byte of the pointer it is, 0 for none.
	bool	bPointer;					//Some byte has a source.

	void	Set(uint64 ulliValue);
};


enum EIRObjectKind
{
	IROB_Slot,			//A function's local, until it returns.
	IROB_Arguments,		//A call's arguments where the caller put them, until it returns.
	IROB_Global,		//A global's data.
	IROB_Function,		//A function: only its address is a value.
	IROB_Unknown,		//A symbol nothing is known of: only its address is a value.
	IROB_Made,			//An object the interpreter's user made (NewObject), to be worked out.
};


//Something a pointer can point into: its bytes as the target lays them out (little end first), which of them have
//a value yet, and which are a pointer's.
struct SIRObject
{
	uint8		uiKind;
	bool		bWritable;
	bool		bDead;			//A slot or the arguments of a call that has returned.
	size		uiFrame;		//The call a slot or arguments are of.
	CChars		szName;			//A symbol's name.
	size		uiSize;
	uint8*		pucBytes;
	uint8*		pucWritten;		//Per byte: 1 once it has a value.
	uint32*		pauiSource;		//Per byte: as SIRValue's.
};


typedef CArrayTemplate<SIRObject>	CArrayIRObjects;
typedef CArrayTemplate<SIRValue>	CArrayIRValues;


//What the interpreter asks of whoever has the program: what a symbol its module doesn't define is.
class CIRInterpreterHost
{
public:
	//The function named, as IR that hasn't been optimised, and the module it is in; NULL if there is none whose
	//value can be worked out (it isn't defined, or may not be called for a constant).
	virtual CIRFunction*	ConstantFunction(const char* szName, CIRModule** ppcModule) =0;

	//The object the symbol named is, made with NewNamedObject (and given its value, perhaps by calling the
	//interpreter), if it is data whose value never changes; 0 if it isn't one.
	virtual uint32			ConstantObject(const char* szName) =0;
};


//Runs a function of the generic opcodes (IROpcode.h) for the target its module is for, to give the value a constant
//expression has.  Memory is objects: a load or a store outside its object, of a byte that was never written, to data
//that never changes or through a pointer to a local that has gone is not a value, and neither is anything else the
//program would have to be run to know.  The reason is GetError's.
class CIRInterpreter
{
protected:
	CIRInterpreterHost*	mpcHost;
	CArrayIRObjects		masObjects;			//Every object, by its number; 0 is no object.
	CChars				mszError;			//Why what is being run has no value.
	uint32				muiErrorSource;		//The source token + 1 of the instruction that wasn't a constant, 0 for none.
	CChars				mszLastError;		//Why the last Call had none (GetError).
	uint32				muiLastErrorSource;
	CArrayChars			maszTrail;			//The functions what is being run failed in, the innermost first.
	CArrayChars			maszLastTrail;		//The last Call's (GetTrail).
	size				muiSteps;
	size				muiStepLimit;
	size				muiDepth;
	size				muiDepthLimit;
	size				muiFrames;			//Calls made: each one's number.

public:
	void		Init(CIRInterpreterHost* pcHost = NULL);
	void		Kill(void);
	void		SetLimits(size uiSteps, size uiDepth);

	bool		Call(CIRModule* pcModule, CIRFunction* pcFunction, SIRValue* pasArguments, size uiNumArguments, SIRValue* psResult);

	uint32		NewObject(size uiSize);
	uint32		NewNamedObject(const char* szName, size uiSize);
	void		Freeze(uint32 uiObject);
	void		Forget(uint32 uiObject);
	SIRValue	PointerTo(uint32 uiObject, uint64 ulliOffset, int iPointerBytes);
	uint32		ObjectOf(SIRValue* psPointer, int iPointerBytes);
	SIRObject*	GetObject(uint32 uiObject);
	bool		IsWritten(uint32 uiObject);

	char*		GetError(void);
	uint32		GetErrorSource(void) const;
	CArrayChars*	GetTrail(void);

protected:
	bool		Fail(const char* szReason, const char* szDetail = NULL);
	bool		Run(CIRModule* pcModule, CIRFunction* pcFunction, CArrayIRValues* pasArguments, CArrayTemplate<SIRType>* pasArgumentTypes, SIRValue* psResult);
	bool		Step(CIRModule* pcModule, CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* pasTemporaries, uint32 uiArguments, CArrayTemplate<size>* pauiArgumentOffsets,
					 CArrayIRValues* pasPending, CArrayTemplate<SIRType>* pasPendingTypes, size uiFrame);
	bool		ValueOf(CIRModule* pcModule, CIRFunction* pcFunction, SIRValue* pasTemporaries, SIRReference sReference, SIRValue* psValue);
	bool		Integer(CIRModule* pcModule, CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* psLeft, SIRValue* psRight, SIRValue* psResult);
	bool		Float(CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* psLeft, SIRValue* psRight, SIRValue* psResult);
	bool		Load(CIRModule* pcModule, SIRType sType, SIRValue* psAddress, int iAddressBytes, SIRValue* psResult);
	bool		Store(CIRModule* pcModule, SIRType sType, SIRValue* psValue, SIRValue* psAddress, int iAddressBytes);
	bool		CallInstruction(CIRModule* pcModule, CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* pasTemporaries, CArrayIRValues* pasPending, CArrayTemplate<SIRType>* pasPendingTypes);

	uint32		AddObject(EIRObjectKind eKind, size uiSize, bool bWritable, size uiFrame, const char* szName);
	uint32		SymbolObject(CIRModule* pcModule, uint32 uiSymbol);
	bool		Place(SIRValue* psAddress, int iPointerBytes, size uiBytes, bool bWrite, SIRObject** ppsObject, size* puiOffset);
	void		EndFrame(size uiFrame, uint32 uiFirstObject);
	void		FreeObject(SIRObject* psObject);
	int			ValueBytes(CIRModule* pcModule, SIRType sType);
};


#endif // __I_R_INTERPRETER_H__
