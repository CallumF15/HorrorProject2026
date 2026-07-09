#pragma once

#include "AEditableMeshActor.h"
#include "InputActionValue.h"
#include "UMeshEditComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMeshEditComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeshEditComponent();
	
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** Set up input action bindings */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TryEditMesh();
	
	void SetTargetMesh(AEditableMeshActor* NewTarget);
	void SetEditingEnabled(bool bEnabled);
	void ToggleEditModeEnabled();
	bool getEditModeEnabled() const;

	EMeshEditType currentMode = EMeshEditType::MoveVertex; //may change to something else;
	
	// void UMeshEditComponent::SetToolMode(EMeshEditType NewMode);
private:
	bool bIsEditingAction = false;
	bool bIsEditModeEnabled = false; //menu toggle

public: //Brush Related 
	UPROPERTY()
	UStaticMeshComponent* BrushPreviewSphere;
	
	UPROPERTY()
	UDynamicMeshComponent* DynamicMeshComponent;

	UPROPERTY(EditAnywhere, Category="Brush")
	float BrushRadius = 50.f;

	void CreateMeshAtBrushLocation();
	void MouseOnScreen();
	bool TraceForBrush(FHitResult& OutHit);

	bool GetMousePlanePosition(FVector& OutPosition);
	void UpdateVerticalBrushMovement();
	void Test(FVector& BrushLocation);

	bool bisXAxisLocked = false;
	bool bisYAxisLocked = false;
	bool bisZAxisLocked = false;
	
	float lockedAxisX;
	float lockedAxisY;
	float lockedAxisZ;

	void CheckLockedAxis(FVector& BrushLocation);

	void ToggleXAxisLock();
	void ToggleYAxisLock();
	void ToggleZAxisLock();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brush")
	float brushScaleMultiplier {3};

	static constexpr float MIN_BRUSH_SCALE {0.5f};
	static constexpr float MAX_BRUSH_SCALE {100.0f};
	
	void AdjustBrushSize(const FInputActionValue& inputValue);
	

protected:
	
	UPROPERTY()
	AEditableMeshActor* TargetMesh;

	bool TraceForMesh(FHitResult& OutHit);

	void SendEditToServer(const FMeshEdit& Edit);

	UFUNCTION(Server, Reliable)
	void ServerSendEdit(AEditableMeshActor* Target, FMeshEdit Edit);
};
