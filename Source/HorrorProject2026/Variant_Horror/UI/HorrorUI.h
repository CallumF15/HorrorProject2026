// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorUI.generated.h"

class AHorrorCharacter;

/**
 *  Simple UI for a first person horror game
 *  Manages character sprint meter display
 */
UCLASS(abstract)
class HORRORPROJECT2026_API UHorrorUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Sets up delegate listeners for the passed character */
	void SetupCharacter(AHorrorCharacter* HorrorCharacter);

	/** Called when the character's sprint meter is updated */
	UFUNCTION()
	void OnSprintMeterUpdated(float Percent);

	/** Called when the character's sprint state changes */
	UFUNCTION()
	void OnSprintStateChanged(bool bSprinting);

	///////////////////////////////////
	//Health Related
	///////////////////////////////////

	UFUNCTION()
	void OnHealthMeterUpdated(float Percent);

	UFUNCTION()
	void OnHealthStateChanged(bool bTakenHealthDamage);

protected:

	/** Passes control to Blueprint to update the sprint meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint Meter Updated"))
	void BP_SprintMeterUpdated(float Percent);

	/** Passes control to Blueprint to update the sprint meter status */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint State Changed"))
	void BP_SprintStateChanged(bool bSprinting);

	///////////////////////////////////
	//Health Related
	///////////////////////////////////

	/** Passes control to Blueprint to update the sprint meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category = "Horror", meta = (DisplayName = "Health Meter Updated"))
	void BP_HealthMeterUpdated(float Percent);

	/** Passes control to Blueprint to update the health meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category = "Horror", meta = (DisplayName = "Health Meter Changed"))
	void BP_HealthMeterChanged(bool bTakenHealthDamage);
};
