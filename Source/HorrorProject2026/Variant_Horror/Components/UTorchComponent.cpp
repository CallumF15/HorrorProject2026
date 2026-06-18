#include "UTorchComponent.h"
#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"


UTorchComponent::UTorchComponent()
{
	// SpotLight->SetMobility(EComponentMobility::Movable);
	// SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	// SpotLight->Intensity = 500.0f;
	// SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	// SpotLight->AttenuationRadius = 1050.0f;
	// SpotLight->InnerConeAngle = 18.7f;
	// SpotLight->OuterConeAngle = 45.24f;
	// SpotLight->SetVisibility(true);
	// SpotLight->SetMobility(EComponentMobility::Movable);
}


void UTorchComponent::SetSpotLight(USpotLightComponent* InSpotLight)
{
	SpotLight = InSpotLight;

	UE_LOG(LogTemp, Warning, TEXT("SetSpotLight called on: %s, SpotLight: %s"), 
	GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
	SpotLight ? TEXT("VALID") : TEXT("NULL"));

	// All spotlight setup lives here
	SpotLight->SetMobility(EComponentMobility::Movable);
	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 500.0f;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;
	SpotLight->SetVisibility(true);
	SpotLight->SetMobility(EComponentMobility::Movable);
}


void UTorchComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (SpotLight)
	{
		bTorchOn = true;
	}
}
void UTorchComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UTorchComponent, bTorchOn);
}

void UTorchComponent::ToggleTorch()
{
	UE_LOG(LogTemp, Warning, TEXT("UTorchComponent::ToggleTorch - SpotLight: %s, bTorchOn: %d, HasAuthority: %d"), 
	SpotLight ? TEXT("VALID") : TEXT("NULL"), bTorchOn, GetOwner()->HasAuthority());

	
	if (GetOwner()->HasAuthority())
	{
		// If this is the server (e.g., Listen Server), toggle directly
		bTorchOn = !bTorchOn;
		UE_LOG(LogTemp, Warning, TEXT("bTorchOn now: %d"), bTorchOn);
		OnRep_TorchState();
	}
	else
	{
		// If this is a client, send request to the server
		ServerToggleTorch();
	}


}
void UTorchComponent::OnRep_TorchState()
{
	if (SpotLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mobility: %d"), (int32)SpotLight->Mobility.GetValue());
		SpotLight->SetVisibility(bTorchOn);
		SpotLight->MarkRenderStateDirty();
		UE_LOG(LogTemp, Warning, TEXT("IsVisible after set: %d"), SpotLight->IsVisible());
	}
}

void UTorchComponent::ServerToggleTorch_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerToggleTorch_Implementation called - bTorchOn: %d"), bTorchOn);
	
	bTorchOn = !bTorchOn;
	UE_LOG(LogTemp, Warning, TEXT("bTorchOn now: %d"), bTorchOn);
	OnRep_TorchState(); // update locally on server
}


