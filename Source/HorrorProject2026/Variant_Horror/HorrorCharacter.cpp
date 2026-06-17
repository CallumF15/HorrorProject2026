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
#include "Components/UHealthComponent.h"

AHorrorCharacter::AHorrorCharacter()
{
	// create the spotlight
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	// create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

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

	// Initialize the walk speed
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// start the sprint tick timer
	GetWorld()->GetTimerManager().SetTimer(SprintTimer, this, &AHorrorCharacter::SprintFixedTick, SprintFixedTickTime, true);
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

void AHorrorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHorrorCharacter, bSprinting);
	DOREPLIFETIME(AHorrorCharacter, bTorchOn);
}




#pragma region OtherMethods

	#pragma region Sprinting

void AHorrorCharacter::DoStartSprint()
{
	//UE_LOG(LogTemp, Warning, TEXT("DoStartSprint CALLED"));
	bSprinting = true;
	
	ServerSetSprinting(true);
	
}

void AHorrorCharacter::DoEndSprint()
{
	//UE_LOG(LogTemp, Warning, TEXT("DoEndSprint CALLED"));
	bSprinting = false;
	
	ServerSetSprinting(false);
}

void AHorrorCharacter::SprintFixedTick()
{
	UE_LOG(LogTemp, Warning, TEXT("Sprinting: %d"), bSprinting);
	UE_LOG(LogTemp, Warning, TEXT("Stamina: %f"), SprintMeter);

	bHasStamina = SprintMeter > 0.01f;
	
	if (bSprinting)
	{
		if (bHasStamina)
			SprintMeter -= SprintFixedTickTime;
		//SprintMeter -= SprintDrainRate * SprintFixedTickTime;
	}
	else
	{
		SprintMeter += SprintFixedTickTime; //recover stamima
		//SprintMeter += SprintRegenRate * SprintFixedTickTime; // can expand and add Regen 
	}

	SprintMeter = FMath::Clamp(SprintMeter, 0.f, SprintTime);
	OnSprintStateChanged.Broadcast(bSprinting);
	
	const float TargetSpeed = bSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	
	//bRecovering = (!bSprinting && SprintMeter < SprintTime);

	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);

	// UE_LOG(LogTemp, 
	// 	Log, TEXT("SprintStates: bSprinting=%s, bSprintButtonHeld=%s, bRecovering=%s, SprintMeter=%.2f"),
	// 	bSprinting ? TEXT("true") : TEXT("false"),
	// 	bSprintButtonHeld ? TEXT("true") : TEXT("false"),
	// 	bRecovering ? TEXT("true") : TEXT("false"),
	// 	SprintMeter
	// );
}


void AHorrorCharacter::OnRep_Sprinting()
{
	if (!IsLocallyControlled())
	{
		// Only non-owners accept server correction
		// Owners trust local simulation
		UE_LOG(LogTemp, Warning, TEXT("IsLocallyControlled sprint = %d"), bSprinting);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ROLE_Authority sprint = %d"), bSprinting);
	}
	
	OnSprintStateChanged.Broadcast(bSprinting);
	
	// Draw Sprint Meter above character in green
	DebugDrawStats(TEXT("Sprint"), SprintMeter, FVector(0, 0, 120.f), FColor::Green);
}

//RPC Call
void AHorrorCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	bSprinting = bNewSprinting;
	
	// UE_LOG(LogTemp, Warning, TEXT("SERVER Sprint = %d"), bNewSprinting);
	// UE_LOG(LogTemp, Warning, TEXT("SERVER Sprinting: %d Stamina: %f"), bSprinting, SprintMeter)
}

//RPC validation
bool AHorrorCharacter::ServerSetSprinting_Validate(bool bNewSprinting)
{
	if(bNewSprinting){

		SprintMeter -= SprintFixedTickTime;
		
		if (SprintMeter < 0.f || SprintMeter > 100.f)
		{
			return false;
		}
	}

	return true;
}

	#pragma endregion


float AHorrorCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent)
	{
		return HealthComponent->ApplyDamage(DamageAmount);
	}

	return DamageAmount;
}

void AHorrorCharacter::StopTakingDamage()
{
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

