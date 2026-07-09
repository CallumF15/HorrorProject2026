#include "BaseGame/Characters/AHunterCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"

		/* Inputs */
#include "EnhancedInputComponent.h"
#include "InputAction.h"

#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
		/* Actors */
#include "../Projectiles/AProjectileActor.h"
		/* componets */
#include "../Components/UHealthComponent.h"
#include "../Components/UShooterComponent.h"
#include "../Components/UTorchComponent.h"

AAHunterCharacter::AAHunterCharacter()
{
	// create health/sprint/torch component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComponent"));
	TorchComponent = CreateDefaultSubobject<UTorchComponent>(TEXT("TorchComponent"));
	ShooterComponent = CreateDefaultSubobject<UShooterComponent>(TEXT("ShooterComponent"));
	
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	//Initialize projectile class
	ProjectileClass = AProjectileActor::StaticClass();
}


void AAHunterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TorchComponent->SetSpotLight(SpotLight);
}

void AAHunterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAHunterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	{
		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			//Sprinting
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AAHunterCharacter::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AAHunterCharacter::DoEndSprint);

			//Toggle Damage Taken
			EnhancedInputComponent->BindAction(ToggleDamageAction, ETriggerEvent::Started, this, &AAHunterCharacter::ToggleDamage);

			//Toggle torch
			EnhancedInputComponent->BindAction(ToggleTorchAction, ETriggerEvent::Started, this, &AAHunterCharacter::ToggleTorch);

			//Shoot weapon
			EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AAHunterCharacter::StartShooting);
			// EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AHorrorCharacter::);
		}
	}
}

void AAHunterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float AAHunterCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent)
	{
		return HealthComponent->ApplyDamage(DamageAmount);
	}

	return DamageAmount;
}
void AAHunterCharacter::ToggleDamage()
{
	if (HealthComponent)
		HealthComponent->ToggleDamage();
}

/// <summary>
/// Displays a debug string above the character with the given label and value, offset by the specified amount. Useful for visualizing stats like health or stamina during development.
/// </summary>
void AAHunterCharacter::DebugDrawStats(FString Label, float Value, FVector Offset, FColor Color)
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


UHealthComponent* AAHunterCharacter::GetHealthComponent() const
{
	return HealthComponent;
}
USprintComponent* AAHunterCharacter::GetSprintComponent() const
{
	return SprintComponent;
}
UTorchComponent* AAHunterCharacter::GetTorchComponent() const
{
	return TorchComponent;
}
UShooterComponent* AAHunterCharacter::GetShooterComponent() const
{
	return ShooterComponent;
}


void AAHunterCharacter::DoStartSprint()
{
	if (SprintComponent)
		SprintComponent->DoStartSprint(); 
}
void AAHunterCharacter::DoEndSprint()
{
	if (SprintComponent)
	{
		SprintComponent->DoEndSprint(); // or whatever your component function is
	}
}
void AAHunterCharacter::ToggleTorch()
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

void AAHunterCharacter::StartShooting()
{
	UE_LOG(LogTemp, Warning, TEXT("SHOOTING"));
 
	if (ShooterComponent)
		ShooterComponent->StartFire();
	else
		UE_LOG(LogTemp, Warning, TEXT("null shooter component"));
}




