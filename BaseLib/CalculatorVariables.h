#ifndef __CALCULATOR_VARIABLES_H__
#define __CALCULATOR_VARIABLES_H__
#include "MapStringTemplate.h"
#include "CalcObjectArray.h"
#include "CalcVariableDefinition.h"


typedef CMapStringTemplate<CCalcVariableDefinition*> CCalcVariablesMap;


class CCalculatorVariables
{
protected:
	CCalcObjectArray	mapcExpressionDefintions;
	CCalcVariablesMap	mmszpcVariables;

public:
	void						Init(void);
	void						Kill(void);
	void						ReInit(void);

	void						Add(CCalcVariableDefinition* pcVariableDefinition);
	void						Add(CCalcExpression* pcExpression);

	CCalcVariableDefinition*	Get(char* szName);
	CCalcExpression*			GetExpression(size uiIndex);
	size						NumExpressions(void);

	void						Print(CChars* psz);
};


#endif // __CALCULATOR_VARIABLES_H__

