#pragma once

#include "EMeshEditType.h"
#include "FMeshEdit.generated.h"

USTRUCT(BlueprintType)
struct FMeshEdit
{
	GENERATED_BODY()
	
	UPROPERTY()
	EMeshEditType Type;

	UPROPERTY()
	int32 VertexIndex;
	
	UPROPERTY() 
	FVector_NetQuantize Position;
	//FVector_NetQuantize is a network-optimized version of FVector in Unreal Engine.
	//It behaves like a normal FVector in code, but when it is replicated over the network, Unreal compresses it.

	//can test these other variables below for networking later if needed
	//FVector_NetQuantize10 postion; 
	//FVector_NetQuantize100 position2;
	//FVector_NetQuantizeNormal                                                             
};
                                           