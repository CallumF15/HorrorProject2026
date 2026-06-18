#include "UHealthComponent.h"
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

void UHealthComponent::OnRep_HealthMeter()
{
	OnHealthMeterUpdated.Broadcast(HealthMeter / MaxHealth);
}
	
void UHealthComponent::HealthFixedTick() {

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
	else
	{
		// --- RECOVERY STATE ---
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

float UHealthComponent::ApplyDamage(float DamageAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("Health after damage: %f"), HealthMeter);
	
	if (bIsPlayerDead || bDisableDamage)
		return 0.0f; // Already dead

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

void UHealthComponent::StopTakingDamage()
{
	bIsHealthTakingDamage = false;
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
	if(GetOwner()->HasAuthority())
	{
		// If this is the server (e.g., Listen Server), toggle directly
		bDisableDamage = !bDisableDamage;
	}
	else
	{
		// If this is a client, send request to the server
		Server_SetDamageDisabled(!bDisableDamage);
	}
}

void UHealthComponent::Server_SetDamageDisabled_Implementation(bool bDisabled)
{
	bDisableDamage = bDisabled;
}



