#pragma once

#include "CoreMinimal.h"
#include "HorrorProject2026Character.h"
#include "Components/UHealthComponent.h"
#include "Components/USprintComponent.h"
#include "Components/UTorchComponent.h"
#include "HorrorCharacter.generated.h"

class USpotLightComponent;

class UInputAction;
class UHealthComponent;
class USprintComponent;
class UTorchComponent;

UCLASS(abstract)
class HORRORPROJECT2026_API AHorrorCharacter : public AHorrorProject2026Character
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;

protected:
	
	AHorrorCharacter();

	//virtual void Tick(float DeltaTime) override; //only needed if we want to do tick-based updates instead of timer-based or debugging

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public: //Component Related
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USprintComponent* SprintComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UTorchComponent* TorchComponent;

	UHealthComponent* GetHealthComponent() const;
	USprintComponent* GetSprintComponent() const;
	UTorchComponent* GetTorchComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStartSprint();

	/** Stops sprinting behavior */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoEndSprint();

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
	
	void DebugDrawStats(FString Label, float Value, FVector Offset, FColor Color);

protected: 
	//Player Health bar reduces based on incoming damage
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void ToggleDamage();
	void ToggleTorch();
};
