// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HorrorProject2026Character.h"
#include "HorrorCharacter.generated.h"


class USpotLightComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateHealthMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthStateChangedDelegate, bool, bTakenHealthDamage);

/**
 *  Simple first person horror character
 *  Provides stamina-based sprinting and Health Damage
 */
UCLASS(abstract)
class HORRORPROJECT2026_API AHorrorCharacter : public AHorrorProject2026Character
{
	GENERATED_BODY()

	/** Player light source */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;
	
protected:

#pragma region INPUT_ACTIONS 

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleDamageAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ToggleTorchAction;

#pragma endregion INPUT_ACTIONS

#pragma region SPRINTING
	/** If true, we're sprinting */
	bool bSprinting = false;

	/** If true, we're recovering stamina */
	bool bRecovering = false;

	/** Default walk speed when not sprinting or recovering */
	UPROPERTY(EditAnywhere, Category="Walk")
	float WalkSpeed = 250.0f;

	/** Time interval for sprinting stamina ticks */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float SprintFixedTickTime = 0.03333f;

	UPROPERTY(ReplicatedUsing = OnRep_SprintMeter) //adding multiplayer code for health replication
	float SprintMeter = 0.0f; 	/** Sprint stamina amount. Maxes at SprintTime */

	/** How long we can sprint for, in seconds */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float SprintTime = 15.0f;

	/** Walk speed while sprinting */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float SprintSpeed = 800.0f;

	/** Walk speed while recovering stamina */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float RecoveringWalkSpeed = 150.0f;

	/** Time it takes for the sprint meter to recover */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RecoveryTime = 0.0f;

	/** Sprint tick timer */
	FTimerHandle SprintTimer;


	UFUNCTION()
	void OnRep_SprintMeter();

	UFUNCTION(Server, Reliable)
	void ServerStartSprint();
	void ServerStartSprint_Implementation();

	UFUNCTION(Server, Reliable)
	void ServerStopSprint();
	void ServerStopSprint_Implementation();


#pragma endregion SPRINTING


#pragma region HEALTH

	UPROPERTY(ReplicatedUsing = OnRep_HealthMeter) //adding multiplayer code for health replication
	float HealthMeter = 0.0f; 				// Current Health

	UPROPERTY(EditDefaultsOnly, Category = "Health") 
	float MaxHealth = 100.0f;			 	// Max Health
	bool bIsHealthTakingDamage = false; 	// If true, we're taking damage 
	bool bIsHealthRecovering = false;		// If true, we're recovering health 
	bool bIsPlayerDead = false;			    // If true, we're recovering health



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


#pragma endregion HEALTH

	void DebugDrawStats(FString Label, float Value, FVector Offset, FColor Color);

	UPROPERTY(ReplicatedUsing = OnRep_TorchState)
	bool bTorchOn;

	/** Called when bTorchOn changes on clients */
	UFUNCTION()
	void OnRep_TorchState();

	/** Server RPC to toggle torch on authoritative server */
	UFUNCTION(Server, Reliable)
	void ServerToggleTorch(); 
	void ServerToggleTorch_Implementation();

public:

	/** Delegate called when the sprint meter should be updated */
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** Delegate called when we start and stop sprinting */
	FSprintStateChangedDelegate OnSprintStateChanged;

	/** Delegate called when the Health meter should be updated */
	FUpdateHealthMeterDelegate OnHealthMeterUpdated;

	/** Delegate called when we take damage to health */
	FHealthStateChangedDelegate OnHealthStateChanged;

protected:

	//Main functions go here

	/** Constructor */
	AHorrorCharacter();

	//virtual void Tick(float DeltaTime) override; //only needed if we want to do tick-based updates instead of timer-based or debugging

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	/** Starts sprinting behavior */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStartSprint();

	/** Stops sprinting behavior */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoEndSprint();

	/** Called while sprinting at a fixed time interval */
	void SprintFixedTick();

protected: 


	//Player Health bar reduces based on incoming damage
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void StopTakingDamage();

	//temp method
	void DisplayMessage();
	void ToggleDamage();
	//end

	void Die();

	/** Called when taking damage at a fixed time interval */
	void HealthFixedTick();


	UPROPERTY(Replicated)
	bool bDisableDamage = false; // Temporary disable damage for testing

	UFUNCTION(Server, Reliable)
	void Server_SetDamageDisabled(bool bDisabled);

protected:

	void ToggleTorch();

};
