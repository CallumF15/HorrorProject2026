#pragma once

#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "UTorchComponent.generated.h" // MUST be LAST include

class USpotLightComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT2026_API UTorchComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTorchComponent();

protected:

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_TorchState)
	bool bTorchOn;

	/** Called when bTorchOn changes on clients */
	UFUNCTION()
	void OnRep_TorchState();

	/** Server RPC to toggle torch on authoritative server */
	UFUNCTION(Server, Reliable)
	void ServerToggleTorch();

	void ToggleTorchState();
	void ApplyTorchState();

private:
	UPROPERTY()
	UCameraComponent* Camera;

public:
	/** Player light source */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;

	void ToggleTorch();
};
