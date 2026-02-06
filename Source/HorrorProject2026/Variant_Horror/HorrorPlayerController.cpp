// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "HorrorProject2026CameraManager.h"

//network 
#include "Net/UnrealNetwork.h"

#include "HorrorCharacter.h"
#include "HorrorUI.h"
#include "HorrorProject2026.h"
#include "Widgets/Input/SVirtualJoystick.h"

AHorrorPlayerController::AHorrorPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AHorrorProject2026CameraManager::StaticClass();
}

void AHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogHorrorProject2026, Error, TEXT("Could not spawn mobile controls widget."));

		}
	}

	if (!IsLocalPlayerController()) return;

	if (!HorrorUI && HorrorUIClass)
	{
		HorrorUI = CreateWidget<UHorrorUI>(this, HorrorUIClass);
		HorrorUI->AddToViewport(0);


		if (APawn* MyPawn = GetPawn())
		{
			if (AHorrorCharacter* HorrorChar = Cast<AHorrorCharacter>(MyPawn))
			{
				HorrorUI->SetupCharacter(HorrorChar);
			}
		}
	}
}

void AHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (!IsLocalPlayerController() || !HorrorUI) return;

	if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(aPawn))
	{
		HorrorUI->SetupCharacter(HorrorCharacter);
		UE_LOG(LogHorrorProject2026, Warning, TEXT("HorrorUI SetupCharacter called"));
	}
}


//void AHorrorPlayerController::OnPossess(APawn* aPawn)
//{
//	Super::OnPossess(aPawn);
//
//	UE_LOG(LogHorrorProject2026, Warning, TEXT("PlayerController OnPossess: %s, IsLocalPlayerController: %d"),
//		*GetNameSafe(aPawn), IsLocalPlayerController());
//
//
//	// only spawn UI on local player controllers
//	if (IsLocalPlayerController())
//	{
//		// set up the UI for the character
//		if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(aPawn))
//		{
//			// create the UI if it doesn't exist
//			if (!HorrorUI)
//			{
//				HorrorUI = CreateWidget<UHorrorUI>(this, HorrorUIClass);
//			
//
//				ULocalPlayer* LP = GetLocalPlayer();
//
//				UE_LOG(LogHorrorProject2026, Warning,
//					TEXT("UI DEBUG | IsLocal=%d LocalPlayer=%s ViewportClient=%s"),
//					IsLocalPlayerController(),
//					LP ? TEXT("YES") : TEXT("NO"),
//					(LP && LP->ViewportClient) ? TEXT("YES") : TEXT("NO")
//				);
//
//				if (HorrorUI) {
//					UE_LOG(LogHorrorProject2026, Warning, TEXT("HorrorUI created and added to viewport!"));
//					HorrorUI->AddToViewport(0);
//				}
//				else
//					UE_LOG(LogHorrorProject2026, Error, TEXT("HorrorUI failed to create!"));
//			}
//
//			HorrorUI->SetupCharacter(HorrorCharacter);
//		}
//	}
//}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}	
}

bool AHorrorPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
