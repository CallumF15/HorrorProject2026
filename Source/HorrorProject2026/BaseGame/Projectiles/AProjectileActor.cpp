#include "AProjectileActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AProjectileActor::AProjectileActor()
{
	bReplicates = true;

	//Definition for the SphereComponent that will serve as the Root component for the projectile and its collision.
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->InitSphereRadius(37.5f);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = SphereComponent;
	
	//Registering the Projectile Impact function on a Hit event.
	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileActor::OnProjectileImpact);
	}

	//Definition for the Mesh that will serve as your visual representation.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/Meshes/SM_FoamBullet.SM_FoamBullet"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
	//Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh loaded successfully"));
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); //if mesh is changed, remember -90 Yaw may need changed
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh FAILED to load"));
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("../Game/Variant_Shooter/Blueprints/Pickups/Projectiles/Materials/M_Explosion.M_Explosion"));
	//   "/Blueprints/Pickups/Projectiles/Materials/M_Explosion.M_Explosion"
	
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UParticleSystem failed"));
	}

	
	// static ConstructorHelpers::FClassFinder<AActor> ExplosionClass(TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/BP_Explosion"));
	//
	// if (ExplosionClass.Succeeded())
	// {
	// 	ExplosionBP = ExplosionClass.Class;
	// }

	//Definition for the Projectile Movement Component.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;
}

void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(UShooterComponent, YourReplicatedVariable);
}

void AProjectileActor::Destroyed()
{
	UE_LOG(LogTemp, Warning, TEXT("Destroyed"));
	
	if (ExplosionEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("World and Explosion"));
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			ExplosionEffect,
			GetActorLocation(),
			FRotator::ZeroRotator,
			true,
			EPSCPoolMethod::AutoRelease
		);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("No World and Explosion"));
	}
	
	FVector spawnLocation = GetActorLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
}



void AProjectileActor::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit,GetInstigator()->Controller, GetOwner(), DamageType);
	}

	Destroy();
}
