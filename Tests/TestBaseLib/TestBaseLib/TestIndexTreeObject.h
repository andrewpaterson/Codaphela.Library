#ifndef __TEST_INDEX_TREE_OBJECT_H__
#define __TEST_INDEX_TREE_OBJECT_H__


class CTestIndexTreeObject
{
public:
	char mszName[1024];

	void	Init(const char* szName);
	char*	GetName(void);
};


class CTestIndexTreeObject2
{
public:
	char mszName[128];

	void	Init(const char* szName);
	char* GetName(void);
	int		NameLength(void);
};


#endif // __TEST_INDEX_TREE_OBJECT_H__

