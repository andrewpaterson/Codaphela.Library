//What the semantic pass (CCPPSemanticPass, the IDE's check of a file as it is written) is known to get wrong of the
//conformance cases (TestCPPConformance.cpp, TestCPPConformanceChecked): for each group, a case's name and (M) the
//pass missed what is wrong with a program that must be refused, or (F) it found something wrong with a program that
//is right.  Take a case off its list as the pass does it; Conformance/lists.py -write writes the lists as they are.
//A group with no line here is expected to have nothing on its list.
struct SConformanceChecked
{
	const char*	szGroup;
	const char*	szNotDone;
};


static const SConformanceChecked gasConformanceChecked[] =
{
	{ "Works",
		"" },
	{ "Stage1",
		"" },
	{ "Stage2",
		"" },
	{ "Stage3",
		"" },
	{ "Stage4",
		"" },
	{ "Stage5",
		"" },
	{ "Stage6",
		"" },
	{ "Stage7",
		"" },
	{ "Stage8",
		"" },
	{ "Stage9",
		"" },
	{ "Stage10",
		"" },
	{ "Exceptions",
		"GotoIntoTryRefused(M) GotoIntoHandlerRefused(M) SwitchIntoTryRefused(M) ConstructorFunctionTryReturnRefused(M) ThrowPrivateCopyRefused(M) " },
	{ "Clause2",
		"" },
	{ "Clause3",
		"" },
	{ "Clause4",
		"" },
	{ "Clause5",
		"MemberAccessNestedTypeRefused(M) UnaryAddressOfUnqualifiedMemberRefused(M) MultiplicativeConstantDivideByZeroRefused(M) ConstantRunTimeConstRefused(M) ConstantNonConstVariableRefused(M) ConstantConstDoubleRefused(M) ConstantVolatileRefused(M) ConstantUnevaluatedBranchTakenRefused(M) ConstantLambdaRefused(M) ConstantReinterpretCastRefused(M) ConstantModifiesGlobalRefused(M) ConstantAssignsGlobalRefused(M) ConstantNewRefused(M) ConstantNullDereferenceRefused(M) ConstantShiftTooFarRefused(M) ConstantNegativeShiftRefused(M) ConstantInactiveUnionMemberRefused(M) ConstantUnrelatedPointerCompareRefused(M) ConstantThisOutsideRefused(M) ConstantAddressOfLocalRefused(M) ConstantReferenceToLocalRefused(M) ConstantAddressAsNumberRefused(M) " },
	{ "Clause6",
		"" },
	{ "Clause7",
		"DclConstexprStaticMemberNeverInitialisedRefused(M) " },
	{ "Clause8",
		"DclReturnCopyRepeatedBaseRefused(M) " },
	{ "Clause9",
		"ClassNestedNoOuterThisRefused(M) " },
	{ "Clause10",
		"" },
	{ "Clause11",
		"" },
	{ "Clause12",
		"SpecialDefaultConstructorMemberRefused(M) " },
	{ "Clause13",
		"" },
	{ "Clause14",
		"TempNonTypeNotLvalueRefused(M) TempVariadicLengthMismatchRefused(M) TempVariadicUnexpandedRefused(M) TempResDependentOrdinaryLookupRefused(M) TempInstMemberUsedRefused(M) TempSfinaeBodyErrorRefused(M) TempVariadicNestedInParameters(F) " },
	{ "Clause16",
		"" },
};
