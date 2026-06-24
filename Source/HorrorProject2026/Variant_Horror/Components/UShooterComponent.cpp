#include "UShooterComponent.h"

#include "AShooter.h"

UShooterComponent::UShooterComponent()
{
	SetIsReplicatedByDefault(true);
	UE_LOG(LogTemp, Warning, TEXT("shooter component constructor"));
	
	//Initialize projectile class
	ProjectileClass = AShooter::StaticClass();
	FireRate = 0.25f; 	//Initialize fire rate
	bIsFiringWeapon = false;
}

void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UShooterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// DOREPLIFETIME(UShooterComponent, HealthMeter);
}

void UShooterComponent::StartFire()
{
	UE_LOG(LogTemp, Warning, TEXT("component shooter firing"));
	
	if (!bIsFiringWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("fired gun"));
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();

		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("No World found in ShooterComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("World found in ShooterComponent"));
		}
		
		World->GetTimerManager().SetTimer(FiringTimer, this, &UShooterComponent::StopFire, FireRate, false);
		HandleFire();
	}
}
	 
void UShooterComponent::StopFire()
{
	bIsFiringWeapon = false;
}

void UShooterComponent::HandleFire_Implementation()
{
	AActor* actor = GetOwner();

	UE_LOG(LogTemp, Log, TEXT("HandleFire_Implementation firing"));
	
	FVector spawnLocation = actor->GetActorLocation() + (actor->GetActorRotation().Vector() * 100.0f) + (actor->GetActorUpVector() * 50.0f);
	FRotator spawnRotation = actor->GetActorRotation();
	 
	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = actor->GetInstigator();
	spawnParameters.Owner = actor;
	
	AActor* spawnedProjectile = GetWorld()->SpawnActor<AActor>(
	ProjectileClass,   // 👈 THIS IS REQUIRED
	spawnLocation,
	spawnRotation,
	spawnParameters
	);

	
}