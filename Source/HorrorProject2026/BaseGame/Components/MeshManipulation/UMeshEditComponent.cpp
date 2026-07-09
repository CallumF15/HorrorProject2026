#include "UMeshEditComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/DynamicMeshComponent.h"
#include "UDynamicMesh.h"
#include "Generators/SphereGenerator.h"
#include "DynamicMesh/MeshTransforms.h"
#include "DynamicMesh/DynamicMesh3.h"

#include "GeometryScript/MeshPrimitiveFunctions.h"

UStaticMesh* PreviewSphereMesh;

UMeshEditComponent::UMeshEditComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));

	if (SphereMesh.Succeeded())
	{
		PreviewSphereMesh = SphereMesh.Object;
	}
}


void UMeshEditComponent::BeginPlay()
{
	Super::BeginPlay();

	BrushPreviewSphere = NewObject<UStaticMeshComponent>(GetOwner());
	// DynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComponent")); // CreateDefaultSubobject should be used in constructor?
	DynamicMeshComponent = NewObject<UDynamicMeshComponent>(GetOwner(), TEXT("DynamicMeshComponent"));
	DynamicMeshComponent->RegisterComponent();
	DynamicMeshComponent->SetIsReplicated(true);
	DynamicMeshComponent->AttachToComponent(GetOwner()->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
	DynamicMeshComponent->GetDynamicMesh()->Reset();

	BrushPreviewSphere->RegisterComponent();
	BrushPreviewSphere->AttachToComponent(GetOwner()->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
	BrushPreviewSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BrushPreviewSphere->SetVisibility(false);
	BrushPreviewSphere->SetStaticMesh(PreviewSphereMesh);
}

void UMeshEditComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsEditModeEnabled) // Editing Mode Disabled
	{
		BrushPreviewSphere->SetVisibility(false); 
	}
	else
	{
		//MouseOnScreen();
		
		FHitResult Hit;

		if (TraceForBrush(Hit))
		{
			FVector BrushLocation = Hit.Location;
			CheckLockedAxis(BrushLocation);
			
			BrushPreviewSphere->SetVisibility(true);
			BrushPreviewSphere->SetWorldLocation(BrushLocation);
			BrushPreviewSphere->SetWorldScale3D(FVector(BrushRadius / 50.f));
		}
	}
}

void UMeshEditComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UMeshEditComponent, TargetMesh);
}

void UMeshEditComponent::CheckLockedAxis(FVector& BrushLocation)
{
	if (bisXAxisLocked)
		BrushLocation.X =  lockedAxisX;  //lock X axis
	
	if (bisYAxisLocked)
		BrushLocation.Y =  lockedAxisY; //lock Y axis
	
	if (bisZAxisLocked)
	{
		BrushLocation.Z =  lockedAxisZ;
		
		//FVector NewLocation;
				
		// if (GetMousePlanePosition(NewLocation))
		// {
		// 	//BrushPreviewSphere->SetWorldLocation(NewLocation);
		//
		// }
	}else
	{
		Test(BrushLocation);
	}
}

void UMeshEditComponent::Test(FVector& BrushLocation)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());

	if (!PC)
		return;

	FVector BrushStartLocation = BrushLocation;
	
	float MouseX;
	float MouseY;

	PC->GetMousePosition(MouseX, MouseY);
	FVector2D BrushStartMousePosition  = FVector2D(MouseX, MouseY);
	
	float MouseDelta = BrushStartMousePosition.Y - MouseY;
	

	BrushLocation.Z += MouseDelta * 2;;

	BrushLocation.Z = BrushStartLocation.Z +(MouseDelta * 2);
}



void UMeshEditComponent::MouseOnScreen()
{
	APlayerController* PC = nullptr;

	APawn* PawnOwner = Cast<APawn>(GetOwner());

	if (PawnOwner)
	{
		PC = Cast<APlayerController>(PawnOwner->GetController());
	}

	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
	}

	FVector WorldLocation;
	FVector WorldDirection;

	if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		FVector End = WorldLocation + WorldDirection * 5000.f;

		FHitResult Hit;

		if (GetWorld()->LineTraceSingleByChannel(
			Hit,
			WorldLocation,
			End,
			ECC_Visibility))
		{
			BrushPreviewSphere->SetWorldLocation(Hit.Location);
		}
	}
}

void UMeshEditComponent::SendEditToServer(const FMeshEdit& Edit)
{
	if (!TargetMesh) return;

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerSendEdit(TargetMesh, Edit);
	}
	else
	{
		TargetMesh->ApplyEdit_Internal(Edit);
	}
}

bool UMeshEditComponent::GetMousePlanePosition(FVector& OutPosition)
{
	APawn* Pawn = Cast<APawn>(GetOwner());

	if (!Pawn)
		return false;

	APlayerController* PC =Cast<APlayerController>(Pawn->GetController());

	if (!PC)
		return false;


	FVector RayStart;
	FVector RayDirection;

	if (!PC->DeprojectMousePositionToWorld(RayStart,RayDirection))
		return false;
	
	// Plane origin (where the brush started)
	FVector PlaneOrigin = BrushPreviewSphere->GetComponentLocation();

	// Plane normal - locks movement along Y
	FVector PlaneNormal = FVector::YAxisVector;
	
	FVector RayEnd = RayStart + RayDirection * 10000.f;

	FVector CameraLocation;
	FRotator CameraRotation;
	
	PC->GetPlayerViewPoint(CameraLocation,CameraRotation);
	
	FPlane MovementPlane(
		PlaneOrigin,
		CameraRotation.Vector()
	);

	
	 bool bHit = FMath::SegmentPlaneIntersection(
		RayStart,
		RayEnd,
		MovementPlane,
		OutPosition
	);

	
	UE_LOG(LogTemp, Warning,
	 TEXT("Plane hit: %s Location: %s"),
	 bHit ? TEXT("true") : TEXT("false"),
	 *OutPosition.ToString()
 );

	return bHit;
}

