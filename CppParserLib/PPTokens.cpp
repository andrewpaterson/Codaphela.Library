#include "BaseLib/Numbers.h"
#include "PPTokens.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokens::Init(void)
{
	mcStack.Init(4 KB);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokens::Kill(void)
{
	int				iStack;
	CPPToken*		pcPPToken;
	int				iNumStacks;
	CMemoryStack*	pcStack;
	int				iElement;
	int				iNumElements;
	void*			pvStackData;

	iNumStacks = mcStack.NumStacks();
	for (iStack = 0; iStack < iNumStacks; iStack++)
	{
		pcStack = mcStack.GetStack(iStack);
		pvStackData = pcStack->GetData();
		iNumElements = pcStack->NumElements();
		for (iElement = 0; iElement < iNumElements; iElement++)
		{
			pcPPToken = (CPPToken*)pvStackData;
			pcPPToken->Kill();

			pvStackData = RemapSinglePointer(pvStackData, pcPPToken->Sizeof());
		}
	}

	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPBlock* CPPTokens::AddBlock(void)
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
CPPWhiteSpace* CPPTokens::AddWhiteSpace(void)
{
	CPPWhiteSpace* pcToken;

	pcToken = (CPPWhiteSpace*)mcStack.Add(sizeof(CPPWhiteSpace));
	CPPWhiteSpace::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPHashes* CPPTokens::AddHashes(void)
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
CPPReplacement* CPPTokens::AddReplacement(void)
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
CPPText* CPPTokens::AddText(void)
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
CPPTextWithSource* CPPTokens::AddTextWithSource(void)
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
CPPHolder* CPPTokens::AddHolder(void)
{
	CPPHolder* pcToken;

	pcToken = (CPPHolder*)mcStack.Add(sizeof(CPPHolder));
	CPPHolder::Construct(pcToken);
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPLine* CPPTokens::AddLine(void)
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
CPPDirective* CPPTokens::AddDirective(void)
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
CPPConditional* CPPTokens::AddConditional(void)
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
CPPInclude* CPPTokens::AddInclude(void)
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
void CPPTokens::Mark(CStackMarkExtended* psMark)
{
	mcStack.Mark(psMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokens::Rollback(CStackMarkExtended* psMark)
{
	mcStack.Rollback(psMark);
}

