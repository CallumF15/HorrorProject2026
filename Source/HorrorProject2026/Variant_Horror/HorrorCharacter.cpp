// Copyright Epic Games, Inc. All Rights Reserved.



#include "Variant_Horror/HorrorCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"

//network
#include "Net/UnrealNetwork.h"

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

			//Toggle Damage Taken
			EnhancedInputComponent->BindAction(ToggleDamageAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleDamage);

			//toggle torch
			EnhancedInputComponent->BindAction(ToggleTorchAction, ETriggerEvent::Started, this, &AHorrorCharacter::ToggleTorch);
		}
	}
}

#pragma region NETWORKING

void AHorrorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHorrorCharacter, HealthMeter);
	DOREPLIFETIME(AHorrorCharacter, SprintMeter);
	DOREPLIFETIME(AHorrorCharacter, bTorchOn);
	DOREPLIFETIME(AHorrorCharacter, bDisableDamage);
}

void AHorrorCharacter::OnRep_HealthMeter()
{
	if (!IsLocallyControlled())
	{
		// Only non-owners accept server correction
		// Owners trust local simulation
	}

	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);

	// Draw Health above character in cyan
	DebugDrawStats(TEXT("Health"), HealthMeter, FVector(0, 0, 100.f), FColor::Red);
}
void AHorrorCharacter::OnRep_SprintMeter()
{
	if (!IsLocallyControlled())
	{
		// Only non-owners accept server correction
		// Owners trust local simulation
	}
	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);
	// Draw Sprint Meter above character in green
	DebugDrawStats(TEXT("Sprint"), SprintMeter, FVector(0, 0, 120.f), FColor::Green);
}

void  AHorrorCharacter::ServerStartSprint_Implementation()
{
	bSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void  AHorrorCharacter::ServerStopSprint_Implementation()
{
	bSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

#pragma endregion

#pragma region OtherMethods

	#pragma region Sprinting

void AHorrorCharacter::DoStartSprint()
{
	if (HasAuthority()) {
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
		else {
			// if we're not the server, call the server function to start sprinting
			ServerStartSprint();
		}
	}else
		ServerStartSprint();
}

void AHorrorCharacter::DoEndSprint()
{
	if (HasAuthority()) 
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

			//DebugDrawStats(TEXT("Health"), SprintMeter, FVector(0, 0, 100.f), FColor::Green);
		}
	}
	else
		ServerStopSprint();
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

				bRecovering = true;
				if (!GetCharacterMovement()->IsFalling())
				{
					GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;
				}

				// set the recovering walk speed
				//GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;
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
	DebugDrawStats(TEXT("Sprint"), SprintMeter, FVector(0, 0, 120.f), FColor::Green);

}

	#pragma endregion

	#pragma region Health

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
	//DisplayMessage();

	//UE_LOG(LogTemp, Warning, TEXT("bIsHealthTakingDamage: %s"), bIsHealthTakingDamage ? TEXT("True") : TEXT("False"));
	if (bIsHealthTakingDamage && bDisableDamage == false)
	{
		LastDamageTime = GetWorld()->GetTimeSeconds();
		//UE_LOG(LogTemp, Warning, TEXT("TimeSinceDamage: %f"), LastDamageTime);

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
			//UE_LOG(LogTemp, Warning, TEXT("It's RECOVERY TIME AHHHH "));
			bIsHealthRecovering = true;
			HealthMeter = FMath::Min(HealthMeter + (HealthRecoveryRate * HealthFixedTickTime), MaxHealth);
		}
		else
		{
			// We are in the "Cooldown" period (waiting for the delay to finish)
			bIsHealthRecovering = false;
		}
	}

	// broadcast UI update
	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);
	// Draw Health above character in cyan
	DebugDrawStats(TEXT("Health"), HealthMeter, FVector(0, 0, 100.f), FColor::Red);
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

	#pragma endregion

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
	if(HasAuthority())
	{
		// If this is the server (e.g., Listen Server), toggle directly
		bDisableDamage = !bDisableDamage;
	}
	else
	{
		// If this is a client, send request to the server
		Server_SetDamageDisabled(!bDisableDamage);
	}

	bDisableDamage = !bDisableDamage;
	
	//if (bDisableDamage)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Damage DISABLED - Recovery can take place"));
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Damage ENABLED"));
	//}
}

void AHorrorCharacter::ToggleTorch()
{
	if (HasAuthority())
	{
		// If this is the server (e.g., Listen Server), toggle directly
		bTorchOn = !bTorchOn;
		OnRep_TorchState();
	}
	else
	{
		// If this is a client, send request to the server
		ServerToggleTorch();
	}
}

void AHorrorCharacter::OnRep_TorchState()
{
	if (SpotLight)
	{
		SpotLight->SetVisibility(bTorchOn);
	}
}

void AHorrorCharacter::ServerToggleTorch_Implementation()
{
	bTorchOn = !bTorchOn;
	OnRep_TorchState(); // update locally on server
}

#pragma endregion

void AHorrorCharacter::Server_SetDamageDisabled_Implementation(bool bDisabled)
{
	bDisableDamage = bDisabled;
}