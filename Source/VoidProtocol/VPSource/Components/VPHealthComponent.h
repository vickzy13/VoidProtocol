// VPHealthComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VPHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (VoidProtocol), meta = (BlueprintSpawnableComponent))
class VOIDPROTOCOL_API UVPHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVPHealthComponent();

    UPROPERTY(BlueprintAssignable)
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FOnDeath OnDeath;

    UFUNCTION(BlueprintCallable)
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable)
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable)
    bool IsDead() const { return bIsDead; }

    UFUNCTION(Server, Reliable)
    void ServerApplyDamage(float Amount);

    void HandleTakeDamage(float Amount);

    void Revive(float ReviveHealth);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_Health)
    float Health = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth = 100.f;

    bool bIsDead = false;

    UFUNCTION()
    void OnRep_Health();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnDeath();
};