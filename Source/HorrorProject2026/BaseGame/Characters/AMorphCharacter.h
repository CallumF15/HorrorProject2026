#pragma once

#include "HorrorProject2026Character.h"
#include "../ThirdPerson/TP_ThirdPersonCharacter.h"
#include "InputMappingContext.h"
#include "AMorphCharacter.generated.h"

class USkeletalMeshComponent;
class UCameraComponent;
class UMeshEditComponent;

UCLASS(abstract)
class HORRORPROJECT2026_API AMorphCharacter : public ATP_ThirdPersonCharacter
{
	GENERATED_BODY()

protected:

	AMorphCharacter();

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void StopEdit();
	void StartEdit();
	void ToggleEditing();

	 //INPUT RELATED
	
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* ToggleMeshEditAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EditMeshAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BrushSizeAction;

	//AXIS RELATED INPUTS

	UPROPERTY(EditAnywhere, Category = "Input Axis")
	UInputAction* LockXAxisAction;

	UPROPERTY(EditAnywhere, Category = "Input Axis")
	UInputAction* LockYAxisAction;

	UPROPERTY(EditAnywhere, Category = "Input Axis")
	UInputAction* LockZAxisAction;

	// MAPPING CONTEXT

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Context") //"Input Context -> appears under category in AMorphCharacter BP
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input Context")
	UInputMappingContext* MeshEditMappingContext;

	public: 	//Component Related
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UMeshEditComponent* MeshEditComponent;
	
	UMeshEditComponent* GetMeshEditComponent() const;
};
