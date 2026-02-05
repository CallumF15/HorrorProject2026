// Copyright Epic Games, Inc. All Rights Reserved.



#include "Variant_Horror/HorrorCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Engine/EngineTypes.h"

AHorrorCharacter::AHorrorCharacter()
{
	// create the spotlight
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 0.5;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;
}

void AHorrorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// initialize sprint & health meter to max
	SprintMeter = SprintTime;
	HealthMeter = MaxHealth;

	// Initialize the walk speed
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// start the sprint tick timer
	GetWorld()->GetTimerManager().SetTimer(SprintTimer, this, &AHorrorCharacter::SprintFixedTick, SprintFixedTickTime, true);

	// start the health tick timer
	GetWorld()->GetTimerManager().SetTimer(HealthTimer, this, &AHorrorCharacter::HealthFixedTick, HealthFixedTickTime, true);
}

void AHorrorCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the sprint timer
	GetWorld()->GetTimerManager().ClearTimer(SprintTimer);
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
			EnhancedInputComponent->BindAction(ToggleAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleDamage);
		}
	}
}

void AHorrorCharacter::DoStartSprint()
{
	// set the sprinting flag
	bSprinting = true;

	// are we out of recovery mode?
	if (!bRecovering)
	{
		// set the sprint walk speed
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

		// call the sprint state changed delegate
		OnSprintStateChanged.Broadcast(true);
	}

}

void AHorrorCharacter::DoEndSprint()
{
	// set the sprinting flag
	bSprinting = false;

	// are we out of recovery mode?
	if (!bRecovering)
	{
		// set the default walk speed
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

		// call the sprint state changed delegate
		OnSprintStateChanged.Broadcast(false);
	}
}

void AHorrorCharacter::SprintFixedTick()
{
	// are we out of recovery, still have stamina and are moving faster than our walk speed?
	if (bSprinting && !bRecovering && GetVelocity().Length() > WalkSpeed)
	{

		// do we still have meter to burn?
		if (SprintMeter > 0.0f)
		{
			// update the sprint meter
			SprintMeter = FMath::Max(SprintMeter - SprintFixedTickTime, 0.0f);

			// have we run out of stamina?
			if (SprintMeter <= 0.0f)
			{
				// raise the recovering flag
				bRecovering = true;

				// set the recovering walk speed
				GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;
			}
		}
		
	} else {

		// recover stamina
		SprintMeter = FMath::Min(SprintMeter + SprintFixedTickTime, SprintTime);

		if (SprintMeter >= SprintTime)
		{
			// lower the recovering flag
			bRecovering = false;

			// set the walk or sprint speed depending on whether the sprint button is down
			GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;

			// update the sprint state depending on whether the button is down or not
			OnSprintStateChanged.Broadcast(bSprinting);
		}

	}

	// broadcast the sprint meter updated delegate
	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);

}

float AHorrorCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsPlayerDead || bDisableDamage)
	{
		return 0.0f; // Already dead
	}

	//apply the damage
	HealthMeter = FMath::Max(HealthMeter - DamageAmount, 0.0f);

	bIsHealthTakingDamage = true;
	bIsHealthRecovering = false;

	//record the time of this damage tick for recovery delay purposes
	LastDamageTime = GetWorld()->GetTimeSeconds();

	if (HealthMeter <= 0.0f)
		Die();

	return DamageAmount;
}

void AHorrorCharacter::StopTakingDamage()
{
	bIsHealthTakingDamage = false;
}

void AHorrorCharacter::Die() {

	bIsPlayerDead = true;
	bIsHealthTakingDamage = false;
	bIsHealthRecovering = false;

	// Disable movement
	//GetCharacterMovement()->DisableMovement();

	// Trigger death animation
	// PlayAnimMontage(DeathAnim);

	// Optionally notify UI/game mode
	//OnPlayerDied.Broadcast();
}

void AHorrorCharacter::HealthFixedTick() {

	if (bIsPlayerDead) return; // Don't process if player is dead

	const float TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;

	if (bDisableDamage == false) {
		TakeDamage(HealthDamageRate, FDamageEvent(), nullptr, nullptr);
	}
	else
		bIsHealthTakingDamage = false;


	//show debug on screen
	DisplayMessage();

	UE_LOG(LogTemp, Warning, TEXT("bIsHealthTakingDamage: %s"), bIsHealthTakingDamage ? TEXT("True") : TEXT("False"));
	if (bIsHealthTakingDamage && bDisableDamage == false)
	{
		LastDamageTime = GetWorld()->GetTimeSeconds();
		UE_LOG(LogTemp, Warning, TEXT("TimeSinceDamage: %f"), LastDamageTime);

		// --- DAMAGE STATE ---
		bIsHealthRecovering = false;

		// Apply damage over time
		HealthMeter = FMath::Max(HealthMeter - (HealthDamageRate * HealthFixedTickTime), 0.0f);

		// RESET THE CHECKPOINT: While damage is happening, the timer stays at 0
		LastDamageTime = GetWorld()->GetTimeSeconds();
	}
	else
	{
		// --- RECOVERY STATE ---
		// Only start recovering if the "checkpoint" was long enough ago
		if (TimeSinceDamage >= HealthRecoveryDelay && HealthMeter < MaxHealth)
		{
			UE_LOG(LogTemp, Warning, TEXT("It's RECOVERY TIME AHHHH "));
			bIsHealthRecovering = true;
			HealthMeter = FMath::Min(HealthMeter + (HealthRecoveryRate * HealthFixedTickTime), MaxHealth);
		}
		else
		{
			// We are in the "Cooldown" period (waiting for the delay to finish)
			bIsHealthRecovering = false;
		}
	}

	// Optional: Adjust walk speed based on state
	//if (bIsHealthRecovering)
	//{
	//	GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;
	//}
	//else
	//{
	//	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	//}

	// broadcast UI update
	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);
}

void AHorrorCharacter::DisplayMessage() {

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.5f,
			FColor::Green,
			FString::Printf(
				TEXT("Health: %.1f | TakingDamage: %d | Recovering: %d"),
				HealthMeter,
				bIsHealthTakingDamage,
				bIsHealthRecovering
			)
		);
	}
}

void AHorrorCharacter::ToggleDamage()
{
	bDisableDamage = !bDisableDamage;
	
	if (bDisableDamage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Damage DISABLED - Recovery can take place"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Damage ENABLED"));
	}
}