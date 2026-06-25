#include "UTorchComponent.h"
#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"


UTorchComponent::UTorchComponent()
{
	SetIsReplicatedByDefault(true);
}

void UTorchComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SpotLight = GetOwner()->FindComponentByClass<USpotLightComponent>();

	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 500.0f;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;
	SpotLight->SetVisibility(true);
	SpotLight->SetMobility(EComponentMobility::Movable);
}
void UTorchComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UTorchComponent, bTorchOn);
}

void UTorchComponent::ToggleTorch()
{
	// UE_LOG(LogTemp, Warning, TEXT("CLIENT CLICK"));

	ServerToggleTorch();
}

void UTorchComponent::ToggleTorchState()
{
	bTorchOn = !bTorchOn;
}

void UTorchComponent::ApplyTorchState()
{
	// UE_LOG(LogTemp, Warning,
	// 	   TEXT("ApplyTorchState: %s SpotLight=%s bTorchOn=%d"),
	// 	   *GetOwner()->GetName(),
	// 	   SpotLight ? TEXT("VALID") : TEXT("NULL"),
	// 	   bTorchOn);

	// UE_LOG(LogTemp, Warning, TEXT("APPLY ENTRY %s"), *GetOwner()->GetName());

	if (!SpotLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("NO SPOTLIGHT ON %s"), *GetOwner()->GetName());
		return;
	}
	
	SpotLight->SetVisibility(bTorchOn);
	
	// UE_LOG(LogTemp, Warning,
	// 	TEXT("ApplyTorchState: %s -> %d"),
	// 	*GetOwner()->GetName(),
	// 	bTorchOn);
}

void UTorchComponent::OnRep_TorchState()
{
	// UE_LOG(LogTemp, Warning,
	// 	TEXT("OnRep_TorchState: %s -> %d, LocalRole=%d"),
	// 	*GetOwner()->GetName(),
	// 	bTorchOn,
	// 	(int32)GetOwner()->GetLocalRole());;
	
	ApplyTorchState();
}

void UTorchComponent::ServerToggleTorch_Implementation()
{
	// UE_LOG(LogTemp, Warning, TEXT("SERVER RPC FIRED"));

	ToggleTorchState();
	ApplyTorchState();

	// UE_LOG(LogTemp, Warning, TEXT("SERVER VALUE AFTER TOGGLE: %d"), bTorchOn);
}




