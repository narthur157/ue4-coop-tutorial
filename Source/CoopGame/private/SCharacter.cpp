#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

ASCharacter::ASCharacter()
{
	bDied = false;
	DefaultFOV = 90;
	ZoomedFOV = 60;
	ZoomInterpSpeed = 1;
	WeaponAttachSocketName = "WeaponSocket";
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AvailableWeapons.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AvailableWeapons must have at least 1 weapon"));
	}

	SetupWeapon();

	CameraComp->SetFieldOfView(DefaultFOV);

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

void ASCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
	if (Health <= 0.0f && !bDied)
	{
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		StopFire();
		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);
	}
}

bool ASCharacter::SetupWeapon_Validate()
{
	return true;
}

void ASCharacter::SetupWeapon_Implementation()
{
	if (Role != ROLE_Authority) {
		SetupWeapon();
		return;
	}
	
	TSubclassOf<ASWeapon> SelectedWeaponClass = GetNextWeaponClass();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = this;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(SelectedWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
}

TSubclassOf<ASWeapon> ASCharacter::GetNextWeaponClass()
{

	TSubclassOf<ASWeapon> SelectedWeaponClass;

	if (CurrentWeapon) {
		SelectedWeaponClass = CurrentWeapon->GetClass();
		CurrentWeapon->Destroy();

		for (int i = 0; i < AvailableWeapons.Num(); i++)
		{
			if (SelectedWeaponClass == AvailableWeapons[i])
			{
				SelectedWeaponClass = AvailableWeapons[(i + 1) % AvailableWeapons.Num()];
				break;
			}
		}
	}
	else
	{
		if (ensure(AvailableWeapons.Num() > 0))
		{
			SelectedWeaponClass = AvailableWeapons[0];
		}
	}

	return SelectedWeaponClass;
}

void ASCharacter::PlaySwitchWeaponSound()
{
	if (SwitchWeaponSound && CurrentWeapon)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SwitchWeaponSound, GetActorLocation());
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);

	if (!IsLocallyControlled())
	{
		FRotator NewRot = CameraComp->RelativeRotation;
		NewRot.Pitch = RemoteViewPitch * 360.0f / 255.0f;;
		CameraComp->SetRelativeRotation(NewRot);
	}
}

void ASCharacter::OnBeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::DoJump()
{
	Jump();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASCharacter::ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::OnBeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::DoJump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &ASCharacter::SetupWeapon);
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}