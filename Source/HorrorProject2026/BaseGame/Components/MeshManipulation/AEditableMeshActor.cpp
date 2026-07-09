#include "AEditableMeshActor.h"
#include "Components/DynamicMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEditableMesh, Log, All);

AEditableMeshActor::AEditableMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMesh"));

	UE_LOG(LogEditableMesh, Warning, TEXT("EditableMeshActor BeginPlay fired"));
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Variant_Horror/Blueprints/Light/Assets/Materials/M_Light.M_Light"));

	if (Material.Succeeded())
	{
		MeshComponent->SetMaterial(0, Material.Object);
	}

	RootComponent = MeshComponent;
}

void AEditableMeshActor::BeginPlay()
{
	Super::BeginPlay();

	CreateTestMesh();


	FMeshEdit TestEdit;

	TestEdit.Type = EMeshEditType::MoveVertex;
	TestEdit.VertexIndex = 0;
	TestEdit.Position = FVector(0,0,200);


	ApplyEdit_Internal(TestEdit);
}

void AEditableMeshActor::ServerApplyEdit_Implementation(const FMeshEdit& Edit)
{
	ApplyEdit_Internal(Edit);

	// replicate to other clients
	MulticastApplyEdit(Edit);
}

void AEditableMeshActor::MulticastApplyEdit_Implementation(const FMeshEdit& Edit)
{
	if (!HasAuthority())
	{
		ApplyEdit_Internal(Edit);
	}
}

void AEditableMeshActor::ApplyEdit_Internal(const FMeshEdit& Edit)
{
	FDynamicMesh3& Mesh = MeshComponent->GetDynamicMesh()->GetMeshRef();
	
	switch (Edit.Type)
	{
		case EMeshEditType::MoveVertex:
			{
				if (Mesh.IsVertex(Edit.VertexIndex))
				{
					Mesh.SetVertex(Edit.VertexIndex, Edit.Position);
				}
				break;
			}
		case EMeshEditType::SplitEdge:
			break;
			
		case EMeshEditType::DeleteTriangle:
			break;

		case EMeshEditType::ExtrudeFace:
			break;
	}
	
	MeshComponent->NotifyMeshUpdated();
}

void AEditableMeshActor::CreateTestMesh()
{
	FDynamicMesh3& Mesh = MeshComponent->GetDynamicMesh()->GetMeshRef();

	int32 V0 = Mesh.AppendVertex(FVector(0,0,0));
	int32 V1 = Mesh.AppendVertex(FVector(100,0,0));
	int32 V2 = Mesh.AppendVertex(FVector(0,100,0));
	int32 V3 = Mesh.AppendVertex(FVector(100,100,0));
	
	Mesh.AppendTriangle(V0,V1,V2);
	Mesh.AppendTriangle(V1,V3,V2);

	UE_LOG(LogEditableMesh, Warning,TEXT("Created vertices: %d %d %d"), V0, V1, V2);

	MeshComponent->NotifyMeshUpdated();
}