void UMeshEditComponent::UpdateVerticalBrushMovement()
{
	// APlayerController* PC =
	// 	Cast<APlayerController>(GetOwner()->GetInstigatorController());
	//
	// if (!PC)
	// 	return;
	//
	// float MouseX;
	// float MouseY;
	//
	// PC->GetMousePosition(MouseX, MouseY);
	//
	// float DeltaY = BrushStartMousePosition.Y - MouseY;
	//
	//
	// FVector NewLocation = BrushStartLocation;
	//
	// // Only vertical movement
	// NewLocation.Z += DeltaY * 2;
	//
	// // Keep X/Y fixed
	// NewLocation.X = BrushStartLocation.X;
	// NewLocation.Y = BrushStartLocation.Y;
	//
	//
	// BrushPreviewSphere->SetWorldLocation(NewLocation);
}

bool UMeshEditComponent::TraceForBrush(FHitResult& OutHit)
{
	APawn* Pawn = Cast<APawn>(GetOwner());

	if (!Pawn)
		return false;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());

	if (!PC)
		return false;
	
	FVector WorldLocation;
	FVector WorldDirection;
	
	if (!PC->DeprojectMousePositionToWorld(WorldLocation,WorldDirection))
		return false;


	FVector End = WorldLocation + WorldDirection * 5000.f;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		WorldLocation,
		End,
		ECC_Visibility,
		Params
	);
}
bool UMeshEditComponent::TraceForMesh(FHitResult& OutHit)
{
	FVector Start;
	FRotator Rotation;

	// from camera
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return false;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	PC->GetPlayerViewPoint(Start, Rotation);
	
	const FVector End = Start + Rotation.Vector() * 1000.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		Params
	);
}
void UMeshEditComponent::TryEditMesh()
{
	if (!TargetMesh) return;

	FHitResult Hit;
	if (!TraceForMesh(Hit)) return;

	int32 VertexIndex = Hit.Item; // depends on how you encode vertex selection

	// switch (currentMode)
	// {
	// 	case EMeshEditType::MoveVertex:
	// 		//Edit.Type = EMeshEditType::MoveVertex;
	// 		break;
	//
	// 	case EMeshEditType::ExtrudeFace:
	// 		//Edit.Type = EMeshEditType::SculptBrush;
	// 		break;
	// }

	FMeshEdit Edit;
	Edit.Type = currentMode;
	Edit.VertexIndex = VertexIndex;
	Edit.Position = Hit.ImpactPoint;

	SendEditToServer(Edit);
}

void UMeshEditComponent::CreateMeshAtBrushLocation()
{
	if (!DynamicMeshComponent || !BrushPreviewSphere)
	{
		return;
	}

	FVector WorldLocation = BrushPreviewSphere->GetComponentLocation();
	FVector LocalLocation = DynamicMeshComponent->GetComponentTransform().InverseTransformPosition(WorldLocation);
	
	UDynamicMesh* DynamicMesh = DynamicMeshComponent->GetDynamicMesh();

	if (!DynamicMesh)
	{
		return;
	}
	
	FGeometryScriptPrimitiveOptions Options;
	
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSphereBox( //could use the one with collision? (AppendSphereBoxWithCollision)
		DynamicMesh,
		Options,
		FTransform(LocalLocation),
		BrushRadius,
		32,
		16
	);
	
	DynamicMeshComponent->NotifyMeshUpdated();
}

void UMeshEditComponent::ServerSendEdit_Implementation(AEditableMeshActor* Target, FMeshEdit Edit)
{
	if (!Target) return;

	Target->ApplyEdit_Internal(Edit);
}

void UMeshEditComponent::SetTargetMesh(AEditableMeshActor* NewTarget)
{
	TargetMesh = NewTarget;
}

//EDITING

void UMeshEditComponent::ToggleEditModeEnabled()
{
	bIsEditModeEnabled = !bIsEditModeEnabled;
	SetComponentTickEnabled(bIsEditModeEnabled);
}
void UMeshEditComponent::SetEditingEnabled(bool bEnabled)
{
	bIsEditingAction = bEnabled;
	SetComponentTickEnabled(bEnabled);
}
bool UMeshEditComponent::getEditModeEnabled() const
{
	return bIsEditModeEnabled;
}

// AXIS  

void UMeshEditComponent::ToggleXAxisLock()
{
	bisXAxisLocked = !bisXAxisLocked;

	if (bisXAxisLocked)
		lockedAxisX = BrushPreviewSphere->GetComponentLocation().X;
}
void UMeshEditComponent::ToggleYAxisLock()
{
	bisYAxisLocked = !bisYAxisLocked;

	if (bisYAxisLocked)
		lockedAxisY = BrushPreviewSphere->GetComponentLocation().Y;
}
void UMeshEditComponent::ToggleZAxisLock() //Z is our vertical axis
{
	bisZAxisLocked = !bisZAxisLocked;

	if (bisZAxisLocked)
		lockedAxisZ = BrushPreviewSphere->GetComponentLocation().Z;
}

//BRUSH

void UMeshEditComponent::AdjustBrushSize(const FInputActionValue& inputValue)
{
	float scroll = inputValue.Get<float>();
	BrushRadius += scroll * brushScaleMultiplier; //scale the brush size 
	BrushRadius = FMath::Clamp(BrushRadius, MIN_BRUSH_SCALE, MAX_BRUSH_SCALE); //clamp the min and max size of brush

	UE_LOG(LogTemp, Warning, TEXT("Scroll: %f"), BrushRadius);
}
