#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "SCountNearbyActorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNearbyActorCountChangedSignature, int32, NearbyActors);


UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USCountNearbyActorComponent : public USphereComponent
{
	GENERATED_BODY()

public:	
	USCountNearbyActorComponent(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Actor counter")
	float CheckInterval;

	UFUNCTION()
	void CheckActors();

	/*UPROPERTY(VisibleAnywhere, Category = "Actor counter")
	USphereComponent* ActorTrackingSphereComp;*/

	FTimerHandle TimerHandle_CheckActors;

	UPROPERTY(EditDefaultsOnly, Category = "Actor counter")
	TSubclassOf<AActor> CheckActorClass;

	virtual void BeginPlay() override;

public:	
	int32 NearbyActors;
	void SetCheckRadius(float Radius);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNearbyActorCountChangedSignature OnNearbyActorCountChanged;
};