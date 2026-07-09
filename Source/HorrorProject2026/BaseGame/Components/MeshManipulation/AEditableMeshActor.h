#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../MeshManipulation/FMeshEdit.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "AEditableMeshActor.generated.h"

using namespace UE::Geometry;

class UDynamicMeshComponent;

UCLASS()
class AEditableMeshActor : public AActor
{
	GENERATED_BODY()

	FDynamicMesh3 MyDynamicMesh;
	
	virtual void BeginPlay() override;

private:
	void CreateTestMesh();
	
public:
	AEditableMeshActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	UDynamicMeshComponent* MeshComponent;

	UFUNCTION(Server, Reliable)
	void ServerApplyEdit(const FMeshEdit& Edit);

	UFUNCTION()
	void ApplyEdit_Internal(const FMeshEdit& Edit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastApplyEdit(const FMeshEdit& Edit);
};
