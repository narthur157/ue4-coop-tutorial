#include "SCountNearbyActorComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

USCountNearbyActorComponent::USCountNearbyActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CheckInterval = 0.5f;

//	ActorTrackingSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("ActorTrackingSphereComp"));
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	NearbyActors = 0;
}


// Called when the game starts
void USCountNearbyActorComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle_CheckActors, this, &USCountNearbyActorComponent::CheckActors, CheckInterval, true, 0.0f);
	}
}

void USCountNearbyActorComponent::SetCheckRadius(float Radius)
{
	SphereRadius = Radius;
}

void USCountNearbyActorComponent::CheckActors()
{
	if (CheckActorClass)
	{
		TArray<AActor *> Result;
		//DrawDebugSphere(GetWorld(), GetComponentLocation(), SphereRadius, 12, FColor::Green, false, CheckInterval, 0, 1);
		GetOverlappingActors(Result, CheckActorClass);
		NearbyActors = Result.Num();
		OnNearbyActorCountChanged.Broadcast(NearbyActors);
	}
}
