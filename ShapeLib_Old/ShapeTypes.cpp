#include "ShapeTypes.h"
#include "CoreLib\TypeDescriptor.h"
#include "ConvexHull.h"
#include "Box.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ShapeTypesInit(void)
{
	CStructDescriptorExtended*	pcStruct;
	CAdditionalInfomation		cAddInfo;
	int							aiVariable[5];
	void*						pvFuncMem;
	int							iPlaneID;
	int							iConvexHullID;

	//CPlane.
	gcTypeRegistrar.mcTypeStorage.AddStructType("CPlaneImm", &pcStruct);
	pcStruct->AddStructMember(PT_Float3, "sNormal");
	pcStruct->AddStructMember(PT_float, "d");
	pcStruct->AddStructMember(PT_Float3, "sPosition");
	iPlaneID = pcStruct->iID;

	//CConvexHull
	gcTypeRegistrar.mcTypeStorage.AddStructType("CConvexHullImm", &pcStruct);
	cAddInfo.GenerateVariableArray(aiVariable, iPlaneID);
	pcStruct->AddStructMemberExtendedType("mcPlanes", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CConvexHullImm::Init);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Init);
	pcStruct->AddStructMemberExtendedType("Init", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CConvexHullImm::Kill);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Kill);
	pcStruct->AddStructMemberExtendedType("Kill", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CConvexHullImm::Load);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Load);
	pcStruct->AddStructMemberExtendedType("Load", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CConvexHullImm::Save);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Save);
	pcStruct->AddStructMemberExtendedType("Save", aiVariable);
	iConvexHullID = pcStruct->iID;

	//CBox
	gcTypeRegistrar.mcTypeStorage.AddStructType("CBoxImm", &pcStruct);
	pcStruct->AddStructMember(iConvexHullID, "[CConvexHull]");  //Poor mans inheritance for the time being.
	pcStruct->AddStructMember(PT_Float3, "msMinimum");
	pcStruct->AddStructMember(PT_Float3, "msMaximum");
	GetClassFunctionPtr(pvFuncMem, CBoxImm::Init);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Init);
	pcStruct->AddStructMemberExtendedType("Init", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CBoxImm::Kill);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Kill);
	pcStruct->AddStructMemberExtendedType("Kill", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CBoxImm::Load);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Load);
	pcStruct->AddStructMemberExtendedType("Load", aiVariable);
	GetClassFunctionPtr(pvFuncMem, CBoxImm::Save);
	cAddInfo.GenerateFunction(aiVariable, pvFuncMem, PT_bool, PT_ChunkFile | POINTER_BIT, SGF_Save);
	pcStruct->AddStructMemberExtendedType("Save", aiVariable);
	iConvexHullID = pcStruct->iID;

	gcTypeRegistrar.mcTypeStorage.ResolveAllStructTypes();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ShapeTypesKill(void)
{
}

