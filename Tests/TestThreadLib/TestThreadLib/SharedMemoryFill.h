#ifndef __SHARED_MEMORY_FILL_H__
#define __SHARED_MEMORY_FILL_H__


class CSharedMemoryFill
{
public:
	char*	mszSharedMemoryName;
	char*	mszMutexName;
	char*	mszFillChar;
	int		miResult;
	int		miChunkSize;

	void Init(char* szSharedMemoryName, char* szMutexName, char* szFillChar, int iChunkSize);
	void Run(void);
};


#endif // !__SHARED_MEMORY_FILL_H__


