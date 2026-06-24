#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ActorComponent.h"
#include "UHealthComponent.generated.h" // MUST be LAST include


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateHealthMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthStateChangedDelegate, bool, bTakenHealthDamage);

// class ACharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Delegate called when the Health meter should be updated */
	FUpdateHealthMeterDelegate OnHealthMeterUpdated;

	/** Delegate called when we take damage to health */
	FHealthStateChangedDelegate OnHealthStateChanged;

protected:
	UHealthComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Health") 
	float MaxHealth = 100.0f;			 	// Maximum Health the player can have
	
	void ToggleDamage(); // Enable / disable damage (temp method)
	float ApplyDamage(float DamageAmount);
	
private:

	UPROPERTY(ReplicatedUsing = OnRep_HealthMeter) //adding multiplayer code for health replication
	float HealthMeter = 0.0f; 				// Current Health
	float MinHealth = 0.0f;					// Minimum health the player can have
	bool bIsHealthTakingDamage = false; 	// Is the player taking Damage?
	bool bIsHealthRecovering = false;		// Is the player recovering health?
	bool bIsPlayerDead = false;			    // Is the player dead?
	float LastDamageTime = 0.0f; // Time of the last damage tick, used for recovery delay calculations
	bool bDisableDamage = false; // Temporary disable damage for testing
	
	/** Time interval for health ticks */
	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float HealthFixedTickTime = 0.03333f; 

	/** Amount of health recovered to recover per second */
	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float HealthRecoveryRate = 10.0f; 	//

	/** Time after damage before health meter recovery begins  */
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthRecoveryDelay = 2.0f;
	
	/** damage value to rem  */
	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float HealthDamageRate = .2f; 
	
	/** Health tick timer */
	FTimerHandle HealthTimer;

	UFUNCTION()
	void OnRep_HealthMeter();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHealth();
	
	/** Called when taking damage at a fixed time interval */
	void HealthFixedTick();
	void OnHealthUpdate();
	void SetCurrentHealth(float HealthValue);
	void Die();
};