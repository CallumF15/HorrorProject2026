// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HorrorProject2026Character.h"
#include "Components/UHealthComponent.h"
#include "HorrorCharacter.generated.h"



class USpotLightComponent;
class UInputAction;
class UHealthComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);


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

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UHealthComponent* HealthComponent;
	
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
	UPROPERTY(ReplicatedUsing = OnRep_Sprinting) 
	bool bSprinting = false;

	/** If true, we're recovering stamina */
	bool bRecovering = false;
	
	bool bHasStamina;

	/** Default walk speed when not sprinting or recovering */
	UPROPERTY(EditAnywhere, Category="Walk")
	float WalkSpeed = 250.0f;

	/** Time interval for sprinting stamina ticks */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float SprintFixedTickTime = 0.03333f; // 30 hz or approximately 30 updates per second
	
	float SprintMeter = 0.0f; 	/** Sprint stamina amount. Maxes at SprintTime */

	/** How long we can sprint for, in seconds */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float SprintTime = 3.0f;

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

	//Methods
	
	UFUNCTION()
	void OnRep_Sprinting();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool bNewSprinting);

#pragma endregion SPRINTING

	
	void DebugDrawStats(FString Label, float Value, FVector Offset, FColor Color);

	UPROPERTY(ReplicatedUsing = OnRep_TorchState)
	bool bTorchOn;

	/** Called when bTorchOn changes on clients */
	UFUNCTION()
	void OnRep_TorchState();

	/** Server RPC to toggle torch on authoritative server */
	UFUNCTION(Server, Reliable)
	void ServerToggleTorch(); 

public:

	/** Delegate called when the sprint meter should be updated */
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** Delegate called when we start and stop sprinting */
	FSprintStateChangedDelegate OnSprintStateChanged;

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


public:
	//UFUNCTION()
	UHealthComponent* GetHealthComponent() const;
	
protected: 


		
	//Player Health bar reduces based on incoming damage
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void StopTakingDamage();
	void ToggleDamage();

protected:

	void ToggleTorch();

};
