#include "Variant_Horror/HorrorCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"


#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Components/UHealthComponent.h"
#include "Components/AShooter.h"
#include "Components/UShooterComponent.h"
#include "Components/UTorchComponent.h"

AHorrorCharacter::AHorrorCharacter()
{
	// create health/sprint/torch component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
	TorchComponent = CreateDefaultSubobject<UTorchComponent>(TEXT("TorchComponent"));
	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));
	
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	//Initialize projectile class
	ProjectileClass = AShooter::StaticClass();
}


void AHorrorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TorchComponent->SetSpotLight(SpotLight);
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
			//Sprinting
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHorrorCharacter::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHorrorCharacter::DoEndSprint);

			//Toggle Damage Taken
			EnhancedInputComponent->BindAction(ToggleDamageAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleDamage);

			//Toggle torch
			EnhancedInputComponent->BindAction(ToggleTorchAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleTorch);

			//Shoot weapon
			EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AHorrorCharacter::StartShooting);
			// EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AHorrorCharacter::);
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
UShooterComponent* AHorrorCharacter::GetShooterComponent() const
{
	return ShooterComponent;
}


void AHorrorCharacter::DoStartSprint()
{
	if (SprintComponent)
		SprintComponent->DoStartSprint(); 
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
	UE_LOG(LogTemp, Warning,
	TEXT("NAME=%s ROLE=%d REMOTE=%d AUTH=%d LOCALCTRL=%d NETMODE=%d"),
	*GetName(),
	(int32)GetLocalRole(),
	(int32)GetRemoteRole(),
	HasAuthority(),
	IsLocallyControlled(),
	(int32)GetNetMode());

	if (!IsLocallyControlled())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("HorrorCharacter::ToggleTorch called"));
	if (TorchComponent)
		TorchComponent->ToggleTorch();
	else
		UE_LOG(LogTemp, Error, TEXT("TorchComponent is NULL"));
}

void AHorrorCharacter::StartShooting()
{
	UE_LOG(LogTemp, Warning, TEXT("SHOOTING"));
 
	if (ShooterComponent)
		ShooterComponent->StartFire();
	else
		UE_LOG(LogTemp, Warning, TEXT("null shooter component"));
}




