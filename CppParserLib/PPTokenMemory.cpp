#include "BaseLib/Numbers.h"
#include "PPTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Init(void)
{
	mcStack.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Kill(void)
{
	//SPPTokenIterator	sIter;
	//CPPToken*			pcPPToken;

	//pcPPToken = StartIteration(&sIter);
	//while (pcPPToken)
	//{
	//	pcPPToken->Kill();
	//	pcPPToken = Iterate(&sIter);
	//}

	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CPPTokenMemory::AddBlock(void)
{
	CPPBlock* pcToken;
		
	pcToken = (CPPBlock*)mcStack.Add(sizeof(CPPBlock));
	CPPBlock::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPWhitespace* CPPTokenMemory::AddWhitespace(void)
{
	CPPWhitespace* pcToken;

	pcToken = (CPPWhitespace*)mcStack.Add(sizeof(CPPWhitespace));
	CPPWhitespace::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPHashes* CPPTokenMemory::AddHashes(void)
{
	CPPHashes* pcToken;

	pcToken = (CPPHashes*)mcStack.Add(sizeof(CPPHashes));
	CPPHashes::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPReplacement* CPPTokenMemory::AddReplacement(void)
{
	CPPReplacement* pcToken;

	pcToken = (CPPReplacement*)mcStack.Add(sizeof(CPPReplacement));
	CPPReplacement::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPText* CPPTokenMemory::AddText(void)
{
	CPPText* pcToken;

	pcToken = (CPPText*)mcStack.Add(sizeof(CPPText));
	CPPText::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTextWithSource* CPPTokenMemory::AddTextWithSource(void)
{
	CPPTextWithSource* pcToken;

	pcToken = (CPPTextWithSource*)mcStack.Add(sizeof(CPPTextWithSource));
	CPPTextWithSource::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenReplacementsHolder* CPPTokenMemory::AddTokenReplacementsHolder(void)
{
	CPPTokenReplacementsHolder* pcToken;

	pcToken = (CPPTokenReplacementsHolder*)mcStack.Add(sizeof(CPPTokenReplacementsHolder));
	CPPTokenReplacementsHolder::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPLine* CPPTokenMemory::AddLine(void)
{
	CPPLine* pcToken;

	pcToken = (CPPLine*)mcStack.Add(sizeof(CPPLine));
	CPPLine::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPPTokenMemory::AddDirective(void)
{
	CPPDirective* pcToken;

	pcToken = (CPPDirective*)mcStack.Add(sizeof(CPPDirective));
	CPPDirective::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPConditional* CPPTokenMemory::AddConditional(void)
{
	CPPConditional* pcToken;

	pcToken = (CPPConditional*)mcStack.Add(sizeof(CPPConditional));
	CPPConditional::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPInclude* CPPTokenMemory::AddInclude(void)
{
	CPPInclude* pcToken;

	pcToken = (CPPInclude*)mcStack.Add(sizeof(CPPInclude));
	CPPInclude::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Mark(CStackMarkExtended* psMark)
{
	mcStack.Mark(psMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Rollback(CStackMarkExtended* psMark)
{
	mcStack.Rollback(psMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenMemory::StartIteration(SPPTokenIterator* psIter)
{
	CPPToken*		pcPPToken;
	int				iNumStacks;
	CMemoryStack*	pcStack;
	int				iNumElements;

	iNumStacks = mcStack.NumStacks();
	psIter->iStack = 0;
	psIter->iElement = 0;

	for (; psIter->iStack < iNumStacks; )
	{
		pcStack = mcStack.GetStack(psIter->iStack);
		psIter->pvStackData = pcStack->GetData();
		iNumElements = pcStack->NumElements();
		if (psIter->iElement < iNumElements)
		{
			pcPPToken = (CPPToken*)psIter->pvStackData;
			psIter->pvStackData = RemapSinglePointer(psIter->pvStackData, pcPPToken->ClassSize());
			psIter->iElement++;
			return pcPPToken;
		}
		psIter->iStack++;
		psIter->iElement = 0;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenMemory::Iterate(SPPTokenIterator* psIter)
{
	CPPToken*		pcPPToken;
	CMemoryStack*	pcStack;
	int				iNumElements;

	for (;;)
	{
		pcStack = mcStack.GetStack(psIter->iStack);
		iNumElements = pcStack->NumElements();
		for (; psIter->iElement < iNumElements; )
		{
			pcPPToken = (CPPToken*)psIter->pvStackData;
			psIter->pvStackData = RemapSinglePointer(psIter->pvStackData, pcPPToken->ClassSize());
			psIter->iElement++;
			return pcPPToken;
		}
		psIter->iStack++;
		psIter->iElement = 0;
		pcStack = mcStack.GetStack(psIter->iStack);
		if (pcStack)
		{
			psIter->pvStackData = pcStack->GetData();
		}
		else
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CPPTokenMemory::Size(void)
{
	return mcStack.GetUsedMemory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenMemory::Print(CChars* psz)
{
	SPPTokenIterator	sIter;
	CPPToken*			pcPPToken;

	pcPPToken = StartIteration(&sIter);
	while (pcPPToken)
	{
		pcPPToken->Print(psz);
		pcPPToken = Iterate(&sIter);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenMemory::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

