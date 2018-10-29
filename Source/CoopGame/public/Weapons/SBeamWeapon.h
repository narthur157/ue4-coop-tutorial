#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SBeamWeapon.generated.h"

class UParticleSystem;

USTRUCT(BlueprintType)
struct FBeamStatus
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FVector_NetQuantize Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	bool bIsActive;
};

UCLASS()
class COOPGAME_API ASBeamWeapon : public ASWeapon
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UParticleSystem* BeamEffect;

	UPROPERTY(ReplicatedUsing= OnRep_BeamStatus)
	FBeamStatus BeamStatus;

	UFUNCTION(BlueprintImplementableEvent, Category = "Functions")
	void OnBeamStatusChange(FBeamStatus BS);

	UFUNCTION()
	void OnRep_BeamStatus();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sockets")
	FName BeamSocket;

	UPROPERTY(EditAnywhere, Category = "Beam")
	UParticleSystemComponent* BeamParticleComponent;
	
	void BeginPlay() override;
	void ApplyBeamStatus();
public:
	ASBeamWeapon();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	virtual void PostNetReceiveLocationAndRotation() override;

	virtual void StartFire() override;
	virtual void StopFire() override;
};
