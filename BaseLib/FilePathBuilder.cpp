#include "FileUtil.h"
#include "FilePathBuilder.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CFilePathBuilder::Build(char* szElements, ...)
{
	va_list		vaMarker;
	char*		szValue;
	CFileUtil	cFileUtil;

	szPath.Init();
	if (szElements != NULL)
	{
		cFileUtil.AppendToPath(&szPath, szElements);

		va_start(vaMarker, szElements);
		szValue = va_arg(vaMarker, char*);
		while (szValue != NULL)
		{
			cFileUtil.AppendToPath(&szPath, szValue);
			szValue = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);
	} 
	return szPath.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFilePathBuilder::Kill(void)
{
	szPath.Kill();
}

