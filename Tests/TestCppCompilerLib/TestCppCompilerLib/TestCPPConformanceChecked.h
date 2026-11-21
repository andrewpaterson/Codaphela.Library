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
		"StaticAssertFalseInFunction(M) " },
	{ "Stage2",
		"" },
	{ "Stage3",
		"" },
	{ "Stage4",
		"" },
	{ "Stage5",
		"" },
	{ "Stage6",
		"MemberClassTemplate(F) " },
	{ "Stage7",
		"" },
	{ "Stage8",
		"" },
	{ "Stage9",
		"VirtualBaseStaticCastRefused(M) " },
	{ "Stage10",
		"NamespaceInBlockScope(M) " },
	{ "Exceptions",
		"GotoIntoTryRefused(M) GotoIntoHandlerRefused(M) SwitchIntoTryRefused(M) ConstructorFunctionTryReturnRefused(M) ThrowPrivateCopyRefused(M) " },
	{ "Clause3",
		"BasicSwitchFloatConditionRefused(M) BasicGotoPastDeclarationRefused(M) BasicGotoIntoBlockRefused(M) BasicIncompleteArraySizeofRefused(M) BasicSizeofVoidRefused(M) " },
	{ "Clause4",
		"ConvContextualSwitchTwoRefused(M) ConvContextualSwitchExplicitRefused(M) " },
	{ "Clause5",
		"LambdaInUnevaluatedRefused(M) LambdaNestedCaptures(F) LambdaNotAssignableRefused(M) MemberAccessNestedTypeRefused(M) PostIncrementVoidPointerRefused(M) DynamicCastNotPolymorphicRefused(M) DynamicCastIncompleteRefused(M) DynamicCastToNonClassRefused(M) UnaryAddressOfBitFieldRefused(M) UnaryAddressOfUnqualifiedMemberRefused(M) PreIncrementFunctionPointerTargetRefused(M) SizeofFunctionRefused(M) SizeofIncompleteRefused(M) SizeofIncompleteArrayRefused(M) SizeofVoidRefused(M) NewVoidRefused(M) AlignofIncompleteRefused(M) AlignofVoidRefused(M) AlignofFunctionRefused(M) CastNotationMemberPointerToPointerRefused(M) MultiplicativeConstantDivideByZeroRefused(M) AdditiveVoidPointerRefused(M) AdditiveFunctionPointerRefused(M) AssignConstMemberClassRefused(M) ConstantRunTimeConstRefused(M) ConstantNonConstVariableRefused(M) ConstantConstDoubleRefused(M) ConstantVolatileRefused(M) ConstantCaseNotConstantRefused(M) ConstantCaseNarrowingRefused(M) ConstantUnevaluatedBranchTakenRefused(M) ConstantLambdaRefused(M) ConstantReinterpretCastRefused(M) ConstantModifiesGlobalRefused(M) ConstantAssignsGlobalRefused(M) ConstantNewRefused(M) ConstantNullDereferenceRefused(M) ConstantShiftTooFarRefused(M) ConstantNegativeShiftRefused(M) ConstantInactiveUnionMemberRefused(M) ConstantUnrelatedPointerCompareRefused(M) ConstantThisOutsideRefused(M) ConstantAddressOfLocalRefused(M) ConstantReferenceToLocalRefused(M) ConditionalBitFieldAddressRefused(M) ConstantAddressAsNumberRefused(M) " },
	{ "Clause6",
		"StmtCaseOutsideSwitchRefused(M) StmtDefaultOutsideSwitchRefused(M) StmtConditionArrayRefused(M) StmtSwitchNonConstantCaseRefused(M) StmtBreakOutsideRefused(M) StmtContinueInSwitchRefused(M) StmtReturnNoValueRefused(M) " },
	{ "Clause7",
		"" },
	{ "Clause8",
		"DclArrayTooManyRefused(M) DclCharacterArrayWideMismatchRefused(M) DclWideArrayNarrowStringRefused(M) DclIntArrayFromStringRefused(M) DclReferenceToRvalueRefused(M) DclReferenceToBitFieldRefused(M) DclReferenceToCallValueRefused(M) DclListNarrowingInAggregateRefused(M) DclListTooManyForScalarRefused(M) DclReturnCopyRepeatedBaseRefused(M) " },
	{ "Clause9",
		"ClassBitFieldAddressRefused(M) ClassBitFieldReferenceRefused(M) ClassNestedNoOuterThisRefused(M) ClassLocalMemberDefinedOutsideRefused(M) " },
	{ "Clause10",
		"ClassMemberNameNotAmbiguous(F) DerivedVirtualBaseDowncastRefused(M) " },
	{ "Clause11",
		"" },
	{ "Clause12",
		"SpecialDefaultConstructorMemberRefused(M) SpecialAssignDeletedByReferenceRefused(M) " },
	{ "Clause13",
		"OverDclBlockDeclarationHidesRefused(M) OverFuncsStaticContextRefused(M) OverCallObjectSurrogate(F) OverCallObjectAmbiguousRefused(M) OverOperatorAmbiguousRefused(M) OverOperatorNoClassOperandRefused(M) OverCtorCopyInitialisationConverting(F) OverCopyInitConversionAmbiguousRefused(M) OverConvFunctionAmbiguousRefused(M) OverRankExactMatchCategory(F) OverRankPointerConversions(F) OverRankUserDefinedAmbiguousRefused(M) OverAddressByTarget(F) OverAddressTemplates(F) OverOperatorUnaryAndBinary(F) OverOperatorDefaultArgumentRefused(M) OverOperatorAssignmentHiddenRefused(M) OverOperatorArrowChain(F) OverOperatorIncrementLongRefused(M) OverLiteralBadParametersRefused(M) OverOperatorImplicitAssignmentByName(F) " },
	{ "Clause14",
		"TempMemberClassThroughInstance(F) TempDeclarationInBlockRefused(M) TempLinkageCRefused(M) TempNonTypeParameterKinds(F) TempNonTypeClassRefused(M) TempNonTypeNotLvalueRefused(M) TempDefaultBeforeNonDefaultClassRefused(M) TempDefaultRedefinedRefused(M) TempDefaultForPackRefused(M) TempPackNotLastClassRefused(M) TempNamesTemplateKeywordMissingRefused(M) TempNonTypeArguments(F) TempTemplateTemplateArguments(F) TempTemplateTemplateDefaultMismatchRefused(M) TempMemberTemplateVirtualRefused(M) TempVariadicLockstep(F) TempVariadicUnexpandedRefused(M) TempFriendSpecialisationOnlyRefused(M) TempPartialSpecialisationAmbiguousRefused(M) TempPartialSpecialisationDependentTypeRefused(M) TempPartialSpecialisationSameAsPrimaryRefused(M) TempPartialSpecialisationDefaultRefused(M) TempFunctionOrderingAmbiguousRefused(M) TempFunctionRedefinitionRefused(M) TempFunctionReturnTypeInSignature(F) TempAliasSelfReferenceRefused(M) TempResDependentBaseNotFoundRefused(M) TempResDependentOrdinaryLookupRefused(M) TempResNonDependentErrorRefused(M) TempInstMemberUsedRefused(M) TempExplicitInstantiation(F) TempExplicitInstantiationTwiceRefused(M) TempExplicitSpecialisationAfterUseRefused(M) TempExplicitSpecialisationOtherNamespaceRefused(M) TempExplicitArgumentsTooManyRefused(M) TempDeductionConflictRefused(M) TempDeductionFromTypes(F) TempSfinae(F) TempSfinaeBodyErrorRefused(M) " },
};
