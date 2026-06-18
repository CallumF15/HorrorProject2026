#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHealthComponent.generated.h" // MUST be LAST include


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateHealthMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthStateChangedDelegate, bool, bTakenHealthDamage);

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
	UPROPERTY(ReplicatedUsing = OnRep_HealthMeter) //adding multiplayer code for health replication
	float HealthMeter = 0.0f; 				// Current Health

	UPROPERTY(EditDefaultsOnly, Category = "Health") 
	float MaxHealth = 100.0f;			 	// Maximum Health the player can have
	bool bIsHealthTakingDamage = false; 	// Is the player taking Damage?
	bool bIsHealthRecovering = false;		// Is the player recovering health?
	bool bIsPlayerDead = false;			    


	/** Time interval for health ticks */
	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float HealthFixedTickTime = 0.03333f; //consider changing tick rate later on 
	
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float HealthRecoveryRate = 10.0f; 	//Amount of health to recover per second

	/** Time after damage before health meter recovery begins  */
	UPROPERTY(EditAnywhere, Category = "Health")
	float HealthRecoveryDelay = 2.0f; // seconds after last damage before recovery begins
	
	/** Time after damage before health meter recovery begins  */
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float HealthDamageRate = .2f; // Amount of health to lose per second while taking damage


	float LastDamageTime = 0.0f; // Time of the last damage tick, used for recovery delay calculations

	/** Health tick timer */
	FTimerHandle HealthTimer;

	UFUNCTION()
	void OnRep_HealthMeter();

	
	/** Called when taking damage at a fixed time interval */
	void HealthFixedTick();
	
	//Damage
	// virtual float ApplyDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	float ApplyDamage(float DamageAmount);
	void StopTakingDamage();
	void ToggleDamage(); // Enable / disable damage (temp method)
	void Die();
	
	UPROPERTY(Replicated)
	bool bDisableDamage = false; // Temporary disable damage for testing

	UFUNCTION(Server, Reliable)
	void Server_SetDamageDisabled(bool bDisabled);
};
