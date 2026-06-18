#include "Variant_Horror/HorrorCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"


#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Components/UHealthComponent.h"
#include "Components/UTorchComponent.h"

AHorrorCharacter::AHorrorCharacter()
{
	// create health/sprint/torch component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
	TorchComponent = CreateDefaultSubobject<UTorchComponent>(TEXT("TorchComponent"));
	
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());
	SpotLight->SetVisibility(false); // start off
	SpotLight->Intensity = 500.0f;
}

void AHorrorCharacter::BeginPlay()
{
	Super::BeginPlay();

	TorchComponent->SetSpotLight(SpotLight);
	
}

void AHorrorCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AHorrorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	{
		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			// Sprinting
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHorrorCharacter::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHorrorCharacter::DoEndSprint);

			//Toggle Damage Taken
			EnhancedInputComponent->BindAction(ToggleDamageAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleDamage);

			//toggle torch
			EnhancedInputComponent->BindAction(ToggleTorchAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleTorch);
		}
	}
}

void AHorrorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}




float AHorrorCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent)
	{
		return HealthComponent->ApplyDamage(DamageAmount);
	}

	return DamageAmount;
}
void AHorrorCharacter::ToggleDamage()
{
	if (HealthComponent)
		HealthComponent->ToggleDamage();
}



/// <summary>
/// Displays a debug string above the character with the given label and value, offset by the specified amount. Useful for visualizing stats like health or stamina during development.
/// </summary>
void AHorrorCharacter::DebugDrawStats(FString Label, float Value, FVector Offset, FColor Color)
{
	if (!GetWorld()) return;

	// Combine the label and value into a string
	const FString Text = FString::Printf(TEXT("%s: %.1f"), *Label, Value);

	DrawDebugString(
		GetWorld(),
		Offset,
		Text,
		this,
		Color,
		0.f,   // 0 = every frame
		true   // draw on top
	);
}


UHealthComponent* AHorrorCharacter::GetHealthComponent() const
{
	return HealthComponent;
}
USprintComponent* AHorrorCharacter::GetSprintComponent() const
{
	return SprintComponent;
}
UTorchComponent* AHorrorCharacter::GetTorchComponent() const
{
	return TorchComponent;
}

void AHorrorCharacter::DoStartSprint()
{
	if (SprintComponent)
	{
		SprintComponent->DoStartSprint(); // or whatever your component function is
	}
}
void AHorrorCharacter::DoEndSprint()
{
	if (SprintComponent)
	{
		SprintComponent->DoEndSprint(); // or whatever your component function is
	}
}
void AHorrorCharacter::ToggleTorch()
{
	// if (TorchComponent)
	// 	TorchComponent->ToggleTorch();

	UE_LOG(LogTemp, Warning, TEXT("HorrorCharacter::ToggleTorch called"));
	if (TorchComponent)
		TorchComponent->ToggleTorch();
	else
		UE_LOG(LogTemp, Error, TEXT("TorchComponent is NULL"));
}


