#include "InstructionFactory.h"


CInstructionFactory	gcInstructionFactory;


CInstructionFactory* CInstructionFactory::GetInstance(void)
{
	//Initialise me.
	return &gcInstructionFactory;
}

 