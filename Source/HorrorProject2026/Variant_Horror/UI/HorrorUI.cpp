// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorUI.h"
#include "HorrorCharacter.h"

void UHorrorUI::SetupCharacter(AHorrorCharacter* HorrorCharacter)
{
	HorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
	HorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UHorrorUI::OnSprintStateChanged);
	HorrorCharacter->OnHealthMeterUpdated.AddDynamic(this, &UHorrorUI::OnHealthMeterUpdated);
	HorrorCharacter->OnHealthStateChanged.AddDynamic(this, &UHorrorUI::OnHealthStateChanged);

}

void UHorrorUI::OnSprintMeterUpdated(float Percent)
{
	// call the BP handler
	BP_SprintMeterUpdated(Percent);
}

void UHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	// call the BP handler
	BP_SprintStateChanged(bSprinting);
}

void UHorrorUI::OnHealthMeterUpdated(float Percent)
{
	// call the BP handler
	BP_HealthMeterUpdated(Percent);
}

void UHorrorUI::OnHealthStateChanged(bool bTakenHealthDamage)
{
	// call the BP handler
	BP_HealthMeterChanged(bTakenHealthDamage);
}
