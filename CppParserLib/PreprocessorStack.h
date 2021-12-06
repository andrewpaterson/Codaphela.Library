#ifndef __PREPROCESSOR_STACK_H__
#define __PREPROCESSOR_STACK_H__
#include "BaseLib/ArrayTemplate.h"


class CPPTokenList;
class CPPTokenMemory;
struct SPreprocessorStackItem
{
	CPPTokenList*		mpcProcessedTokens;
	CPPTokenMemory*		mpcTokenMemory;

	void	Init(CPPTokenList* pcProcessedTokens, CPPTokenMemory* pcTokenMemory);
};



typedef CArrayTemplate<SPreprocessorStackItem>	CArrayPreprocessorStackItems;

class CPreprocessorStack
{
protected:
	CArrayPreprocessorStackItems	macStack;

public:
	void			Init(void);
	void			Kill(void);

	void			Push(CPPTokenList* pcProcessedTokens, CPPTokenMemory* pcTokenMemory);
	void			Pop(void);

	CPPTokenMemory* GetTokenMemory(void);
	CPPTokenList*	GetTokenList(void);
};

#endif // !__PREPROCESSOR_STACK_H__

