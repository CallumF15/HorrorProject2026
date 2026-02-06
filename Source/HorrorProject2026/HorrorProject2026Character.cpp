// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorProject2026Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"


//network 
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HorrorProject2026.h"

AHorrorProject2026Character::AHorrorProject2026Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AHorrorProject2026Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static bool bLogged = false;
	if (!bLogged)
	{
		int32 Count = 0;
		for (TActorIterator<AHorrorProject2026Character> It(GetWorld()); It; ++It)
		{
			Count++;
		}

		if (Count > 1)
		{
			bLogged = true;
			DebugAllPlayers();
		}
	}
}


void AHorrorProject2026Character::DebugAllPlayers()
{
	for (TActorIterator<AHorrorProject2026Character> It(GetWorld()); It; ++It)
	{
		AHorrorProject2026Character* Char = *It;
		if (Char)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Character: %s, Local: %d, Role: %d"),
				*Char->GetName(),
				Char->IsLocallyControlled(),
				(int32)Char->GetLocalRole()
			);
		}
	}
}


void AHorrorProject2026Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHorrorProject2026Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHorrorProject2026Character::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHorrorProject2026Character::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHorrorProject2026Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AHorrorProject2026Character::LookInput);

		//jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHorrorProject2026Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHorrorProject2026Character::DoJumpEnd);
	}
	else
	{
		UE_LOG(LogHorrorProject2026, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AHorrorProject2026Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AHorrorProject2026Character::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AHorrorProject2026Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AHorrorProject2026Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AHorrorProject2026Character::DoJumpStart()
{
	//UE_LOG(LogTemp, Warning, TEXT("CanJump(): %s"), CanJump() ? TEXT("True") : TEXT("False"));
	//UE_LOG(LogTemp, Warning, TEXT("IsFalling(): %s"), GetCharacterMovement()->IsFalling() ? TEXT("True") : TEXT("False"));	
	//UE_LOG(LogTemp, Warning, TEXT("JumpZVelocity: %f"), GetCharacterMovement()->JumpZVelocity);

	// pass Jump to the character
	Jump();
}

void AHorrorProject2026Character::DoJumpEnd()
{
	//UE_LOG(LogTemp, Warning, TEXT("CanJump(): %s"), CanJump() ? TEXT("True") : TEXT("False"));
	//UE_LOG(LogTemp, Warning, TEXT("IsFalling(): %s"), GetCharacterMovement()->IsFalling() ? TEXT("True") : TEXT("False"));

	// pass StopJumping to the character
	StopJumping();
}




