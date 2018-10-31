#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SHordeGameMode.h"

USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.0f;
	bIsDead = false;

	TeamNum = 255;

	SetIsReplicated(true);
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead)
	{
		bIsDead = true;
		return;
	}

	// We are friendly to ourselves, but we are not our friends
	if (IsFriendly(DamagedActor, DamageCauser) && DamageCauser != DamageCauser)
	{
		return;
	}
	
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ASHordeGameMode* GM = Cast<ASHordeGameMode>(GetWorld()->GetAuthGameMode());

		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);

}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	USHealthComponent* HealthA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));
	
	if (HealthA == nullptr || HealthB)
	{
		return false;
	}

	return HealthA->TeamNum == HealthB->TeamNum;
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, OldHealth - Health, nullptr, nullptr, nullptr);
}
