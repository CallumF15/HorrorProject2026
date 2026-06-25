#include "USprintComponent.h"
#include "Net/UnrealNetwork.h"


USprintComponent::USprintComponent()
{
	SetIsReplicatedByDefault(true);
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetWorld())
	{
		return;
	}

	// SprintMeter = SprintTime;
	UE_LOG(LogTemp, Warning, TEXT("Health BeginPlay: %f"), SprintMeter);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		// initialize sprint & health meter to max
		SprintMeter = SprintTime;
	}
	
	// Initialize the walk speed
	//GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;  //find way to get Character inside component

	// start the sprint tick timer
	GetWorld()->GetTimerManager().SetTimer(SprintTimer, this, &USprintComponent::SprintFixedTick, SprintFixedTickTime, true);
	
	GetWorld()->GetTimerManager().SetTimer(
		SprintTimer,
		this,
		&USprintComponent::SprintFixedTick,
		SprintFixedTickTime,
		true);
}

void USprintComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the sprint timer
	GetWorld()->GetTimerManager().ClearTimer(SprintTimer);
}

void USprintComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USprintComponent, bSprinting);
}

void USprintComponent::SprintFixedTick()
{
	// UE_LOG(LogTemp, Warning, TEXT("Sprinting: %d"), bSprinting);
	// UE_LOG(LogTemp, Warning, TEXT("Stamina: %f"), SprintMeter);

	bHasStamina = SprintMeter > 0.01f;
	
	if (bSprinting)
	{
		if (bHasStamina)
			SprintMeter -= SprintFixedTickTime;
		//SprintMeter -= SprintDrainRate * SprintFixedTickTime;
	}
	else
	{
		SprintMeter += SprintFixedTickTime; //recover stamima
		//SprintMeter += SprintRegenRate * SprintFixedTickTime; // can expand and add Regen 
	}

	SprintMeter = FMath::Clamp(SprintMeter, 0.f, SprintTime);
	OnSprintStateChanged.Broadcast(bSprinting);
	
	const float TargetSpeed = bSprinting ? SprintSpeed : WalkSpeed;
	//GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;  ///////////////////////////////////////////////////////
	
	//bRecovering = (!bSprinting && SprintMeter < SprintTime);

	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);

	// UE_LOG(LogTemp, 
	// 	Log, TEXT("SprintStates: bSprinting=%s, bSprintButtonHeld=%s, bRecovering=%s, SprintMeter=%.2f"),
	// 	bSprinting ? TEXT("true") : TEXT("false"),
	// 	bSprintButtonHeld ? TEXT("true") : TEXT("false"),
	// 	bRecovering ? TEXT("true") : TEXT("false"),
	// 	SprintMeter
	// );
}

void USprintComponent::DoStartSprint()
{
	//UE_LOG(LogTemp, Warning, TEXT("DoStartSprint CALLED"));
	bSprinting = true;
	
	ServerSetSprinting(true);
	
}

void USprintComponent::DoEndSprint()
{
	//UE_LOG(LogTemp, Warning, TEXT("DoEndSprint CALLED"));
	bSprinting = false;
	
	ServerSetSprinting(false);
}


void USprintComponent::OnRep_Sprinting()
{
	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);
}

//RPC Call
void USprintComponent::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	bSprinting = bNewSprinting;
	
	// UE_LOG(LogTemp, Warning, TEXT("SERVER Sprint = %d"), bNewSprinting);
	// UE_LOG(LogTemp, Warning, TEXT("SERVER Sprinting: %d Stamina: %f"), bSprinting, SprintMeter)
}


//validate sprinting, if sprint exceeds max or goes below min then the client will be kicked 
bool USprintComponent::ServerSetSprinting_Validate(bool bNewSprinting)
{
	if(bNewSprinting){

		SprintMeter -= SprintFixedTickTime;
		
		if (SprintMeter < 0.f || SprintMeter > 100.f)
		{
			return false;
		}
	}

	return true;
}

