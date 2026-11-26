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
#include <stdlib.h>
#include <string.h>
#include "IRFunction.h"
#include "IRInterpreter.h"




//////////////////////////////////////////////////////////////////////////
//
// A value that is no pointer.
//
//////////////////////////////////////////////////////////////////////////
void SIRValue::Set(uint64 ulliValue)
{
	ulliBits = ulliValue;
	memset(auiSource, 0, sizeof(auiSource));
	bPointer = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::Init(CIRInterpreterHost* pcHost)
{
	mpcHost = pcHost;
	masObjects.Init();
	mszError.Init();
	muiErrorSource = 0;
	mszLastError.Init();
	muiLastErrorSource = 0;
	maszTrail.Init();
	maszLastTrail.Init();
	muiSteps = 0;
	muiStepLimit = IR_INTERPRETER_STEPS;
	muiInstructions = 0;
	muiInstructionLimit = IR_INTERPRETER_STEPS * IR_INTERPRETER_INSTRUCTIONS_PER_STEP;
	muiDepth = 0;
	muiDepthLimit = IR_INTERPRETER_DEPTH;
	muiFrames = 0;
	mpsFrame = NULL;

	//Object 0 is no object.
	AddObject(IROB_Unknown, 0, false, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::Kill(void)
{
	size	i;

	while (mpsFrame)
	{
		PopFrame(false);
	}
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		FreeObject(masObjects.Get(i));
		masObjects.Get(i)->szName.Kill();
	}
	masObjects.Kill();
	maszLastTrail.Kill();
	maszTrail.Kill();
	mszLastError.Kill();
	mszError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many full-expressions (instructions marked IRF_FullExpression) may be
// evaluated before a value is given up on, and how deep calls may go.  As
// many instructions and jumps as IR_INTERPRETER_INSTRUCTIONS_PER_STEP times
// the steps may be run whatever is marked.  [implimits]
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::SetLimits(size uiSteps, size uiDepth)
{
	muiStepLimit = uiSteps;
	muiInstructionLimit = uiSteps * IR_INTERPRETER_INSTRUCTIONS_PER_STEP;
	muiDepthLimit = uiDepth;
}


//////////////////////////////////////////////////////////////////////////
//
// The labels of the functions the last Call failed in, from the one that
// failed out to the one called.
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CIRInterpreter::GetTrail(void)
{
	return &maszLastTrail;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIRInterpreter::GetError(void) const
{
	return mszLastError.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::GetErrorSource(void) const
{
	return muiLastErrorSource;
}


//////////////////////////////////////////////////////////////////////////
//
// Why there is no value: the first reason is the one kept.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Fail(const char* szReason, const char* szDetail)
{
	if (mszError.Length() == 0)
	{
		mszError.Append(szReason);
		if (szDetail)
		{
			mszError.Append(" [");
			mszError.Append(szDetail);
			mszError.Append("]");
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes a value of the type is in memory and as an argument.
//
//////////////////////////////////////////////////////////////////////////
int CIRInterpreter::ValueBytes(CIRModule* pcModule, SIRType sType)
{
	return sType.Is(IRTK_I1) ? 1 : pcModule->GetSize(sType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::AddObject(EIRObjectKind eKind, size uiSize, bool bWritable, size uiFrame, const char* szName)
{
	SIRObject*	psObject;

	psObject = masObjects.Add();
	psObject->uiKind = (uint8)eKind;
	psObject->bWritable = bWritable;
	psObject->bDead = false;
	psObject->uiFrame = uiFrame;
	psObject->szName.Init(szName ? szName : "");
	psObject->uiSize = uiSize;
	psObject->pucBytes = NULL;
	psObject->pucWritten = NULL;
	psObject->pauiSource = NULL;
	if (uiSize > 0)
	{
		psObject->pucBytes = (uint8*)calloc(uiSize, 1);
		psObject->pucWritten = (uint8*)calloc(uiSize, 1);
		psObject->pauiSource = (uint32*)calloc(uiSize, sizeof(uint32));
	}
	return (uint32)(masObjects.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::FreeObject(SIRObject* psObject)
{
	free(psObject->pucBytes);
	free(psObject->pucWritten);
	free(psObject->pauiSource);
	psObject->pucBytes = NULL;
	psObject->pucWritten = NULL;
	psObject->pauiSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An object of uiSize bytes to be worked out: none of it has a value yet,
// and what is run may write it (a variable its constructor makes).
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::NewObject(size uiSize)
{
	return AddObject(IROB_Made, uiSize, true, 0, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The object a symbol is, made by the host (CIRInterpreterHost::
// ConstantObject) to be given its value: what is run may write it until
// it is frozen.
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::NewNamedObject(const char* szName, size uiSize)
{
	return AddObject(IROB_Global, uiSize, true, 0, szName);
}


//////////////////////////////////////////////////////////////////////////
//
// The object has its value: from now on it is data that never changes.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::Freeze(uint32 uiObject)
{
	masObjects.Get(uiObject)->bWritable = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The object is no symbol's after all (its value couldn't be worked out):
// a pointer to it is to nothing, and the symbol is asked for again.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::Forget(uint32 uiObject)
{
	SIRObject*	psObject;

	psObject = masObjects.Get(uiObject);
	psObject->bDead = true;
	psObject->szName.Clear();
	FreeObject(psObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRObject* CIRInterpreter::GetObject(uint32 uiObject)
{
	return masObjects.Get(uiObject);
}


//////////////////////////////////////////////////////////////////////////
//
// True if every byte of the object has a value.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::IsWritten(uint32 uiObject)
{
	SIRObject*	psObject;
	size		i;

	psObject = masObjects.Get(uiObject);
	for (i = 0; i < psObject->uiSize; i++)
	{
		if (!psObject->pucWritten[i])
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer to the byte at ulliOffset of an object.
//
//////////////////////////////////////////////////////////////////////////
SIRValue CIRInterpreter::PointerTo(uint32 uiObject, uint64 ulliOffset, int iPointerBytes)
{
	SIRValue	sValue;
	int			i;

	sValue.Set((iPointerBytes < 8) ? (ulliOffset & ((((uint64)1) << (8 * iPointerBytes)) - 1)) : ulliOffset);
	if (uiObject != 0)
	{
		for (i = 0; i < iPointerBytes; i++)
		{
			sValue.auiSource[i] = IR_SOURCE(uiObject, i);
		}
		sValue.bPointer = true;
	}
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The object a value points into if it is a whole pointer of
// iPointerBytes, in order; 0 if it is not (null, an integer, or part of a
// pointer).
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::ObjectOf(SIRValue* psPointer, int iPointerBytes)
{
	uint32	uiObject;
	int		i;

	if (!psPointer->bPointer)
	{
		return 0;
	}
	uiObject = IR_SOURCE_OBJECT(psPointer->auiSource[0]);
	for (i = 0; i < iPointerBytes; i++)
	{
		if (psPointer->auiSource[i] != IR_SOURCE(uiObject, i))
		{
			return 0;
		}
	}
	return uiObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object a symbol is: a function, or a global with the data it has
// when the program starts, made the first time it is named.  A symbol
// nothing defines is an object nothing can be done with but have its
// address.
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRInterpreter::SymbolObject(CIRModule* pcModule, uint32 uiSymbol)
{
	char*			szName;
	CIRModule*		pcDefined;
	CIRGlobal*		pcGlobal;
	SIRDataItem*	psItem;
	SIRObject*		psObject;
	SIRValue		sPointer;
	uint32			uiObject;
	uint32			uiTarget;
	size			i;
	size			uiOffset;
	int				iBytes;
	int				j;

	szName = pcModule->GetSymbolName(uiSymbol);
	for (i = 1; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		if (((psObject->uiKind == IROB_Global) || (psObject->uiKind == IROB_Function) || (psObject->uiKind == IROB_Unknown)) && psObject->szName.Equals(szName))
		{
			return (uint32)i;
		}
	}

	pcDefined = pcModule;
	if (pcModule->FindFunction(uiSymbol) || (mpcHost && mpcHost->ConstantFunction(szName, &pcDefined)))
	{
		return AddObject(IROB_Function, 0, false, 0, szName);
	}
	pcGlobal = pcModule->FindGlobal(uiSymbol);
	if (pcGlobal && !(pcGlobal->muiAttributes & IRA_Constant))
	{
		pcGlobal = NULL;
	}
	if (!pcGlobal)
	{
		pcGlobal = pcModule->FindConstantData(uiSymbol);
	}
	if (!pcGlobal)
	{
		uiObject = mpcHost ? mpcHost->ConstantObject(szName) : 0;
		return (uiObject != 0) ? uiObject : AddObject(IROB_Unknown, 0, false, 0, szName);
	}

	//Its items, one after another.  The object is added first: an item may be its own address.
	uiObject = AddObject(IROB_Global, pcGlobal->Size(pcDefined), false, 0, szName);
	uiOffset = 0;
	for (i = 0; i < pcGlobal->masItems.NumElements(); i++)
	{
		psItem = pcGlobal->masItems.Get(i);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
			iBytes = ValueBytes(pcDefined, psItem->sType);
			for (j = 0; j < iBytes; j++)
			{
				masObjects.Get(uiObject)->pucBytes[uiOffset + j] = (uint8)(psItem->ulliValue >> (8 * j));
			}
			uiOffset += iBytes;
			break;
		case IRDK_Symbol:
			iBytes = ValueBytes(pcDefined, psItem->sType);
			uiTarget = SymbolObject(pcDefined, (uint32)psItem->ulliValue);
			sPointer = PointerTo(uiTarget, (uint64)psItem->lliOffset, iBytes);
			psObject = masObjects.Get(uiObject);
			for (j = 0; j < iBytes; j++)
			{
				psObject->pucBytes[uiOffset + j] = (uint8)(sPointer.ulliBits >> (8 * j));
				psObject->pauiSource[uiOffset + j] = sPointer.auiSource[j];
			}
			uiOffset += iBytes;
			break;
		case IRDK_Bytes:
			memcpy(masObjects.Get(uiObject)->pucBytes + uiOffset, pcGlobal->macBytes.GetData() + psItem->uiStart, psItem->uiLength);
			uiOffset += psItem->uiLength;
			break;
		default:
			uiOffset += (size)psItem->ulliValue;
			break;
		}
	}
	psObject = masObjects.Get(uiObject);
	if (psObject->uiSize > 0)
	{
		memset(psObject->pucWritten, 1, psObject->uiSize);
	}
	return uiObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object and offset a load or a store of uiBytes through an address
// is of, if it may be done.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Place(SIRValue* psAddress, int iPointerBytes, size uiBytes, bool bWrite, SIRObject** ppsObject, size* puiOffset)
{
	uint32		uiObject;
	SIRObject*	psObject;

	uiObject = ObjectOf(psAddress, iPointerBytes);
	if (uiObject == 0)
	{
		return Fail((psAddress->ulliBits == 0) && !psAddress->bPointer ? "It reads or writes through a null pointer" : "It reads or writes through a pointer made from a number");
	}
	psObject = masObjects.Get(uiObject);
	if ((psObject->uiKind == IROB_Function) || (psObject->uiKind == IROB_Unknown))
	{
		return Fail(bWrite ? "It writes what isn't a constant" : "It reads what isn't a constant", psObject->szName.Text());
	}
	if (psObject->bDead)
	{
		return Fail("It reads or writes a local of a function that has returned");
	}
	if ((psAddress->ulliBits > psObject->uiSize) || (uiBytes > psObject->uiSize - (size)psAddress->ulliBits))
	{
		return Fail("It reads or writes outside an object", (psObject->szName.Length() > 0) ? psObject->szName.Text() : NULL);
	}
	if (bWrite && !psObject->bWritable)
	{
		return Fail("It writes what can't be changed", (psObject->szName.Length() > 0) ? psObject->szName.Text() : NULL);
	}
	*ppsObject = psObject;
	*puiOffset = (size)psAddress->ulliBits;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Load(CIRModule* pcModule, SIRType sType, SIRValue* psAddress, int iAddressBytes, SIRValue* psResult)
{
	SIRObject*	psObject;
	size		uiOffset;
	int			iBytes;
	int			i;

	iBytes = ValueBytes(pcModule, sType);
	if (!Place(psAddress, iAddressBytes, (size)iBytes, false, &psObject, &uiOffset))
	{
		return false;
	}
	psResult->Set(0);
	for (i = 0; i < iBytes; i++)
	{
		if (!psObject->pucWritten[uiOffset + i])
		{
			return Fail("It reads a variable that has no value yet");
		}
		psResult->ulliBits |= ((uint64)psObject->pucBytes[uiOffset + i]) << (8 * i);
		psResult->auiSource[i] = psObject->pauiSource[uiOffset + i];
		psResult->bPointer |= psResult->auiSource[i] != 0;
	}
	if (sType.Is(IRTK_I1))
	{
		psResult->ulliBits &= 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Store(CIRModule* pcModule, SIRType sType, SIRValue* psValue, SIRValue* psAddress, int iAddressBytes)
{
	SIRObject*	psObject;
	size		uiOffset;
	int			iBytes;
	int			i;

	iBytes = ValueBytes(pcModule, sType);
	if (!Place(psAddress, iAddressBytes, (size)iBytes, true, &psObject, &uiOffset))
	{
		return false;
	}
	for (i = 0; i < iBytes; i++)
	{
		psObject->pucBytes[uiOffset + i] = (uint8)(psValue->ulliBits >> (8 * i));
		psObject->pucWritten[uiOffset + i] = 1;
		psObject->pauiSource[uiOffset + i] = psValue->auiSource[i];
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What an operand is: a temporary's value, a constant, or a symbol's
// address.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::ValueOf(CIRModule* pcModule, CIRFunction* pcFunction, SIRValue* pasTemporaries, SIRReference sReference, SIRValue* psValue)
{
	switch (sReference.Kind())
	{
	case IRRK_Temporary:
		*psValue = pasTemporaries[sReference.Index()];
		return true;
	case IRRK_Constant:
		psValue->Set(pcFunction->GetConstant(sReference)->ulliBits);
		return true;
	case IRRK_Symbol:
		*psValue = PointerTo(SymbolObject(pcModule, sReference.Index()), 0, pcModule->GetPointerSize(0));
		return true;
	default:
		return Fail("It is not IR that can be run");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An integer instruction (IRFoldInteger's arithmetic).  A pointer's bits
// are its offset, so what is done to pointers as integers is done here to
// what they are: equal if they are the same place, ordered and subtracted
// inside one object, moved by adding a number; a conversion keeps the
// bytes it keeps.  Anything else makes a number of a pointer, which is no
// constant.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Integer(CIRModule* pcModule, CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* psLeft, SIRValue* psRight, SIRValue* psResult)
{
	SIRType	sType;
	SIRType	sFrom;
	uint64	ulliResult;
	uint32	uiLeft;
	uint32	uiRight;
	uint16	uiOpcode;
	int		iBits;
	int		iBytes;
	int		i;
	bool	bComparison;

	uiOpcode = psInstruction->uiOpcode;
	sType = psInstruction->sType;
	sFrom = pcFunction->TypeOf(psInstruction->asArguments[0]);
	iBits = sType.IsPointer() ? 8 * pcModule->GetPointerSize(sType.uiSpace) : sType.IntegerBits();
	bComparison = IsIRIntegerComparison(uiOpcode);
	if (psLeft->bPointer || psRight->bPointer)
	{
		iBytes = ValueBytes(pcModule, bComparison ? sType : sFrom);
		uiLeft = ObjectOf(psLeft, iBytes);
		uiRight = ObjectOf(psRight, iBytes);
		if (bComparison && ((uiOpcode == IRO_Eq) || (uiOpcode == IRO_Ne)) && (uiLeft != uiRight))
		{
			//Two objects, or an object and null, are not the same place.
			if (((uiLeft == 0) && (psLeft->bPointer || (psLeft->ulliBits != 0))) || ((uiRight == 0) && (psRight->bPointer || (psRight->ulliBits != 0))))
			{
				return Fail("It compares a pointer with a number");
			}
			psResult->Set(uiOpcode == IRO_Ne);
			return true;
		}
		if ((bComparison || (uiOpcode == IRO_Sub)) && (uiLeft != 0) && (uiLeft == uiRight))
		{
			//Inside one object the offsets say it all.
		}
		else if (((uiOpcode == IRO_Add) && ((uiLeft != 0) != (uiRight != 0)) && !(psLeft->bPointer && psRight->bPointer)) || ((uiOpcode == IRO_Sub) && (uiLeft != 0) && !psRight->bPointer))
		{
			if (!IRFoldInteger(uiOpcode, sType, sFrom, iBits, psLeft->ulliBits, psRight->ulliBits, &ulliResult))
			{
				return Fail("It is not IR that can be run");
			}
			*psResult = PointerTo((uiLeft != 0) ? uiLeft : uiRight, IRTruncate(sType, ulliResult), iBytes);
			return true;
		}
		else if ((uiOpcode == IRO_Copy) || (uiOpcode == IRO_ZExt) || (uiOpcode == IRO_Trunc) || (uiOpcode == IRO_PtrToI) || (uiOpcode == IRO_IToPtr))
		{
			//The bytes kept are the bytes they were.
			*psResult = *psLeft;
			iBytes = ValueBytes(pcModule, sType);
			psResult->ulliBits = (iBytes < 8) ? (psLeft->ulliBits & ((((uint64)1) << (8 * iBytes)) - 1)) : psLeft->ulliBits;
			psResult->bPointer = false;
			for (i = 0; i < IR_VALUE_BYTES; i++)
			{
				if (i >= iBytes)
				{
					psResult->auiSource[i] = 0;
				}
				psResult->bPointer |= psResult->auiSource[i] != 0;
			}
			return true;
		}
		else
		{
			return Fail(bComparison ? "It compares pointers to different objects" : "It uses a pointer as a number");
		}
	}

	if (!IRFoldInteger(uiOpcode, sType, sFrom, iBits, psLeft->ulliBits, psRight->ulliBits, &ulliResult))
	{
		if ((uiOpcode == IRO_SDiv) || (uiOpcode == IRO_UDiv) || (uiOpcode == IRO_SRem) || (uiOpcode == IRO_URem))
		{
			return Fail("It divides by zero, or the least number by -1");
		}
		if ((uiOpcode == IRO_Shl) || (uiOpcode == IRO_LShr) || (uiOpcode == IRO_AShr))
		{
			return Fail("It shifts by the width of the number or more");
		}
		return Fail("It is not IR that can be run");
	}
	if ((psInstruction->uiFlags & IRF_NoSignedWrap) && IRSignedOverflows(uiOpcode, iBits, psLeft->ulliBits, psRight->ulliBits, ulliResult))
	{
		return Fail("Its signed arithmetic overflows");
	}
	psResult->Set(bComparison ? (ulliResult & 1) : IRTruncate(sType, ulliResult));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A floating point instruction, worked out with the host's float and
// double.
//
//////////////////////////////////////////////////////////////////////////
double IRInterpreterFloat(SIRType sType, uint64 ulliBits)
{
	float	fValue;
	double	dValue;
	uint32	uiBits;

	if (sType.Is(IRTK_F32))
	{
		uiBits = (uint32)ulliBits;
		memcpy(&fValue, &uiBits, sizeof(fValue));
		return fValue;
	}
	memcpy(&dValue, &ulliBits, sizeof(dValue));
	return dValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint64 IRInterpreterFloatBits(SIRType sType, double dValue)
{
	float	fValue;
	uint64	ulliBits;
	uint32	uiBits;

	if (sType.Is(IRTK_F32))
	{
		fValue = (float)dValue;
		memcpy(&uiBits, &fValue, sizeof(uiBits));
		return uiBits;
	}
	memcpy(&ulliBits, &dValue, sizeof(ulliBits));
	return ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Float(CIRFunction* pcFunction, SIRInstruction* psInstruction, SIRValue* psLeft, SIRValue* psRight, SIRValue* psResult)
{
	SIRType	sType;
	SIRType	sFrom;
	double	a;
	double	b;
	double	dLimit;
	int		iBits;

	sType = psInstruction->sType;
	sFrom = pcFunction->TypeOf(psInstruction->asArguments[0]);
	if (psLeft->bPointer || psRight->bPointer)
	{
		return Fail("It uses a pointer as a number");
	}
	a = sFrom.IsFloat() ? IRInterpreterFloat(sFrom, psLeft->ulliBits) : 0.0;
	b = IRInterpreterFloat(sType, psRight->ulliBits);
	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	psResult->Set(IRInterpreterFloatBits(sType, sType.Is(IRTK_F32) ? (double)((float)a + (float)b) : a + b));	break;
	case IRO_FSub:	psResult->Set(IRInterpreterFloatBits(sType, sType.Is(IRTK_F32) ? (double)((float)a - (float)b) : a - b));	break;
	case IRO_FMul:	psResult->Set(IRInterpreterFloatBits(sType, sType.Is(IRTK_F32) ? (double)((float)a * (float)b) : a * b));	break;
	case IRO_FDiv:
		if (b == 0.0)
		{
			return Fail("It divides by zero");
		}
		psResult->Set(IRInterpreterFloatBits(sType, sType.Is(IRTK_F32) ? (double)((float)a / (float)b) : a / b));
		break;
	case IRO_FNeg:	psResult->Set(IRInterpreterFloatBits(sType, -a));	break;
	case IRO_FEq:	psResult->Set(a == b);	break;
	case IRO_FNe:	psResult->Set(a != b);	break;
	case IRO_FLt:	psResult->Set(a < b);	break;
	case IRO_FLe:	psResult->Set(a <= b);	break;
	case IRO_FGt:	psResult->Set(a > b);	break;
	case IRO_FGe:	psResult->Set(a >= b);	break;
	case IRO_FOrdered:		psResult->Set((a == a) && (b == b));	break;
	case IRO_FUnordered:	psResult->Set((a != a) || (b != b));	break;
	case IRO_SIToF:	psResult->Set(IRInterpreterFloatBits(sType, (double)IRSignExtend(sFrom, psLeft->ulliBits)));	break;
	case IRO_UIToF:	psResult->Set(IRInterpreterFloatBits(sType, (double)IRTruncate(sFrom, psLeft->ulliBits)));	break;
	case IRO_FToSI:
	case IRO_FToUI:
		//A value its type can't hold is no value.  [conv.fpint] 1
		iBits = sType.IntegerBits();
		dLimit = 18446744073709551616.0;
		for (; iBits < 64; iBits++)
		{
			dLimit /= 2.0;
		}
		if ((a != a) || ((psInstruction->uiOpcode == IRO_FToSI) ? ((a >= dLimit / 2.0) || (a < -dLimit / 2.0)) : ((a >= dLimit) || (a <= -1.0))))
		{
			return Fail("It converts a floating point number to an integer that can't hold it");
		}
		psResult->Set(IRTruncate(sType, (psInstruction->uiOpcode == IRO_FToSI) ? (uint64)(int64)a : (uint64)a));
		break;
	case IRO_FExt:
	case IRO_FTrunc:
		psResult->Set(IRInterpreterFloatBits(sType, a));
		break;
	default:
		return Fail("It is not IR that can be run");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call: the function a symbol or a pointer names, with the arguments
// the arg instructions before it gave, in a frame that is then the
// innermost (Run runs it).
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::CallInstruction(SIRFrame* psFrame, SIRInstruction* psInstruction)
{
	SIRValue		sCallee;
	SIRObject*		psObject;
	CIRModule*		pcCalledModule;
	CIRFunction*	pcCalled;
	CChars			szName;
	uint32			uiObject;
	int				iSymbol;
	bool			bResult;
	CIRModule*		pcModule;
	CIRFunction*	pcFunction;

	pcModule = psFrame->pcModule;
	pcFunction = psFrame->pcFunction;
	if (!ValueOf(pcModule, pcFunction, psFrame->pasTemporaries, psInstruction->asArguments[0], &sCallee))
	{
		return false;
	}
	uiObject = ObjectOf(&sCallee, pcModule->GetPointerSize(pcFunction->TypeOf(psInstruction->asArguments[0]).uiSpace));
	psObject = masObjects.Get(uiObject);
	if ((uiObject == 0) || (psObject->uiKind != IROB_Function) || (sCallee.ulliBits != 0))
	{
		return Fail("It calls what isn't a function whose value can be worked out", ((uiObject != 0) && (psObject->szName.Length() > 0)) ? psObject->szName.Text() : NULL);
	}

	//The host writing the callee may work out other constants, which adds objects: psObject is not kept.
	szName.Init(psObject->szName.Text());
	pcCalledModule = pcModule;
	iSymbol = pcModule->FindSymbol(szName.Text());
	pcCalled = (iSymbol >= 0) ? pcModule->FindFunction((uint32)iSymbol) : NULL;
	if (!pcCalled && mpcHost)
	{
		pcCalled = mpcHost->ConstantFunction(szName.Text(), &pcCalledModule);
	}
	if (!pcCalled)
	{
		Fail("It calls what isn't a function whose value can be worked out", szName.Text());
		szName.Kill();
		return false;
	}
	szName.Kill();

	//Run goes on in the callee, and gives the caller what it returns.
	bResult = PushFrame(pcCalledModule, pcCalled, &psFrame->asPending, &psFrame->asPendingTypes);
	psFrame->asPending.ReInit();
	psFrame->asPendingTypes.ReInit();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// One instruction.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Step(SIRFrame* psFrame, SIRInstruction* psInstruction)
{
	CIRModule*	pcModule;
	CIRFunction*	pcFunction;
	SIRValue*	pasTemporaries;
	SIROpcode*	psOpcode;
	SIRValue	sLeft;
	SIRValue	sRight;
	SIRValue	sResult;
	SIRType		sType;
	uint64		ulliIndex;
	uint32		uiObject;
	int			iPointerBytes;

	pcModule = psFrame->pcModule;
	pcFunction = psFrame->pcFunction;
	pasTemporaries = psFrame->pasTemporaries;
	psOpcode = GetIROpcode(psInstruction->uiOpcode);
	if (!psOpcode || (psInstruction->uiOpcode >= IRO_NumGeneric))
	{
		return Fail("It is not IR that can be run");
	}
	if (psInstruction->uiOpcode == IRO_Call)
	{
		return CallInstruction(psFrame, psInstruction);
	}

	sLeft.Set(0);
	sRight.Set(0);
	if (!psInstruction->asArguments[0].IsNone() && !ValueOf(pcModule, pcFunction, pasTemporaries, psInstruction->asArguments[0], &sLeft))
	{
		return false;
	}
	if (!psInstruction->asArguments[1].IsNone() && !ValueOf(pcModule, pcFunction, pasTemporaries, psInstruction->asArguments[1], &sRight))
	{
		return false;
	}

	sType = psInstruction->sType;
	iPointerBytes = pcModule->GetPointerSize(sType.IsPointer() ? sType.uiSpace : 0);
	sResult.Set(0);
	switch (psInstruction->uiOpcode)
	{
	case IRO_Slot:
		sResult = PointerTo(AddObject(IROB_Slot, (size)sLeft.ulliBits, true, psFrame->uiFrame, NULL), 0, iPointerBytes);
		break;
	case IRO_ArgAddress:
		ulliIndex = sLeft.ulliBits;
		if (ulliIndex >= psFrame->auiOffsets.NumElements())
		{
			return Fail("It takes the address of an argument it wasn't given");
		}
		sResult = PointerTo(psFrame->uiArguments, *psFrame->auiOffsets.Get((size)ulliIndex), iPointerBytes);
		break;
	case IRO_PtrAdd:
		uiObject = ObjectOf(&sLeft, iPointerBytes);
		if (((uiObject == 0) && sLeft.bPointer) || sRight.bPointer)
		{
			return Fail("It uses a pointer as a number");
		}
		sResult = PointerTo(uiObject, sLeft.ulliBits + (uint64)IRSignExtend(pcFunction->TypeOf(psInstruction->asArguments[1]), sRight.ulliBits), iPointerBytes);
		break;
	case IRO_SpaceCast:
		//The same place, in a pointer of another width: its object's again.
		uiObject = ObjectOf(&sLeft, pcModule->GetPointerSize(pcFunction->TypeOf(psInstruction->asArguments[0]).uiSpace));
		if ((uiObject == 0) && sLeft.bPointer)
		{
			return Fail("It uses a pointer as a number");
		}
		sResult = PointerTo(uiObject, sLeft.ulliBits, iPointerBytes);
		break;
	case IRO_Load:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return Fail("It reads a volatile variable");
		}
		if (!Load(pcModule, sType, &sLeft, pcModule->GetPointerSize(pcFunction->TypeOf(psInstruction->asArguments[0]).uiSpace), &sResult))
		{
			return false;
		}
		break;
	case IRO_Store:
		if (psInstruction->uiFlags & IRF_Volatile)
		{
			return Fail("It writes a volatile variable");
		}
		return Store(pcModule, sType, &sLeft, &sRight, pcModule->GetPointerSize(pcFunction->TypeOf(psInstruction->asArguments[1]).uiSpace));
	case IRO_Arg:
		psFrame->asPending.Add(&sLeft);
		psFrame->asPendingTypes.Add(&sType);
		return true;
	case IRO_Thrown:
		//A callee that threw has failed already (RunFrame): here the call returned.
		sResult.Set(0);
		break;
	default:
		if (sType.IsFloat() || pcFunction->TypeOf(psInstruction->asArguments[0]).IsFloat())
		{
			if (!Float(pcFunction, psInstruction, &sLeft, &sRight, &sResult))
			{
				return false;
			}
		}
		else if (!Integer(pcModule, pcFunction, psInstruction, &sLeft, &sRight, &sResult))
		{
			return false;
		}
		break;
	}
	if (psInstruction->sResult.IsTemporary())
	{
		pasTemporaries[psInstruction->sResult.Index()] = sResult;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call that has returned: its slots and its arguments are gone, and a
// pointer to one of them is to nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::EndFrame(size uiFrame, uint32 uiFirstObject)
{
	SIRObject*	psObject;
	size		i;

	for (i = uiFirstObject; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		if (((psObject->uiKind == IROB_Slot) || (psObject->uiKind == IROB_Arguments)) && (psObject->uiFrame == uiFrame))
		{
			psObject->bDead = true;
			FreeObject(psObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a function with its arguments, until it returns.  Each call it
// makes is a frame, run until it returns, when its caller goes on: none
// takes the C stack.  A call the host makes while one is being run (to
// work out another constant) is run above the frames there were.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Run(CIRModule* pcModule, CIRFunction* pcFunction, CArrayIRValues* pasArguments, CArrayTemplate<SIRType>* pasArgumentTypes, SIRValue* psResult)
{
	SIRFrame*		psBase;
	SIRFrame*		psFrame;
	SIRFrame*		psCaller;
	SIRInstruction*	psCall;
	SIRValue		sValue;
	bool			bResult;

	psResult->Set(0);
	psBase = mpsFrame;
	if (!PushFrame(pcModule, pcFunction, pasArguments, pasArgumentTypes))
	{
		return false;
	}

	bResult = true;
	while (bResult && (mpsFrame != psBase))
	{
		psFrame = mpsFrame;
		bResult = RunFrame(psFrame);
		if (bResult && !psFrame->pcBlock)
		{
			//It has returned: its caller has its value and goes on after the call.
			sValue = psFrame->sResult;
			PopFrame(false);
			psCaller = mpsFrame;
			if (psCaller == psBase)
			{
				*psResult = sValue;
			}
			else
			{
				psCall = psCaller->pcBlock->masInstructions.Get(psCaller->uiInstruction);
				if (psCall->sResult.IsTemporary())
				{
					psCaller->pasTemporaries[psCall->sResult.Index()] = sValue;
				}
				psCaller->uiInstruction++;
			}
		}
	}

	//What wasn't a constant is the innermost call's instruction it was in, or its call's.
	while (mpsFrame != psBase)
	{
		psFrame = mpsFrame;
		if ((muiErrorSource == 0) && psFrame->pcBlock && (psFrame->uiInstruction < psFrame->pcBlock->masInstructions.NumElements()))
		{
			muiErrorSource = psFrame->pcBlock->masInstructions.Get(psFrame->uiInstruction)->uiSource;
		}
		PopFrame(true);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A call's frame, the innermost: its arguments where the caller put them,
// each its own size and the first lowest (a parameter is also its value),
// at its function's first block.  False if it can't be called, or the
// arguments or the first block's phis aren't values; it is gone then.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::PushFrame(CIRModule* pcModule, CIRFunction* pcFunction, CArrayIRValues* pasArguments, CArrayTemplate<SIRType>* pasArgumentTypes)
{
	SIRFrame*	psFrame;
	SIRValue	sAddress;
	size		uiOffset;
	size		uiMaxPhis;
	size		i;
	bool		bResult;

	if (pasArguments->NumElements() < pcFunction->masParameters.NumElements())
	{
		return Fail("It is called with too few arguments");
	}
	if (muiDepth >= muiDepthLimit)
	{
		return Fail("Its calls go too deep to be worked out");
	}

	psFrame = (SIRFrame*)malloc(sizeof(SIRFrame));
	psFrame->psCaller = mpsFrame;
	psFrame->pcModule = pcModule;
	psFrame->pcFunction = pcFunction;
	psFrame->pcBlock = NULL;
	psFrame->uiInstruction = 0;
	psFrame->sResult.Set(0);
	psFrame->asPending.Init();
	psFrame->asPendingTypes.Init();
	muiDepth++;
	psFrame->uiFrame = ++muiFrames;
	psFrame->uiFirstObject = (uint32)masObjects.NumElements();
	mpsFrame = psFrame;

	psFrame->auiOffsets.Init();
	uiOffset = 0;
	for (i = 0; i < pasArguments->NumElements(); i++)
	{
		psFrame->auiOffsets.Add(&uiOffset);
		uiOffset += ValueBytes(pcModule, *pasArgumentTypes->Get(i));
	}
	psFrame->uiArguments = AddObject(IROB_Arguments, uiOffset, true, psFrame->uiFrame, NULL);
	bResult = true;
	for (i = 0; bResult && (i < pasArguments->NumElements()); i++)
	{
		sAddress = PointerTo(psFrame->uiArguments, *psFrame->auiOffsets.Get(i), pcModule->GetPointerSize(0));
		bResult = Store(pcModule, *pasArgumentTypes->Get(i), pasArguments->Get(i), &sAddress, pcModule->GetPointerSize(0));
	}

	uiMaxPhis = 0;
	for (i = 0; i < pcFunction->NumBlocks(); i++)
	{
		if (pcFunction->GetBlock(i)->masPhis.NumElements() > uiMaxPhis)
		{
			uiMaxPhis = pcFunction->GetBlock(i)->masPhis.NumElements();
		}
	}
	psFrame->pasTemporaries = (SIRValue*)calloc(pcFunction->NumTemporaries() + 1, sizeof(SIRValue));
	psFrame->pasPhis = (SIRValue*)calloc(uiMaxPhis + 1, sizeof(SIRValue));
	for (i = 0; i < pcFunction->masParameters.NumElements(); i++)
	{
		psFrame->pasTemporaries[pcFunction->masParameters.Get(i)->Index()] = *pasArguments->Get(i);
	}

	bResult = bResult && EnterBlock(psFrame, pcFunction->GetEntry());
	if (!bResult)
	{
		PopFrame(true);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The innermost call has returned, or failed (then it is in the trail of
// functions what was run failed in): its slots and its arguments are gone,
// and its caller is the innermost.
//
//////////////////////////////////////////////////////////////////////////
void CIRInterpreter::PopFrame(bool bFailed)
{
	SIRFrame*	psFrame;

	psFrame = mpsFrame;
	psFrame->asPending.Kill();
	psFrame->asPendingTypes.Kill();
	psFrame->auiOffsets.Kill();
	free(psFrame->pasTemporaries);
	free(psFrame->pasPhis);
	EndFrame(psFrame->uiFrame, psFrame->uiFirstObject);
	muiDepth--;
	if (bFailed)
	{
		maszTrail.Add(psFrame->pcModule->GetSymbolName(psFrame->pcFunction->muiSymbol));
	}
	mpsFrame = psFrame->psCaller;
	free(psFrame);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a call from where it is: its block's instructions and jump, and the
// blocks after, until it returns (pcBlock is NULL) or makes a call (the
// callee's frame is the innermost; the caller is at the call).  False if
// what it does isn't a value.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::RunFrame(SIRFrame* psFrame)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRValue		sValue;
	bool			bResult;

	while (psFrame->pcBlock)
	{
		pcBlock = psFrame->pcBlock;
		while (psFrame->uiInstruction < pcBlock->masInstructions.NumElements())
		{
			psInstruction = pcBlock->masInstructions.Get(psFrame->uiInstruction);
			if (!CountStep(psInstruction->uiFlags & IRF_FullExpression))
			{
				return false;
			}
			if (psInstruction->uiOpcode == IRO_Call)
			{
				return CallInstruction(psFrame, psInstruction);
			}
			if (!Step(psFrame, psInstruction))
			{
				return false;
			}
			psFrame->uiInstruction++;
		}

		switch (pcBlock->msJump.uiKind)
		{
		case IRJ_Jump:
			bResult = EnterBlock(psFrame, pcBlock->msJump.pcThen);
			break;
		case IRJ_Branch:
			bResult = ValueOf(psFrame->pcModule, psFrame->pcFunction, psFrame->pasTemporaries, pcBlock->msJump.sArgument, &sValue);
			bResult = bResult && EnterBlock(psFrame, (sValue.ulliBits & 1) ? pcBlock->msJump.pcThen : pcBlock->msJump.pcElse);
			break;
		case IRJ_Return:
			bResult = true;
			if (!pcBlock->msJump.sArgument.IsNone())
			{
				bResult = ValueOf(psFrame->pcModule, psFrame->pcFunction, psFrame->pasTemporaries, pcBlock->msJump.sArgument, &psFrame->sResult);
			}
			psFrame->pcBlock = NULL;
			break;
		case IRJ_Throw:
			bResult = Fail("It throws an exception");
			if (muiErrorSource == 0)
			{
				muiErrorSource = pcBlock->msJump.uiSource;
			}
			break;
		default:
			bResult = Fail("It reaches where the program can't go on");
			if (muiErrorSource == 0)
			{
				muiErrorSource = pcBlock->msJump.uiSource;
			}
			break;
		}
		if (!bResult || !CountStep(false))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An instruction or a jump is run: a step too if it ends a full-expression
// (bFullExpression).  False, with why, once either limit is passed.
// [implimits]
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::CountStep(bool bFullExpression)
{
	if ((bFullExpression && (++muiSteps > muiStepLimit)) || (++muiInstructions > muiInstructionLimit))
	{
		return Fail("It takes too long to work out");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A call goes to a block (from the one it is in, NULL at its function's
// first): its phis, all read before any is written, then its first
// instruction.  A jump to no block returns.  False if a phi isn't a value;
// the call stays where it was.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::EnterBlock(SIRFrame* psFrame, CIRBlock* pcBlock)
{
	SIRPhi*			psPhi;
	SIRPhiArgument*	psPhiArgument;
	CIRBlock*		pcPrevious;
	size			i;

	pcPrevious = psFrame->pcBlock;
	if (!pcBlock)
	{
		psFrame->pcBlock = NULL;
		return true;
	}
	for (i = 0; i < pcBlock->masPhis.NumElements(); i++)
	{
		psPhi = pcBlock->masPhis.Get(i);
		psPhiArgument = pcPrevious ? pcBlock->FindPhiArgument(psPhi, pcPrevious) : NULL;
		if (!psPhiArgument)
		{
			return Fail("It is not IR that can be run");
		}
		if (!ValueOf(psFrame->pcModule, psFrame->pcFunction, psFrame->pasTemporaries, psPhiArgument->sValue, &psFrame->pasPhis[i]))
		{
			return false;
		}
	}
	for (i = 0; i < pcBlock->masPhis.NumElements(); i++)
	{
		psFrame->pasTemporaries[pcBlock->masPhis.Get(i)->sResult.Index()] = psFrame->pasPhis[i];
	}
	psFrame->pcBlock = pcBlock;
	psFrame->uiInstruction = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value a function returns for the arguments (each of its parameter's
// type), if it can be worked out without running the program; else false,
// and GetError says why and GetErrorSource where.  It may be called while
// a call is being run (the host making an object a symbol is): that call
// goes on afterwards as it was, with what is left of the steps.
//
//////////////////////////////////////////////////////////////////////////
bool CIRInterpreter::Call(CIRModule* pcModule, CIRFunction* pcFunction, SIRValue* pasArguments, size uiNumArguments, SIRValue* psResult)
{
	CArrayIRValues			asArguments;
	CArrayTemplate<SIRType>	asTypes;
	CChars					szOuterError;
	CArrayChars				aszOuterTrail;
	SIRType					sType;
	uint32					uiOuterSource;
	size					i;
	bool					bResult;

	szOuterError.Init(mszError.Text());
	uiOuterSource = muiErrorSource;
	mszError.Clear();
	muiErrorSource = 0;
	aszOuterTrail.Init();
	aszOuterTrail.Copy(&maszTrail);
	maszTrail.ReInit();
	if (muiDepth == 0)
	{
		muiSteps = 0;
		muiInstructions = 0;
	}

	bResult = uiNumArguments == pcFunction->masParameters.NumElements();
	if (!bResult)
	{
		Fail("It is called with the wrong number of arguments");
	}
	asArguments.Init();
	asTypes.Init();
	for (i = 0; bResult && (i < uiNumArguments); i++)
	{
		asArguments.Add(&pasArguments[i]);
		sType = pcFunction->TypeOf(*pcFunction->masParameters.Get(i));
		asTypes.Add(&sType);
	}
	bResult = bResult && Run(pcModule, pcFunction, &asArguments, &asTypes, psResult);
	asArguments.Kill();
	asTypes.Kill();

	mszLastError.Clear();
	mszLastError.Append(mszError.Text());
	muiLastErrorSource = muiErrorSource;
	maszLastTrail.ReInit();
	maszLastTrail.Copy(&maszTrail);
	mszError.Clear();
	mszError.Append(szOuterError.Text());
	muiErrorSource = uiOuterSource;
	maszTrail.ReInit();
	maszTrail.Copy(&aszOuterTrail);
	aszOuterTrail.Kill();
	szOuterError.Kill();
	return bResult;
}
