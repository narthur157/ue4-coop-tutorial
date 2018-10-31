#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USoundBase;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASCharacter();

protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void OnBeginCrouch();
	void EndCrouch();
	void DoJump();
	void BeginZoom();
	void EndZoom();

	TSubclassOf<ASWeapon> GetNextWeaponClass();

	UFUNCTION()
	void PlaySwitchWeaponSound();

	UFUNCTION(Server, Reliable, WithValidation)
	void SetupWeapon();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	bool bWantsToZoom;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Animations")
	bool bDied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	USoundBase* SwitchWeaponSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<ASWeapon>> AvailableWeapons;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(ReplicatedUsing=PlaySwitchWeaponSound)
	ASWeapon* CurrentWeapon;

public:
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual FVector GetPawnViewLocation() const override;	
};
