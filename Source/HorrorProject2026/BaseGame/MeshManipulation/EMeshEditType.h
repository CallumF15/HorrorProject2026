#pragma once

#include "CoreMinimal.h"
#include "EMeshEditType.generated.h"

UENUM(BlueprintType)
enum class EMeshEditType : uint8
{
	MoveVertex     UMETA(DisplayName = "Move Vertex"),
	SplitEdge      UMETA(DisplayName = "Split Edge"),
	DeleteTriangle UMETA(DisplayName = "Delete Triangle"),
	ExtrudeFace    UMETA(DisplayName = "Extrude Face")
};
