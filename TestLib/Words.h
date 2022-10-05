#ifndef __WORDS_H__
#define __WORDS_H__
#include "BaseLib/ArrayChars.h"


extern CArrayChars gaszArchaicWords;
extern CArrayChars gaszArchaicDefinitions;
extern CArrayChars gaszCommonWords;


class CRandom;


void GetArchaicDefinitions(CArrayChars* pasz);
void GetArchaicWords(CArrayChars* pasz);
void GetCommonWords(CArrayChars* pasz);

void PrintRandomWords(CChars* psz, CRandom* pcRandom, bool bIncludeArchaic, int iWordCount);

void WordsInit(void);
void WordsKill(void);


#endif // !__WORDS_H__

