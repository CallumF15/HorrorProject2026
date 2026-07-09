#include "AMorphCharacter.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../Components/MeshManipulation/UMeshEditComponent.h"

AMorphCharacter::AMorphCharacter()
{
	MeshEditComponent = CreateDefaultSubobject<UMeshEditComponent>(TEXT("MeshEditComponent"));
}

void AMorphCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(MeshEditComponent);

	//UE_LOG(LogEditableMesh, Warning, TEXT("AMorphCharacter BeginPlay fired"));
}

void AMorphCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMorphCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	{
		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			EnhancedInputComponent->BindAction(ToggleMeshEditAction, ETriggerEvent::Started, this, &AMorphCharacter::ToggleEditing);
			EnhancedInputComponent->BindAction(EditMeshAction, ETriggerEvent::Triggered, this, &AMorphCharacter::StartEdit);
			EnhancedInputComponent->BindAction(EditMeshAction, ETriggerEvent::Completed, this, &AMorphCharacter::StopEdit);
			
			EnhancedInputComponent->BindAction(BrushSizeAction, ETriggerEvent::Started, MeshEditComponent, &UMeshEditComponent::AdjustBrushSize);  //adjust size of brush
			
			EnhancedInputComponent->BindAction(LockXAxisAction, ETriggerEvent::Started, MeshEditComponent, &UMeshEditComponent::ToggleXAxisLock);  //lock X axis
			EnhancedInputComponent->BindAction(LockYAxisAction, ETriggerEvent::Started, MeshEditComponent, &UMeshEditComponent::ToggleYAxisLock);  //lock Y axis
			EnhancedInputComponent->BindAction(LockZAxisAction, ETriggerEvent::Started, MeshEditComponent, &UMeshEditComponent::ToggleZAxisLock);  //lock Z axis
		}
	}
}

void AMorphCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMorphCharacter::StartEdit()
{
	UE_LOG(LogTemp, Warning, TEXT("StartEdit"));

	if (!MeshEditComponent->getEditModeEnabled())
	{
		return;
	}
	
	MeshEditComponent->SetEditingEnabled(true);
	MeshEditComponent->CreateMeshAtBrushLocation();
}

void AMorphCharacter::StopEdit()
{
	UE_LOG(LogTemp, Warning, TEXT("StopEdit"));
	
	if (!MeshEditComponent->getEditModeEnabled())
	{
		return;
	}
	
	MeshEditComponent->SetEditingEnabled(false);
}

void AMorphCharacter::ToggleEditing()
{
	MeshEditComponent->ToggleEditModeEnabled();
	UE_LOG(LogTemp, Warning, TEXT("Edit Mode Toggled: %s"), MeshEditComponent->getEditModeEnabled() ? TEXT("Enabled") : TEXT("Disabled"));
	
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!PC)
		return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (!Subsystem)
		return;
	
	if (!MeshEditComponent->getEditModeEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed Default IMC MESH"));
		Subsystem->RemoveMappingContext(MeshEditMappingContext);
		Subsystem->AddMappingContext(DefaultMappingContext, 1);
		Super::bCanLook = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed Default IMC DEFAULT"));
		Subsystem->RemoveMappingContext(DefaultMappingContext);
		Subsystem->AddMappingContext(MeshEditMappingContext, 1);
		Super::bCanLook = false;
	}
}

UMeshEditComponent* AMorphCharacter::GetMeshEditComponent() const
{
	return MeshEditComponent;
}

