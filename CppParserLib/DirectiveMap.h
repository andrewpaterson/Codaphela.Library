#ifndef __DIRECTIVE_MAP_H__
#define __DIRECTIVE_MAP_H__
#include "BaseLib/IndexPrimitiveTemplate.h"
#include "ASCIITree.h"
#include "PreprocessorDirective.h"


typedef CIndexPrimitiveTemplate<int64, CPreprocessorDirective>	CDirectivesIndex;


class CDirectiveMap
{
protected:
	CASCIITree			mcNameToIDIndex;
	CDirectivesIndex	mcIDToDirectiveIndex;

public:
	void 						Init(void);
	void 						Kill(void);
	CPreprocessorDirective*		AddDirective(CExternalString* pcName, EPreprocessorDirective eDirective);
	CPreprocessorDirective*		AddDirective(char* szName, EPreprocessorDirective eDirective);
	void						RemoveDirective(CExternalString* pcName);
	void						RemoveDirective(char* szName);
	CPreprocessorDirective*		GetDirective(CExternalString* pcName, BOOL bExact);
	CPreprocessorDirective*		GetDirective(CChars* pszName, BOOL bExact);
	CPreprocessorDirective*		GetDirective(char* szName, BOOL bExact);
	void						Dump(void);
};


#endif // !__DIRECTIVE_MAP_H__

