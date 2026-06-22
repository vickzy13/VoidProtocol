// VPCharacter.h — what stays:
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VPCharacter.generated.h"

class UVPHealthComponent;

UENUM(BlueprintType)
enum class EVPRole : uint8
{
    None        UMETA(DisplayName = "None"),
    Infiltrator UMETA(DisplayName = "Infiltrator"),
    Hacker      UMETA(DisplayName = "Hacker")
};

UCLASS()
class VOIDPROTOCOL_API AVPCharacter : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UVPHealthComponent* HealthComponent;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Role, BlueprintReadOnly, Category = "Role")
    EVPRole VPRole = EVPRole::None;

    UPROPERTY(ReplicatedUsing = OnRep_DetectionLevel, BlueprintReadOnly, Category = "Stealth")
    float DetectionLevel = 0.f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stealth")
    FVector ThreatLocation = FVector::ZeroVector;

    UFUNCTION()
    void OnRep_Role();

    UFUNCTION()
    void OnRep_DetectionLevel();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    AVPCharacter();

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    void SetRole_Server(EVPRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Role")
    EVPRole GetRole() const { return VPRole; }

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void SetDetectionLevel(float NewLevel) { DetectionLevel = FMath::Clamp(NewLevel, 0.f, 100.f); }

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    float GetDetectionLevel() const { return DetectionLevel; }

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void SetThreatLocation(FVector NewLocation) { ThreatLocation = NewLocation; }

    UFUNCTION(BlueprintCallable, Category = "Stealth")
    FVector GetThreatLocation() const { return ThreatLocation; }

    FORCEINLINE UVPHealthComponent* GetHealthComponent() const { return HealthComponent; }
};