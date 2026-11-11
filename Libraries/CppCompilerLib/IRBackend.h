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
#ifndef __I_R_BACKEND_H__
#define __I_R_BACKEND_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "CPPErrors.h"
#include "IRModule.h"
#include "IRSourceMarkers.h"


//A phi's copy on a jump (ParallelCopy): the phi, the value it takes, its type, and whether it reads the saved value.
struct SIRCopy
{
	SIRReference	sDestination;
	SIRReference	sSource;		//IRNone once written.
	SIRType			sType;
	bool			bSaved;			//The source was saved to break a cycle: the copy reads the saved value.
};
typedef CArrayTemplate<SIRCopy>	CArrayIRCopies;


//A step of a parallel copy: a copy written (sDestination from sSource, or from the saved value), or a source saved.
struct SIRCopyStep
{
	bool			bSave;			//Save sSource (to break a cycle), else write the copy.
	SIRReference	sDestination;
	SIRReference	sSource;
	SIRType			sType;
	bool			bSaved;
};
typedef CArrayTemplate<SIRCopyStep>	CArrayIRCopySteps;


//The most signed digits (a multiplier's non-adjacent form, SignedDigits) a multiply by a constant is written with
//as shifts and adds (ExpandMultiplies).
#define IR_MAX_MULTIPLY_DIGITS	8


//What the 65C64 (CIR65Backend) and W65C816 (CIR816Backend) back ends share: the module and the label counter, the
//function being written and the text it goes to, its source markers, errors and block labels, the runtime routines
//its code calls, the order of the phis' copies on a jump (ParallelCopy), multiplies by constants as shifts and adds
//when the target says they cost less (ExpandMultiplies), and writing a module as sections (each function and global a section of its own), its globals'
//data, labels and errors.
class CIRBackend
{
protected:
	CIRModule*				mpcModule;
	int*					mpiLabels;
	CIRFunction*			mpcFunction;
	CChars*					mpszOut;
	uint32					muiSource;			//The source token + 1 of the last marker written (Source).
	bool					mbSourceMarkers;	//Source writes markers (SetSourceMarkers), for a listing.
	CIRSourceMarkers		mcSourceMarkers;	//The markers while the text passes run.
	CCPPErrors*				mpcErrors;
	bool					mbResult;
	CArrayTemplate<int32>	maiBlockLabels;		//Per block index.
	CArrayChars				maszRuntime;		//The runtime routines' labels the code calls (UseRuntime), once each.

public:
	virtual bool	GenerateFunction(CIRFunction* pcFunction, const char* szLabel, CChars* pszDest, CCPPErrors* pcErrors) = 0;

	bool			GenerateModule(CChars* pszDest, CCPPErrors* pcErrors);
	void			GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest);
	void			SetSourceMarkers(bool bSourceMarkers);
	CArrayChars*	GetRuntime(void);

	static void		AppendSoftFloatName(CChars* psz, SIRInstruction* psInstruction, SIRType sFrom);
	static int		FloatCompareValues(uint16 uiOpcode, int* aiValues, bool* pbNot);
	static bool		SameOperands(SIRInstruction* psFirst, SIRInstruction* psSecond);
	static int		SignedDigits(uint64 ulliMultiplier, int iBits, int* aiPlaces, int* aiSigns);

protected:
	void			Init(CIRModule* pcModule, int* piLabels, CMallocator* pcMallocator = NULL);
	void			Kill(void);

	void			Binding(const char* szName, uint32 uiAttributes, CChars* pszDest);
	bool			Error(const char* szMessage, const char* szDetail = NULL);
	void			Source(uint32 uiSource);
	int32			NewLabel(void);
	void			UseRuntime(const char* szName);
	SIRInstruction*	BranchComparison(CIRBlock* pcBlock);
	bool			ParallelCopy(CArrayIRCopies* pasCopies, CArrayIRCopySteps* pasSteps);
	void			ExpandMultiplies(void);
	SIRReference	InsertBefore(CIRBlock* pcBlock, size* puiIndex, uint16 uiOpcode, SIRType sType, SIRReference sFirst, SIRReference sSecond);

	virtual int		MultiplyCost(SIRType sType, uint64 ulliMultiplier) = 0;
	virtual int		ShiftAddCost(SIRType sType, int* aiPlaces, int* aiSigns, int iDigits) = 0;

	virtual bool	CopyOverlaps(SIRReference sSource, SIRReference sDestination) = 0;
	virtual bool	CopySamePlace(SIRReference sValue1, SIRReference sValue2) = 0;
	void			Label(int32 iLabel);
};


#endif // __I_R_BACKEND_H__

