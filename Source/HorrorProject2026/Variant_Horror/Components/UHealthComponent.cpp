#include "UHealthComponent.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("No World in HealthComponent BeginPlay"));
		return;
	}

	// HealthMeter = MaxHealth;
	UE_LOG(LogTemp, Warning, TEXT("Health BeginPlay: %f"), HealthMeter);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HealthMeter = MaxHealth;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		HealthTimer,
		this,
		&UHealthComponent::HealthFixedTick,
		HealthFixedTickTime,
		true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, HealthMeter);
}

//RepNotify function. This function will be triggered, when a client successfully receives the replicated data
void UHealthComponent::OnRep_HealthMeter()
{
	// UE_LOG(LogTemp, Warning, TEXT("Health updated via replication: %f"), HealthMeter);
	OnHealthUpdate();
	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);
}

// void UHealthComponent::Server_SetDamageDisabled_Implementation(bool bDisabled)
// {
// 	bDisableDamage = bDisabled;
// }

void UHealthComponent::ServerHealth_Implementation()
{
	//if (HealthMeter < MinHealth || HealthMeter > MaxHealth) return;
}

//validation function helps the server determine whether an RPC should or should not run
//client makes a call to execute a server RPC, the validation function is called first on the server.
bool UHealthComponent::ServerHealth_Validate()
{
	// If the inputs pass validation, the implementation is called.
	// If the inputs fail validation, the invoking client is disconnected from the server.
	
	if (HealthMeter > MaxHealth || HealthMeter < 0) //check this (specifically HealthFixedTickTime)
	{
		return false;
	}
	
	return true;
}

void UHealthComponent::HealthFixedTick()
{
	if (bIsPlayerDead) return; // Don't process if player is dead

	const float TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;
	
	 if (bDisableDamage == false) {
	 	ApplyDamage(HealthDamageRate);
	 }
	 else
	 	bIsHealthTakingDamage = false;
	
	//show debug on screen
	//DisplayMessage();

	//UE_LOG(LogTemp, Warning, TEXT("bIsHealthTakingDamage: %s"), bIsHealthTakingDamage ? TEXT("True") : TEXT("False"));
	if (bIsHealthTakingDamage)
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
	else // RECOVERY STATE
	{
		// Only start recovering if the "checkpoint" was long enough ago
		if (TimeSinceDamage >= HealthRecoveryDelay && HealthMeter < MaxHealth)
		{
			//UE_LOG(LogTemp, Warning, TEXT("RECOVERING HEALTH"));
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
}

void UHealthComponent::OnHealthUpdate()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	//Client-specific functionality
	if (Pawn->IsLocallyControlled())
	{
		// FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), HealthMeter);
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	 
		if (HealthMeter <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}
	 
	//Server-specific functionality
	if (Pawn->GetLocalRole() == ROLE_Authority)
	{
		// FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), HealthMeter);
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
	 
	//Functions that occur on all machines.
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

//by checking that the Network Role of the Actor is ROLE_Authority, you restrict this function to execute only if it is called on the hosted game server.
void UHealthComponent::SetCurrentHealth(float healthValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	
	if (Pawn->GetLocalRole() == ROLE_Authority) 
	{
		HealthMeter = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float UHealthComponent::ApplyDamage(float DamageAmount)
{
	// UE_LOG(LogTemp, Warning, TEXT("Health after damage: %f"), HealthMeter);
	
	if (bIsPlayerDead || bDisableDamage)
		return 0.0f; // Already dead

	//apply the damage
	SetCurrentHealth(HealthMeter - DamageAmount);
	
	// UE_LOG(LogTemp, Warning, TEXT("Damage taken: %f | New Health: %f | Owner: %s"),
	// DamageAmount,
	// HealthMeter,
	// *GetOwner()->GetName());
	
	bIsHealthTakingDamage = true;
	bIsHealthRecovering = false;

	//record the time of this damage tick for recovery delay purposes
	LastDamageTime = GetWorld()->GetTimeSeconds();

	if (HealthMeter <= 0.0f)
		Die();

	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);

	return DamageAmount;
}

void UHealthComponent::Die() {

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

void UHealthComponent::ToggleDamage()
{
	bDisableDamage = !bDisableDamage;

	// UE_LOG(LogTemp, Warning, TEXT("Damage toggled locally: %d"), bDisableDamage);
	// OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);
	
	// if(GetOwner()->HasAuthority())
	// {
	// 	// If this is the server (e.g., Listen Server), toggle directly
	// 	bDisableDamage = !bDisableDamage;
	// }
	// else
	// {
	// 	//If this is a client, send request to the server
	// 	Server_SetDamageDisabled(!bDisableDamage);
	// }
}





