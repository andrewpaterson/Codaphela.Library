#include "StandardLib/Unknowns.h"


class CTestUnknown : public CUnknown
{
CONSTRUCTABLE(CTestUnknown)
public:

	int32	miCount;

	void	Init(void);
	void	Init(int iCount);
	void	Kill(void);
	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
	void	Print(CChars* psz);
};


class CTestNamedUnknown : public CUnknown
{
CONSTRUCTABLE(CTestNamedUnknown)
public:
	CChars	mszName;

	void	Init(const char* szName);
	void	Kill(void);
	char*	GetName(void);
	bool	Iterable(void);
	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
	void	Print(CChars* psz);
};


class CTestIterableUnknown : public CTestUnknown
{
CONSTRUCTABLE(CTestIterableUnknown)
public:
	int	miID;

	void	Init(int iID);
	bool	Iterable(void);
	const char*	GetClassName(void);
	void	Print(CChars* psz);
};


class CTestUnknownJobbie : public CUnknown
{
CONSTRUCTABLE(CTestUnknownJobbie)
public:
	int32	miANumber;
	CChars	mszText;

	void 	Init(int32 iNumber, const char* pszText);
	void 	Kill(void);
	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
	void	Print(CChars* psz);
};

