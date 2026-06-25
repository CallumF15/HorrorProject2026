// Copyright Epic Games, Inc. All Rights Reserved.


#include "HunterUI.h"
#include "../Characters/HunterCharacter.h"

void UHunterUI::SetupCharacter(AHunterCharacter* HorrorCharacter)
{
	UHealthComponent* HealthComp = HorrorCharacter->GetHealthComponent();
	USprintComponent* SprintComp = HorrorCharacter->GetSprintComponent();
	
	SprintComp->OnSprintMeterUpdated.AddDynamic(this, &UHunterUI::OnSprintMeterUpdated);
	SprintComp->OnSprintStateChanged.AddDynamic(this, &UHunterUI::OnSprintStateChanged);
	HealthComp->OnHealthMeterUpdated.AddDynamic(this, &UHunterUI::OnHealthMeterUpdated);
	HealthComp->OnHealthStateChanged.AddDynamic(this, &UHunterUI::OnHealthStateChanged);
}

void UHunterUI::OnSprintMeterUpdated(float Percent)
{
	// call the BP handler
	BP_SprintMeterUpdated(Percent);
}

void UHunterUI::OnSprintStateChanged(bool bSprinting)
{
	// call the BP handler
	BP_SprintStateChanged(bSprinting);
}

void UHunterUI::OnHealthMeterUpdated(float Percent)
{
	// call the BP handler
	BP_HealthMeterUpdated(Percent);
}

void UHunterUI::OnHealthStateChanged(bool bTakenHealthDamage)
{
	// call the BP handler
	BP_HealthMeterChanged(bTakenHealthDamage);
}
